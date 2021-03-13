#include "../Externals/Include/Include.h"

#define deg2rad(x) ((x)*((3.1415926f)/(180.0f)))

using namespace glm;
using namespace std;

float timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

//default window size
const int WINDOW_WIDTH = 1440;
const int WINDOW_HEIGHT = 900;

// current window size
int screenWidth = WINDOW_WIDTH, screenHeight = WINDOW_HEIGHT;

typedef struct
{
	GLuint diffuseTexture;
} PhongMaterial;

vector<PhongMaterial> allMaterial;
vector<PhongMaterial> allMaterialBar;

typedef struct
{
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object
	GLuint ibo;

	int materialId;
	int vertexCount;
	GLuint m_texture;
} Shape;

Shape nano;
Shape quad;

struct model
{
	vector<Shape> shapes;

	vec3 position = vec3(0, 0, 0);
	vec3 scale = vec3(1, 1, 1);
	vec3 rotation = vec3(0, 0, 0);
};
model scene_model;

struct camera
{
	vec3 position;
	vec3 center;
	vec3 up_vector;
};
camera main_camera;

struct project_setting
{
	GLfloat nearClip, farClip;
	GLfloat fovy;
	GLfloat aspect;
};
project_setting proj;


// cubemap
GLuint skybox_tex;
GLuint skybox_vao;
vector<string> faces
{
	"cubemaps/face-r.png",
	"cubemaps/face-l.png",
	"cubemaps/face-t.png",
	"cubemaps/face-d.png",
	"cubemaps/face-b.png",
	"cubemaps/face-f.png"
};
GLuint program;
GLuint iLocP;
GLuint iLocV;
GLuint eye_pos;
mat4 project_matrix(1.0f);
mat4 view_matrix(1.0f);

// model
GLuint model_m;
GLuint model_v;
GLuint model_p;
GLuint eye_pos_m;
size_t draw_count;
GLuint model_program;
GLuint nano_shadow;
GLuint nano_tex;
GLuint nano_skytex;

// quad
GLuint quad_m;
GLuint quad_v;
GLuint quad_p;
GLuint eye_pos_q;
size_t draw_count_quad;
GLuint quad_program;
GLuint quad_shadow;
GLuint quad_tex;

// shadow
GLuint shadow_program;
GLuint shadow_fbo;
GLuint shadow_mvp;
GLuint depthrbo_shadow;
GLuint fboDataTexture_shadow;

// window FBO
GLuint vao_window;
GLuint fbo_window;
GLuint depthrbo_window;
GLuint window_program;
GLuint window_vertex_buffer;
GLuint fboDataTexture_window;
GLuint diffRender_Sobj;
GLuint diffRender_Snoobj;
GLuint diffRender_Sb;

// Sobj FBO
GLuint vao_Sobj;
GLuint fbo_Sobj;
GLuint depthrbo_Sobj;
GLuint fboDataTexture_Sobj;

// Snoobj FBO
GLuint vao_Snoobj;
GLuint fbo_Snoobj;
GLuint depthrbo_Snoobj;
GLuint fboDataTexture_Snoobj;
GLuint useShadow;

// Sb FBO
GLuint vao_Sb;
GLuint fbo_Sb;
GLuint depthrbo_Sb;
GLuint fboDataTexture_Sb;

// switch FBo uniform
GLuint switch_FBO;
int framebuffer;

// rotate
int direction = 0;

static const GLfloat window_vertex[] =
{
	//vec2 position vec2 texture_coord
	1.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f
};

char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void freeShaderSource(char** srcp)
{
	delete[] srcp[0];
	delete[] srcp;
}

void printGLShaderLog(const GLuint shader)
{
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// maxLength already includes the NULL terminator. no need to +1
		GLchar* errorLog = new GLchar[maxLength];
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		printf("%s\n", errorLog);
		delete[] errorLog;
	}
}

// define a simple data structure for storing texture image raw data
typedef struct _TextureData
{
	_TextureData(void) :
		width(0),
		height(0),
		data(0)
	{
	}

	int width;
	int height;
	unsigned char* data;
} TextureData;

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
TextureData loadImage(const char* const Filepath)
{
	TextureData texture;
	int n;
	int len = strlen("..\\Assets\\cubemaps\\") + strlen(Filepath) + 1;
	char *filepath = (char*)malloc(sizeof(char) * len);
	memset(filepath, '\0', len);
	strcat(filepath, "..\\Assets\\cubemaps\\");
	strcat(filepath, Filepath);
	stbi_set_flip_vertically_on_load(true);
	stbi_uc *data = stbi_load(filepath, &texture.width, &texture.height, &n, 4);
	if (data != NULL)
	{
		texture.data = new unsigned char[texture.width * texture.height * 4 * sizeof(unsigned char)];
		memcpy(texture.data, data, texture.width * texture.height * 4 * sizeof(unsigned char));
		stbi_image_free(data);
	}
	return texture;
}

// load obj file
void My_LoadModels(const string& path)
{
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	for (int s = 0; s < shapes.size(); ++s) {  // for 'ladybug.obj', there is only one object
		int index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			nano.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &nano.vao);
	glBindVertexArray(nano.vao);

	glGenBuffers(1, &nano.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, nano.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	shapes.clear();
	shapes.shrink_to_fit();
	materials.clear();
	materials.shrink_to_fit();
	vertices.clear();
	vertices.shrink_to_fit();
	texcoords.clear();
	texcoords.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();

	cout << "Load " << nano.vertexCount << " vertices" << endl;
}

void My_LoadModels2(const string& path)
{
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	vector<float> vertices, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	for (int s = 0; s < shapes.size(); ++s) {  // for 'ladybug.obj', there is only one object
		int index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			quad.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &quad.vao);
	glBindVertexArray(quad.vao);

	glGenBuffers(1, &quad.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size()));
	glEnableVertexAttribArray(1);

	shapes.clear();
	shapes.shrink_to_fit();
	materials.clear();
	materials.shrink_to_fit();
	vertices.clear();
	vertices.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();

	cout << "Load " << quad.vertexCount << " vertices" << endl;
}

void initParameter()
{
	proj.nearClip = 0.1;
	proj.farClip = 1000.0;
	proj.fovy = 80.0f;
	proj.aspect = (float)(WINDOW_WIDTH) / (float)WINDOW_HEIGHT; // adjust width for side by side view

	main_camera.position = vec3(0.0f, 0.0f, 0.0f);
	main_camera.center = vec3(-1.0f, -1.0f, 0.0f);
	main_camera.up_vector = vec3(0.0f, 1.0f, 0.0f);
}

void createShader()
{
	// Create Shader Program
	program = glCreateProgram();
	model_program = glCreateProgram();
	window_program = glCreateProgram();
	quad_program = glCreateProgram();
	shadow_program = glCreateProgram();

	/* Create customize shader by tell openGL specify shader type */
	// intial scene
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint vertexShader_model = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader_model = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint vertexShader_window = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader_window = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint vertexShader_quad = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader_quad = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint vertexShader_shadow = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader_shadow = glCreateShader(GL_FRAGMENT_SHADER);


	/* Load shader file */
	// initial scene shader
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");

	char** vertexShaderSource_model = loadShaderSource("vertex_model.vs.glsl");
	char** fragmentShaderSource_model = loadShaderSource("fragment_model.fs.glsl");

	char** vertexShaderSource_window = loadShaderSource("vertex1.vs.glsl");
	char** fragmentShaderSource_window = loadShaderSource("fragment1.fs.glsl");

	char** vertexShaderSource_quad = loadShaderSource("quad.vs.glsl");
	char** fragmentShaderSource_quad = loadShaderSource("quad.fs.glsl");

	char** vertexShaderSource_shadow = loadShaderSource("shadow.vs.glsl");
	char** fragmentShaderSource_shadow = loadShaderSource("shadow.fs.glsl");

	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	glShaderSource(vertexShader_model, 1, vertexShaderSource_model, NULL);
	glShaderSource(fragmentShader_model, 1, fragmentShaderSource_model, NULL);

	glShaderSource(vertexShader_window, 1, vertexShaderSource_window, NULL);
	glShaderSource(fragmentShader_window, 1, fragmentShaderSource_window, NULL);

	glShaderSource(vertexShader_quad, 1, vertexShaderSource_quad, NULL);
	glShaderSource(fragmentShader_quad, 1, fragmentShaderSource_quad, NULL);

	glShaderSource(vertexShader_shadow, 1, vertexShaderSource_shadow, NULL);
	glShaderSource(fragmentShader_shadow, 1, fragmentShaderSource_shadow, NULL);

	/* Free the shader file string(won't be used any more) */
	// intial scene
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	freeShaderSource(vertexShaderSource_model);
	freeShaderSource(fragmentShaderSource_model);

	freeShaderSource(vertexShaderSource_window);
	freeShaderSource(fragmentShaderSource_window);

	freeShaderSource(vertexShaderSource_quad);
	freeShaderSource(fragmentShaderSource_quad);

	freeShaderSource(vertexShaderSource_shadow);
	freeShaderSource(fragmentShaderSource_shadow);


	/* Compile these shaders */
	// intial scene
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	glCompileShader(vertexShader_model);
	glCompileShader(fragmentShader_model);

	glCompileShader(vertexShader_window);
	glCompileShader(fragmentShader_window);

	glCompileShader(vertexShader_quad);
	glCompileShader(fragmentShader_quad);

	glCompileShader(vertexShader_shadow);
	glCompileShader(fragmentShader_shadow);

	printGLShaderLog(vertexShader);
	printGLShaderLog(fragmentShader);

	printGLShaderLog(vertexShader_model);
	printGLShaderLog(fragmentShader_model);

	printGLShaderLog(vertexShader_window);
	printGLShaderLog(fragmentShader_window);

	printGLShaderLog(vertexShader_quad);
	printGLShaderLog(fragmentShader_quad);

	printGLShaderLog(vertexShader_shadow);
	printGLShaderLog(fragmentShader_shadow);

	/* Assign the program we created before with these shaders */
	// intial scene
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glAttachShader(model_program, vertexShader_model);
	glAttachShader(model_program, fragmentShader_model);
	glLinkProgram(model_program);

	glAttachShader(window_program, vertexShader_window);
	glAttachShader(window_program, fragmentShader_window);
	glLinkProgram(window_program);

	glAttachShader(quad_program, vertexShader_quad);
	glAttachShader(quad_program, fragmentShader_quad);
	glLinkProgram(quad_program);

	glAttachShader(shadow_program, vertexShader_shadow);
	glAttachShader(shadow_program, fragmentShader_shadow);
	glLinkProgram(shadow_program);

	glUseProgram(program);
}

void setUniform() {

	// cubemap uniform
	iLocP = glGetUniformLocation(program, "um4p");
	iLocV = glGetUniformLocation(program, "um4v");
	eye_pos = glGetUniformLocation(program, "eyePos");

	// model uniform
	model_m = glGetUniformLocation(model_program, "um4m");
	model_p = glGetUniformLocation(model_program, "um4p");
	model_v = glGetUniformLocation(model_program, "um4v");
	eye_pos_m = glGetUniformLocation(model_program, "eyePos");
	nano_tex = glGetUniformLocation(model_program, "shadow_tex");
	nano_skytex = glGetUniformLocation(model_program, "tex_cubemap");
	nano_shadow = glGetUniformLocation(model_program, "shadowM");

	// quad uniform
	quad_m = glGetUniformLocation(quad_program, "um4m");
	quad_p = glGetUniformLocation(quad_program, "um4p");
	quad_v = glGetUniformLocation(quad_program, "um4v");
	eye_pos_q = glGetUniformLocation(quad_program, "eyePos");
	quad_tex = glGetUniformLocation(quad_program, "shadow_tex");
	quad_shadow = glGetUniformLocation(quad_program, "shadowM");
	useShadow = glGetUniformLocation(quad_program, "shadowUse");

	// shadow uniform
	shadow_mvp = glGetUniformLocation(shadow_program, "um4mvp");


	// window uniform
	diffRender_Sobj = glGetUniformLocation(window_program, "tex_Sobj");
	diffRender_Snoobj = glGetUniformLocation(window_program, "tex_Snoobj");
	diffRender_Sb = glGetUniformLocation(window_program, "tex_Sb");
	switch_FBO = glGetUniformLocation(window_program, "switch_fbo");
}

void createFBO() {
	//Create FBO
	//glGenFramebuffers(1, &fbo_window);
	glGenFramebuffers(1, &fbo_Sobj);
	glGenFramebuffers(1, &fbo_Snoobj);
	glGenFramebuffers(1, &fbo_Sb);


	glGenFramebuffers(1, &shadow_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);

	glGenTextures(1, &fboDataTexture_shadow);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_shadow);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fboDataTexture_shadow, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &vao_window);
	glBindVertexArray(vao_window);

	glGenBuffers(1, &window_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, window_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_vertex), window_vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void loadCubeMap() {
	glGenTextures(1, &skybox_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);

	int width, height, nrChannels;
	for (int i = 0; i < 6; i++) {
		//TextureData tdata = loadImage(faces[i].c_str());
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (!data)
			cout << "error" << endl;
		else {
			cout << i << " " << width << endl;
			cout << i << " " << height << endl;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void My_Init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	createShader();
	initParameter();
	setUniform();
	createFBO();


	// load cubeMap
	loadCubeMap();
	glGenVertexArrays(1, &skybox_vao);


	// load model
	My_LoadModels("../Assets/nanosuit.obj");
	My_LoadModels2("../Assets/quad.obj");
}

void My_Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	mat4 model_matrix = translate(mat4(1.0f), vec3(-10.0f, -13.0f, -8.0f));
	mat4 Scale = scale(mat4(1.0), vec3(0.5, 0.35, 0.5));
	mat4 rotate_matrix;
	if (direction == 1)
		rotate_matrix = rotate(mat4(1.0), timer_cnt, vec3(0.0, 1.0, 0.0));
	else if (direction == 2)
		rotate_matrix = rotate(mat4(1.0), -timer_cnt, vec3(0.0, 1.0, 0.0));
	mat4 model_matrix_nano = model_matrix * rotate_matrix * Scale;

	mat4 model_matrix_q = translate(mat4(1.0f), vec3(-10.0f, -13.0f, 0.0f));
	Scale = scale(mat4(1.0), vec3(20.0f));
	mat4 model_matrix_quad = model_matrix_q * Scale;
	
	
	// shadow
	vec3 light_position = vec3(-31.75, 26.05, -97.72);
	const float shadow_range = 15.0f;
	mat4 light_proj_matrix = ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, 0.0f, 200.0f);
	mat4 light_view_matrix = lookAt(light_position, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;

	glEnable(GL_DEPTH_TEST);

	mat4 scale_bias_matrix = translate(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));
	mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

	glUseProgram(shadow_program);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 4096, 4096);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);
	glUniformMatrix4fv(shadow_mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * model_matrix_nano));
	glBindVertexArray(nano.vao);
	glDrawArrays(GL_TRIANGLES, 0, nano.vertexCount);
	glBindVertexArray(0);
	glUniformMatrix4fv(shadow_mvp, 1, GL_FALSE, value_ptr(light_vp_matrix * model_matrix_quad));
	glBindVertexArray(quad.vao);
	glDrawArrays(GL_TRIANGLES, 0, quad.vertexCount);
	glBindVertexArray(0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

	// render model (Sobj)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Sobj);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glViewport(0, 0, screenWidth, screenHeight);

	// enable stencil test
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(model_program);
	glUniform1i(nano_tex, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_shadow);

	glUniform1i(nano_skytex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skybox_tex);

	mat4 shadow_matrix = shadow_sbpv_matrix * model_matrix_nano;

	glUniformMatrix4fv(model_m, 1, GL_FALSE, &model_matrix_nano[0][0]);
	glUniformMatrix4fv(model_v, 1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(model_p, 1, GL_FALSE, &project_matrix[0][0]);
	glUniformMatrix4fv(nano_shadow, 1, GL_FALSE, value_ptr(shadow_matrix));
	glUniform3fv(eye_pos_m, 1, &main_camera.position[0]);
	glBindVertexArray(nano.vao);
	glDrawArrays(GL_TRIANGLES, 0, nano.vertexCount);


	// disable stencil test
	glStencilMask(0x00);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(0);
	glUseProgram(0);

	// quad model
	//glDisable(GL_DEPTH_TEST);
	glUseProgram(quad_program);
	glBindVertexArray(quad.vao);

	glUniform1i(useShadow, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_shadow);
	glUniform1i(quad_tex, 1);

	shadow_matrix = shadow_sbpv_matrix * model_matrix_quad;

	glUniformMatrix4fv(quad_m, 1, GL_FALSE, &model_matrix_quad[0][0]);
	glUniformMatrix4fv(quad_v, 1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(quad_p, 1, GL_FALSE, &project_matrix[0][0]);
	glUniformMatrix4fv(quad_shadow, 1, GL_FALSE, value_ptr(shadow_matrix));
	glUniform3fv(eye_pos_q, 1, &main_camera.position[0]);
	glDrawArrays(GL_TRIANGLES, 0, quad.vertexCount);
	//glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// render model (Snoobj)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Snoobj);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glViewport(0, 0, screenWidth, screenHeight);

	// enable stencil test
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(model_program);
	glUniform1i(nano_tex, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_shadow);

	glUniform1i(nano_skytex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skybox_tex);

	shadow_matrix = shadow_sbpv_matrix * model_matrix_nano;

	glUniformMatrix4fv(model_m, 1, GL_FALSE, &model_matrix_nano[0][0]);
	glUniformMatrix4fv(model_v, 1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(model_p, 1, GL_FALSE, &project_matrix[0][0]);
	glUniformMatrix4fv(nano_shadow, 1, GL_FALSE, value_ptr(shadow_matrix));
	glUniform3fv(eye_pos_m, 1, &main_camera.position[0]);
	glBindVertexArray(nano.vao);
	glDrawArrays(GL_TRIANGLES, 0, nano.vertexCount);

	// disable stencil test
	glStencilMask(0x00);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(0);
	glUseProgram(0);

	// quad model
	//glDisable(GL_DEPTH_TEST);
	glUseProgram(quad_program);
	glBindVertexArray(quad.vao);

	glUniform1i(useShadow, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_shadow);
	glUniform1i(quad_tex, 1);

	shadow_matrix = shadow_sbpv_matrix * model_matrix_quad;

	glUniformMatrix4fv(quad_m, 1, GL_FALSE, &model_matrix_quad[0][0]);
	glUniformMatrix4fv(quad_v, 1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(quad_p, 1, GL_FALSE, &project_matrix[0][0]);
	glUniformMatrix4fv(quad_shadow, 1, GL_FALSE, value_ptr(shadow_matrix));
	glUniform3fv(eye_pos_q, 1, &main_camera.position[0]);
	glDrawArrays(GL_TRIANGLES, 0, quad.vertexCount);
	//glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
	glUseProgram(0);



	// render model (Sb)
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_Sb);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glViewport(0, 0, screenWidth, screenHeight);

	// render cubemap
	glUseProgram(program);
	glBindVertexArray(skybox_vao);
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, &project_matrix[0][0]);
	glUniform3fv(eye_pos, 1, &main_camera.position[0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(0);
	glBindVertexArray(0);

	// render nano
	glUseProgram(model_program);
	glUniform1i(nano_tex, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_shadow);

	glUniform1i(nano_skytex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skybox_tex);

	shadow_matrix = shadow_sbpv_matrix * model_matrix_nano;

	glUniformMatrix4fv(model_m, 1, GL_FALSE, &model_matrix_nano[0][0]);
	glUniformMatrix4fv(model_v, 1, GL_FALSE, &view_matrix[0][0]);
	glUniformMatrix4fv(model_p, 1, GL_FALSE, &project_matrix[0][0]);
	glUniformMatrix4fv(nano_shadow, 1, GL_FALSE, value_ptr(shadow_matrix));
	glUniform3fv(eye_pos_m, 1, &main_camera.position[0]);
	glBindVertexArray(nano.vao);
	glDrawArrays(GL_TRIANGLES, 0, nano.vertexCount);

	glBindVertexArray(0);
	glUseProgram(0);



	// default framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindVertexArray(vao_window);
	glUseProgram(window_program);

	// choose framebuffer
	if (framebuffer == 0) {
		glUniform1i(switch_FBO, 0);
	}
	else if (framebuffer == 1) {
		glUniform1i(switch_FBO, 1);
	}
	else if (framebuffer == 2) {
		glUniform1i(switch_FBO, 2);
	}
	else if (framebuffer == 3) {
		glUniform1i(switch_FBO, 3);
	}

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(diffRender_Sobj, 0);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Sobj);
	glActiveTexture(GL_TEXTURE1);
	glUniform1i(diffRender_Snoobj, 1);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Snoobj);
	glActiveTexture(GL_TEXTURE2);
	glUniform1i(diffRender_Sb, 2);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Sb);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	proj.aspect = (float)(width) / (float)height;
	screenWidth = width;
	screenHeight = height;

	glViewport(0, 0, screenWidth, screenHeight);
	project_matrix = perspective(deg2rad(proj.fovy), proj.aspect, proj.nearClip, proj.farClip);
	view_matrix = lookAt(main_camera.position, main_camera.center, main_camera.up_vector);
	/* Sobj */
	// renew DEPTH_ATTACHMENT and COLOR_ATTACHMENT
	glDeleteRenderbuffers(1, &depthrbo_Sobj);
	glDeleteTextures(1, &fboDataTexture_Sobj);
	glGenRenderbuffers(1, &depthrbo_Sobj);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_Sobj);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenWidth);

	glGenTextures(1, &fboDataTexture_Sobj);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Sobj);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_Sobj);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthrbo_Sobj);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_Sobj, 0);

	/* Snoobj */
	// renew DEPTH_ATTACHMENT and COLOR_ATTACHMENT
	glDeleteRenderbuffers(1, &depthrbo_Snoobj);
	glDeleteTextures(1, &fboDataTexture_Snoobj);
	glGenRenderbuffers(1, &depthrbo_Snoobj);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_Snoobj);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenWidth);

	glGenTextures(1, &fboDataTexture_Snoobj);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Snoobj);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_Snoobj);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthrbo_Snoobj);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_Snoobj, 0);


	/* Sb */
	// renew DEPTH_ATTACHMENT and COLOR_ATTACHMENT
	glDeleteRenderbuffers(1, &depthrbo_Sb);
	glDeleteTextures(1, &fboDataTexture_Sb);
	glGenRenderbuffers(1, &depthrbo_Sb);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrbo_Sb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, screenWidth, screenWidth);

	glGenTextures(1, &fboDataTexture_Sb);
	glBindTexture(GL_TEXTURE_2D, fboDataTexture_Sb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_Sb);
	//Set depthrbo to current fbo
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrbo_Sb);
	//Set buffertexture to current fbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboDataTexture_Sb, 0);
}

void My_Timer(int val)
{
	timer_cnt += 0.05f;
	glutPostRedisplay();
	glutTimerFunc(timer_speed, My_Timer, val);
}

void My_Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
	}
	else if (state == GLUT_UP)
	{
		printf("Mouse %d is released at (%d, %d)\n", button, x, y);
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	if (key == 'i') {
		framebuffer += 1;
		if (framebuffer == 4)
			framebuffer = 0;
	}
	else if (key == 'q') {
		direction = 1;
	}
	else if (key == 'e') {
		direction = 2;
	}
	else if (key == 't') {
		direction = 3;
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	vec3 move;
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1440, 900);
	glutCreateWindow("AS3_Differential_Rendering"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	glPrintContextInfo();
	My_Init();

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0);

	// Enter main event loop.
	glutMainLoop();

	return 0;
}
