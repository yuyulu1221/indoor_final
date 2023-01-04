#include "main.h"

#define STB_IMAGE_IMPLEMENTATION

//	timer
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2

//	exit
#define MENU_EXIT 3

using namespace glm;
using namespace std;

void My_Reshape(int, int);

// mouse trackball
vec2 mousePos; // for trackball
bool mouseUp = true;  // for trackball
float vHorizontal = radians(0.0f);
float vVertical = radians(0.0f);

//	about view
mat4 view_matrix;
mat4 proj_matrix;

vec3 viewEye = vec3(4.0f, 1.0f, -1.5f);
vec3 viewCenter = vec3(3.0f, 1.0f, -1.5f);
vec3 viewUp = vec3(0.0f, 1.0f, 0.0f);
int previousX = 0, previousY = 0;
static int targetAngle = 0;
static float viewRadius = 33.0f, targetHeight = -1.0f;

mat4 light_view_matrix;
mat4 light_proj_matrix;
mat4 scale_bias;
mat4 shadow_matrix;
//vec3 directLightVec = vec3(0.f, -1.f, 0.f);
//vec3 directLightVec = -normalize(vec3(-2.51449f, 0.477241f, -1.21263f));
vec3 directLightVec = normalize(vec3(0.542f, -0.141f, -0.422f));
//vec3 directLightPos = vec3(0.f) - 2.5f * directLightVec;
vec3 directLightPos = vec3(-2.845f, 2.028f, -1.293f);
float shadowRange = 5.f;

// pointLight Shadow
bool isPtLightShowed = false;
vec3 ptLightPos = vec3(1.87659f, 0.4625f, 0.103928f);
//vec3 ptLightPosition = vec3(2.f, 1.f, -1.5f);
vec4 ptLightColor = vec4(1.f);
mat4 ptLight_proj_matrix;

//	about timer
int timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

int ShadingCase = 0;
bool isBump = false;
bool isBloom = true;
bool isSSAO = true;
bool isFxaa = true;


float degToRad(float degree) {
	return degree * 3.1415926 / 180.0f;
}

const std::vector<std::string> split(const std::string& str) {

	string pattern = " ";
	std::vector<std::string> result;
	std::string::size_type begin, end;

	end = str.find(pattern);
	begin = 0;

	while (end != std::string::npos) {
		if (end - begin != 0) {
			result.push_back(str.substr(begin, end - begin));
		}
		begin = end + pattern.size();
		end = str.find(pattern, begin);
	}

	if (begin != str.length()) {
		result.push_back(str.substr(begin));
	}
	printf("%s\n", result[0]);

	//vec3 outResult = vec3(result[0], result[1], result[2]);
	return result;
}

void viewToward(vec3 eye, vec3 center) {

	vec3 deltaVector = center  - eye;
	targetAngle = asin(pow(pow(deltaVector.x,2)+pow(deltaVector.z, 2),0.5) / deltaVector.z);
	//targetHeight = deltaVector.y;
	//viewEye = eye;
	//viewCenter = center;
	//viewCenter = vec3(viewRadius * cos(degToRad(targetAngle)), targetHeight, viewRadius * sin(degToRad(targetAngle))) + viewEye;
	//view_matrix = lookAt(viewEye, viewCenter, viewUp);
}

// buffer -------------------------------------------------------------------------------------------------
void setFrameBuffer(int w, int h)
{
	setGBuffer(w, h);
	setShadowBuffer();
	setPtShadowBuffer();
	setDeferredBuffer(w, h);
	setBloomBuffer(w, h);
	setWindowBuffer(w, h);
}

void setGBuffer(int width, int height) {
	glGenFramebuffers(1, &gbuffer.fbo);

	glGenTextures(9, gbuffer.tex);

	//	vertex map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	normal map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[1]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	ambient map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[2]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	diffuse map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[3]);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	specular map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	emissive map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[5]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	shadow map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[6]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	ptShadow map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[7]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//	depth map
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[8]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gbuffer.tex[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gbuffer.tex[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, gbuffer.tex[2], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, gbuffer.tex[3], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, gbuffer.tex[4], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, gbuffer.tex[5], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, gbuffer.tex[6], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, gbuffer.tex[7], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gbuffer.tex[8], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setShadowBuffer() 
{
	glGenFramebuffers(1, &shadow_buffer.fbo);

	glGenTextures(1, &shadow_buffer.tex);

	glBindTexture(GL_TEXTURE_2D, shadow_buffer.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, shadow_buffer.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_buffer.tex, 0);
}

void setPtShadowBuffer()
{
	glGenFramebuffers(1, &ptshadow_buffer.fbo);

	glGenTextures(1, &ptshadow_buffer.tex);

	glBindTexture(GL_TEXTURE_CUBE_MAP, ptshadow_buffer.tex);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, ptshadow_buffer.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ptshadow_buffer.tex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setDeferredBuffer(int width, int height)
{
	glGenFramebuffers(1, &deferred_buffer.fbo);

	glGenTextures(3, deferred_buffer.tex);

	glBindTexture(GL_TEXTURE_2D, deferred_buffer.tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, deferred_buffer.tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, deferred_buffer.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, deferred_buffer.tex[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, deferred_buffer.tex[1], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setBloomBuffer(int width, int height) {
	glGenFramebuffers(1, &bloom_buffer.fbo);

	glGenTextures(1, &bloom_buffer.tex);

	glBindTexture(GL_TEXTURE_2D, bloom_buffer.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, bloom_buffer.fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, bloom_buffer.tex, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setWindowBuffer(int width, int height) {
	glGenFramebuffers(1, &window_buffer.fbo);

	glGenRenderbuffers(1, &window_buffer.rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, window_buffer.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, window_buffer.fbo);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, window_buffer.rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void bindGBuffer() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gbuffer.fbo);
	const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
									GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
									GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
	glDrawBuffers(8, draw_buffers);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.f);
}

void bindShadowBuffer() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, shadow_buffer.fbo);
	glDrawBuffer(GL_NONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.f);
}

void bindPtShadowBuffer() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ptshadow_buffer.fbo);
	glDrawBuffer(GL_NONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.f);
}

void bindDeferredBuffer() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, deferred_buffer.fbo);
	const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, draw_buffers);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.f);
}

void bindBloomBuffer() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloom_buffer.fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.f);
}

void bindWindowBuffer() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.f);
}

void framebufferReshape(int width, int height) {
	//printf("reshape\n");
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[5]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[6]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[7]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, gbuffer.tex[8]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, deferred_buffer.tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, deferred_buffer.tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, bloom_buffer.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindRenderbuffer(GL_RENDERBUFFER, window_buffer.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
}
// --------------------------------------------------------------------------------------------------------

// Program -------------------------------------------------------------------------------------------------
char** loadShaderSource(const char* file) {
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char** srcp = new char* [1];
	srcp[0] = src;
	return srcp;
}

void freeShaderSource(char** srcp) {
	delete[] srcp[0];
	delete[] srcp;
}

void setProgram(GLuint &program, const char* vShader, const char* fShader, const char* gShader) 
{
	program = glCreateProgram();

	// vertex shader
	if (vShader != nullptr) {
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		char** vs_sourse = loadShaderSource(vShader);
		glShaderSource(vs, 1, vs_sourse, NULL);
		freeShaderSource(vs_sourse);
		glCompileShader(vs);
		glAttachShader(program, vs);
	}
	else {
		//printf("Vertex shader: nullptr\n");
	}

	// fragment shader
	if (fShader != nullptr) {
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		char** fs_sourse = loadShaderSource(fShader);
		glShaderSource(fs, 1, fs_sourse, NULL);
		freeShaderSource(fs_sourse);
		glCompileShader(fs);
		glAttachShader(program, fs);
	}
	else {
		//printf("Fragment shader: nullptr\n");
	}

	// geometry shader
	if (gShader != nullptr) {
		GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
		char** gs_sourse = loadShaderSource(gShader);
		glShaderSource(gs, 1, gs_sourse, NULL);
		freeShaderSource(gs_sourse);
		glCompileShader(gs);
		glAttachShader(program, gs);
	}
	else {
		//printf("Geometry shader: nullptr\n");
	}
	
	glLinkProgram(program);
}

void loadScene() {
	loadObj("Grey_White_Room.obj", room_materials, room_shapes);
	loadObj("trice.obj", trice_materials, trice_shapes);
	loadObj("Sphere.obj", sphere_materials, sphere_shapes);
}

void loadObj(const char* path, vector<Material>& oMaterials, vector<Shape>& oShapes) {
	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_CalcTangentSpace);

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* aiMaterial = scene->mMaterials[i];
		aiColor3D color(1.f, 1.f, 1.f);
		Material material;
		aiString texturePath;

		//	load name
		if (aiMaterial->Get(AI_MATKEY_NAME, material.Name) == aiReturn_SUCCESS)
		{
			std::printf("material %d load %s \n", i, material.Name.C_Str());
		}
		else
		{
			material.Name.Set("NULL");
			std::printf("material %d load name fail\n", i);
		}

		//	load abient color
		if (aiMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) == aiReturn_SUCCESS)
		{
			material.Ka = glm::vec3(color.r, color.g, color.b);
			std::printf("material %d load ambient (%f, %f, %f) \n", i, material.Ka.x, material.Ka.y, material.Ka.z);
		}
		else
		{
			material.Ka = glm::vec3(0.f, 0.f, 0.f);
			std::printf("material %d load ambient fail\n", i);
		}

		//	load diffuse color
		if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS)
		{
			material.Kd = glm::vec3(color.r, color.g, color.b);
			std::printf("material %d load diffuse (%f, %f, %f) \n", i, material.Kd.x, material.Kd.y, material.Kd.z);
		}
		else
		{
			material.Kd = glm::vec3(0.f, 0.f, 0.f);
			std::printf("material %d load diffuse fail\n", i);
		}

		//	load specular color
		if (aiMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS)
		{
			material.Ks = glm::vec3(color.r, color.g, color.b);
			std::printf("material %d load specular (%f, %f, %f) \n", i, material.Ks.x, material.Ks.y, material.Ks.z);
		}
		else
		{
			material.Ks = glm::vec3(0.f, 0.f, 0.f);
			std::printf("material %d load specular fail\n", i);
		}

		//	load emissive
		if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color) == aiReturn_SUCCESS)
		{
			material.Ke = glm::vec3(color.r, color.g, color.b);
			std::printf("material %d load emissive (%f, %f, %f) \n", i, material.Ke.x, material.Ke.y, material.Ke.z);
		}
		else
		{
			material.Ke = glm::vec3(0.f, 0.f, 0.f);
			std::printf("material %d load emissive fail\n", i);
		}

		//	load shininess
		if (aiMaterial->Get(AI_MATKEY_SHININESS, material.Ns) == aiReturn_SUCCESS)
		{
			std::printf("material %d load shininess %f \n", i, material.Ns);
		}
		else
		{
			material.Ns = 0.f;
			std::printf("material %d load shininess fail\n", i);
		}

		//	load texture
		if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			material.isTextureAttached = true;
			//material.LoadTexture(texturePath.C_Str());
			material.texture = LoadImg(texturePath.C_Str());
			setTexture(material, TEX);
			std::printf("material %d load %s \n", i, texturePath.C_Str());
		}
		else
		{
			material.isTextureAttached = false;
			//material.LoadDefaultTexture();
			material.texture = LoadImg("NULL.png");
			setTexture(material, TEX);
			std::printf("material %d load default texture \n", i);
		}

		if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == aiReturn_SUCCESS)
		{
			material.isTextureNormal = true;
			//material.LoadTexture(texturePath.C_Str());
			material.texture = LoadImg(texturePath.C_Str());
			//material.SetTextureNormal();
			setTexture(material, NOR);
			std::printf("material %d load %s \n", i, texturePath.C_Str());
		}
		// ??????????????????
		else if (aiMaterial->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == aiReturn_SUCCESS)
		{
			material.isTextureNormal = true;
			//material.LoadTexture(texturePath.C_Str());
			material.texture = LoadImg(texturePath.C_Str());
			//material.SetTextureNormal();
			setTexture(material, NOR);
			
			std::printf("material %d load %s \n", i, texturePath.C_Str());
		}
		else
		{
			material.isTextureNormal = false;
			//material.LoadDefaultTexture();
			material.texture = LoadImg("NULL.png");
			//material.SetTextureNormal();
			setTexture(material, NOR);
			std::printf("material %d load default normals texture \n", i);
		}


		if (aiMaterial->GetTexture(aiTextureType_DISPLACEMENT, 0, &texturePath) == aiReturn_SUCCESS)
		{
			//material.LoadTexture(texturePath.C_Str());
			material.texture = LoadImg(texturePath.C_Str());
			//material.SetDispTexture();
			setTexture(material, DISP);
			std::printf("material %d load %s \n", i, texturePath.C_Str());
		}
		else
		{
			//material.LoadDefaultTexture();
			material.texture = LoadImg("NULL.png");
			//material.SetDispTexture();
			setTexture(material, DISP);
			std::printf("material %d load default displacement texture \n", i);
		}

		//material.Release();
		free((void*)material.texture.data);
		oMaterials.push_back(material);
		std::printf("--------------------------------------------\n");
	}

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[i];
		Shape shape;

		vector<float> vertices;
		vector<float> texcoords;
		vector<float> normals;
		vector<float> tangents;
		vector<float> bitangents;
		vector<unsigned int> indices;

		for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
		{
			vertices.push_back(mesh->mVertices[v].x);
			vertices.push_back(mesh->mVertices[v].y);
			vertices.push_back(mesh->mVertices[v].z);

			if (mesh->mTextureCoords[0] != nullptr)
			{
				texcoords.push_back(mesh->mTextureCoords[0][v].x);
				texcoords.push_back(mesh->mTextureCoords[0][v].y);
			}
			else
			{
				texcoords.push_back(0.f);
				texcoords.push_back(0.f);
			}

			normals.push_back(mesh->mNormals[v].x);
			normals.push_back(mesh->mNormals[v].y);
			normals.push_back(mesh->mNormals[v].z);

			if (mesh->HasTangentsAndBitangents())
			{
				tangents.push_back(mesh->mTangents[v].x);
				tangents.push_back(mesh->mTangents[v].y);
				tangents.push_back(mesh->mTangents[v].z);

				bitangents.push_back(mesh->mBitangents[v].x);
				bitangents.push_back(mesh->mBitangents[v].y);
				bitangents.push_back(mesh->mBitangents[v].z);
			}
			else
			{
				tangents.push_back(0.f);
				tangents.push_back(0.f);
				tangents.push_back(0.f);

				bitangents.push_back(0.f);
				bitangents.push_back(0.f);
				bitangents.push_back(0.f);
			}


		}

		for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
		{
			if (mesh->mFaces[f].mNumIndices == 3)
			{
				indices.push_back(mesh->mFaces[f].mIndices[0]);
				indices.push_back(mesh->mFaces[f].mIndices[1]);
				indices.push_back(mesh->mFaces[f].mIndices[2]);
			}
		}

		shape.materialID = mesh->mMaterialIndex;
		shape.drawCount = indices.size();
		//printf("part %d, material index = %d\n", i, mesh->mMaterialIndex);
		//printf("mtl ID = %d\n", sceneObj.materialID);

		printf("size of pt: %d\n", vertices.size());
		printf("size of tex: %d\n", texcoords.size());
		printf("size of nor: %d\n", normals.size());
		printf("size of tan: %d\n", tangents.size());
		printf("size of btan: %d\n", bitangents.size());

		glGenVertexArrays(1, &shape.vao);
		glBindVertexArray(shape.vao);

		glGenBuffers(1, &shape.vbo_vertex);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_vertex);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &shape.vbo_texcoord);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_texcoord);
		glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GL_FLOAT), texcoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glGenBuffers(1, &shape.vbo_normal);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_normal);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &shape.vbo_tangent);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_tangent);
		glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(GL_FLOAT), tangents.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);

		glGenBuffers(1, &shape.vbo_bitangent);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_bitangent);
		glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(GL_FLOAT), bitangents.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(4);

		glGenBuffers(1, &shape.ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GL_UNSIGNED_INT), indices.data(), GL_STATIC_DRAW);

		oShapes.push_back(shape);

		vertices.clear();
		vertices.shrink_to_fit();
		texcoords.clear();
		texcoords.shrink_to_fit();
		normals.clear();
		normals.shrink_to_fit();
		tangents.clear();
		tangents.shrink_to_fit();
		bitangents.clear();
		bitangents.shrink_to_fit();
		indices.clear();
		indices.shrink_to_fit();
	}

	aiReleaseImport(scene);
}

TextureData LoadImg(const char* path) {
	TextureData texture;
	int n;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(path, &texture.width, &texture.height, &n, 4);

	if (data != NULL)
	{
		texture.data = new unsigned char[texture.width * texture.height * 4 * sizeof(unsigned char)];
		memcpy(texture.data, data, texture.width * texture.height * 4 * sizeof(unsigned char));
		stbi_image_free(data);
	}
	else
	{
		printf("load img %s fail\n", path);
	}

	return texture;
}

void setTexture(Material& m, int mode) {
	if (!m.tex)
	{
		glGenTextures(1, &m.tex);
	}
	if (!m.nor)
	{
		glGenTextures(1, &m.nor);
	}
	if (!m.disp)
	{
		glGenTextures(1, &m.disp);
	}


	if (mode == TEX) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m.tex);
	}
	else if (mode == NOR) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m.nor);
	}
	else if (mode == DISP) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m.disp);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m.texture.width, m.texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m.texture.data);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void bindTexture(Material& m, int mode) {
	glActiveTexture(GL_TEXTURE0 + mode);
	if (mode == TEX) glBindTexture(GL_TEXTURE_2D, m.tex);
	else if (mode == NOR) glBindTexture(GL_TEXTURE_2D, m.nor);
	else if (mode == DISP) glBindTexture(GL_TEXTURE_2D, m.disp);
}

void loadDeferredCanvas() {
	glGenVertexArrays(1, &deferred_canvas.vao);
	glBindVertexArray(deferred_canvas.vao);

	glGenBuffers(1, &deferred_canvas.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, deferred_canvas.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_positions), window_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void loadBloomCanvas() {
	glGenVertexArrays(1, &bloom_canvas.vao);
	glBindVertexArray(bloom_canvas.vao);

	glGenBuffers(1, &bloom_canvas.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, bloom_canvas.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_positions), window_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

void loadScreen() {
	glGenVertexArrays(1, &screen.vao);
	glBindVertexArray(screen.vao);

	glGenBuffers(1, &screen.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, screen.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(window_positions), window_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
}

// Uniform ------------------------------------------------------------------------------------------------
void setSceneUniformLocation() {
	GLuint id = colormap_program;
	scene_uniform.isPtLightShowed = glGetUniformLocation(id, "isPtLightShowed");
	scene_uniform.tex_diffuse = glGetUniformLocation(id, "tex_diffuse");
	scene_uniform.tex_normal = glGetUniformLocation(id, "tex_normal");
	scene_uniform.tex_disp = glGetUniformLocation(id, "tex_disp");
	scene_uniform.tex_shadow = glGetUniformLocation(id, "tex_shadow");
	scene_uniform.tex_ptShadow = glGetUniformLocation(id, "tex_ptShadow");

	scene_uniform.isBump = glGetUniformLocation(id, "isBump");
	scene_uniform.isTexAttached = glGetUniformLocation(id, "isTexAttached");
	scene_uniform.isTextureNormal = glGetUniformLocation(id, "isTextureNormal");

	scene_uniform.um4m = glGetUniformLocation(id, "um4m");
	scene_uniform.um4v = glGetUniformLocation(id, "um4v");
	scene_uniform.um4p = glGetUniformLocation(id, "um4p");
	scene_uniform.shadow_matrix = glGetUniformLocation(id, "shadow_matrix");

	scene_uniform.u3fvKa = glGetUniformLocation(id, "u3fvKa");
	scene_uniform.u3fvKd = glGetUniformLocation(id, "u3fvKd");
	scene_uniform.u3fvKs = glGetUniformLocation(id, "u3fvKs");
	scene_uniform.u3fvKe = glGetUniformLocation(id, "u3fvKe");

	GLuint sId = shadowmap_program;
	scene_uniform.lightViewVP = glGetUniformLocation(sId, "lightViewVP");
	scene_uniform.lightViewM = glGetUniformLocation(sId, "lightViewM");

	GLuint ptSid = ptshadowmap_program;
	scene_uniform.ptLightPos = glGetUniformLocation(ptSid, "ptLightPos");
	scene_uniform.ptLightViewVP = glGetUniformLocation(ptSid, "ptLightViewVP");
	scene_uniform.ptLightViewM = glGetUniformLocation(ptSid, "ptLightViewM");

	glUseProgram(colormap_program);
	glUniform1i(scene_uniform.tex_diffuse, 0);
	glUniform1i(scene_uniform.tex_normal, 1);
	glUniform1i(scene_uniform.tex_disp, 2);
	glUniform1i(scene_uniform.tex_shadow, 3);
	glUniform1i(scene_uniform.tex_ptShadow, 4);
	glUseProgram(0);
}

void setDeferredUniformLocation() {
	GLuint id = deferred_program;
	deferred_uniform.shadingCase = glGetUniformLocation(id, "shadingCase");
	deferred_uniform.SSAOCase = glGetUniformLocation(id, "SSAOCase");
	deferred_uniform.directLightVec = glGetUniformLocation(id, "directLightVec");
	deferred_uniform.pointLightPosition = glGetUniformLocation(id, "pointLightPosition");
	deferred_uniform.eyePosition = glGetUniformLocation(id, "eyePosition");
	deferred_uniform.view_matirx = glGetUniformLocation(id, "view_matirx");
	deferred_uniform.proj_matrix = glGetUniformLocation(id, "proj_matrix");
	deferred_uniform.inv_proj_matrix = glGetUniformLocation(id, "inv_proj_matrix");
	deferred_uniform.noise_scale = glGetUniformLocation(id, "noise_scale");

	glUseProgram(deferred_program);
	glUniform1i(glGetUniformLocation(id, "position_map"), 0);
	glUniform1i(glGetUniformLocation(id, "normal_map"), 1);
	glUniform1i(glGetUniformLocation(id, "ambient_map"), 2);
	glUniform1i(glGetUniformLocation(id, "diffuse_map"), 3);
	glUniform1i(glGetUniformLocation(id, "specular_map"), 4);
	glUniform1i(glGetUniformLocation(id, "emissive_map"), 5);
	glUniform1i(glGetUniformLocation(id, "shadow_map"), 6);
	glUniform1i(glGetUniformLocation(id, "ptShadow_map"), 7);
	glUniform1i(glGetUniformLocation(id, "depth_map"), 8);
	glUniform1i(glGetUniformLocation(id, "noise_map"), 31);

	glUniform1i(deferred_uniform.shadingCase, 0);
}

void setBloomUniformLocation() {
	GLuint id = bloom_program;
	bloom_uniform.isBloom = glGetUniformLocation(id, "isBloom");
	glUseProgram(bloom_program);
	glUniform1i(glGetUniformLocation(id, "main_texture"), 0);
	glUniform1i(glGetUniformLocation(id, "hdr_texture"), 1);
}

void setWindowUniformLocation() {
	GLuint id = window_program;
	window_uniform.isFxaa = glGetUniformLocation(id, "isFxaa");
	glUseProgram(window_program);
	glUniform1i(glGetUniformLocation(id, "Texture"), 0);
}

void setLightVP(mat4 vp) {
	glUseProgram(shadowmap_program);
	glUniformMatrix4fv(scene_uniform.lightViewVP, 1, GL_FALSE, glm::value_ptr(vp));
	glUseProgram(0);
}

void setPtLightVP(vector<mat4> ptLightVP) {
	glUseProgram(ptshadowmap_program);
	glUniformMatrix4fv(scene_uniform.ptLightViewVP, ptLightVP.size(), GL_FALSE, reinterpret_cast<GLfloat*>(ptLightVP.data()));
	glUseProgram(0);
}
// ------------------------------------------------------------------------------------------------------

// Draw -------------------------------------------------------------------------------------------------
void drawShadowMap() {
	glUseProgram(shadowmap_program);
	glViewport(0, 0, 1024, 1024);
	for (unsigned int i = 0; i < room_shapes.size(); ++i)
	{
		glUniformMatrix4fv(scene_uniform.lightViewM, 1, GL_FALSE, glm::value_ptr(mat4(1.f)));
		glBindVertexArray(room_shapes[i].vao);
		glDrawElements(GL_TRIANGLES, room_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	mat4 m_trice = translate(mat4(1.f), vec3(2.05f, 0.628725f, -1.9f)) * scale(mat4(1.f), vec3(0.001f));
	for (unsigned int i = 0; i < trice_shapes.size(); ++i)
	{
		glUniformMatrix4fv(scene_uniform.lightViewM, 1, GL_FALSE, glm::value_ptr(m_trice));
		glBindVertexArray(trice_shapes[i].vao);
		glDrawElements(GL_TRIANGLES, trice_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	glUseProgram(0);
}

void drawPtShadowMap() {
	glUseProgram(ptshadowmap_program);
	glViewport(0, 0, 1024, 1024);
	glUniform3fv(glGetUniformLocation(ptshadowmap_program, "ptLightPos"), 1, glm::value_ptr(ptLightPos));

	for (unsigned int i = 0; i < room_shapes.size(); ++i)
	{
		glUniformMatrix4fv(scene_uniform.ptLightViewM, 1, GL_FALSE, glm::value_ptr(mat4(1.f)));
		glBindVertexArray(room_shapes[i].vao);
		glDrawElements(GL_TRIANGLES, room_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}

	mat4 m_trice = translate(mat4(1.f), vec3(2.05f, 0.628725f, -1.9f)) * scale(mat4(1.f), vec3(0.001f));
	for (unsigned int i = 0; i < trice_shapes.size(); ++i)
	{
		glUniformMatrix4fv(scene_uniform.ptLightViewM, 1, GL_FALSE, glm::value_ptr(m_trice));
		glBindVertexArray(trice_shapes[i].vao);
		glDrawElements(GL_TRIANGLES, trice_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	glUseProgram(0);
}

void drawPtLight() {
	glUseProgram(colormap_program);
	glViewport(0, 0, window_width, window_height);

	mat4 m_sphere = translate(mat4(1.f), ptLightPos) * scale(mat4(1.f),vec3(0.22f));
	for (unsigned int i = 0; i < sphere_shapes.size(); ++i)
	{
		int materialID = sphere_shapes[i].materialID;
		glUniformMatrix4fv(scene_uniform.um4m, 1, GL_FALSE, glm::value_ptr(m_sphere));
		glUniform1i(scene_uniform.isTexAttached, sphere_materials[materialID].isTextureAttached);
		glUniform1i(scene_uniform.isTextureNormal, sphere_materials[materialID].isTextureNormal);
		glUniform3fv(scene_uniform.u3fvKa, 1, glm::value_ptr(vec3(sphere_materials[materialID].Ka)));
		glUniform3fv(scene_uniform.u3fvKd, 1, glm::value_ptr(vec3(sphere_materials[materialID].Kd)));
		glUniform3fv(scene_uniform.u3fvKs, 1, glm::value_ptr(vec3(sphere_materials[materialID].Ks)));
		glUniform3fv(scene_uniform.u3fvKe, 1, glm::value_ptr(vec3(sphere_materials[materialID].Ke)));
		glBindVertexArray(sphere_shapes[i].vao);
		bindTexture(sphere_materials[materialID], TEX);
		glDrawElements(GL_TRIANGLES, sphere_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	glUseProgram(0);
}

void drawColorMap() {
	glUseProgram(colormap_program);
	glViewport(0, 0, window_width, window_height);
	glUniform3fv(glGetUniformLocation(colormap_program, "ptLightPos"), 1, glm::value_ptr(ptLightPos));

	for (unsigned int i = 0; i < room_shapes.size(); ++i)
	{
		int materialID = room_shapes[i].materialID;
		glUniformMatrix4fv(scene_uniform.um4m, 1, GL_FALSE, glm::value_ptr(mat4(1.f)));
		glUniform1i(scene_uniform.isTexAttached, room_materials[materialID].isTextureAttached);
		glUniform1i(scene_uniform.isTextureNormal, room_materials[materialID].isTextureNormal);
		glUniform3fv(scene_uniform.u3fvKa, 1, glm::value_ptr(vec3(room_materials[materialID].Ka)));
		glUniform3fv(scene_uniform.u3fvKd, 1, glm::value_ptr(vec3(room_materials[materialID].Kd)));
		glUniform3fv(scene_uniform.u3fvKs, 1, glm::value_ptr(vec3(room_materials[materialID].Ks)));
		glUniform3fv(scene_uniform.u3fvKe, 1, glm::value_ptr(vec3(room_materials[materialID].Ke)));
		glBindVertexArray(room_shapes[i].vao);
		//room_materials[materialID].BindTexture(0);
		bindTexture(room_materials[materialID], TEX);
		bindTexture(room_materials[materialID], NOR);
		bindTexture(room_materials[materialID], DISP);
		//room_materials[materialID].BindTextureNormal(1);
		//room_materials[materialID].BindDispTexture(2);
		glDrawElements(GL_TRIANGLES, room_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}

	mat4 m_trice = translate(mat4(1.f), vec3(2.05f, 0.628725f, -1.9f)) * scale(mat4(1.f), vec3(0.001f));
	for (unsigned int i = 0; i < trice_shapes.size(); ++i)
	{
		int materialID = trice_shapes[i].materialID;
		glUniformMatrix4fv(scene_uniform.um4m, 1, GL_FALSE, glm::value_ptr(m_trice));
		glUniform1i(scene_uniform.isTexAttached, trice_materials[materialID].isTextureAttached);
		glUniform1i(scene_uniform.isTextureNormal, trice_materials[materialID].isTextureNormal);
		glUniform3fv(scene_uniform.u3fvKa, 1, glm::value_ptr(vec3(trice_materials[materialID].Ka)));
		glUniform3fv(scene_uniform.u3fvKd, 1, glm::value_ptr(vec3(trice_materials[materialID].Kd)));
		glUniform3fv(scene_uniform.u3fvKs, 1, glm::value_ptr(vec3(trice_materials[materialID].Ks)));
		glUniform3fv(scene_uniform.u3fvKe, 1, glm::value_ptr(vec3(trice_materials[materialID].Ke)));
		glBindVertexArray(trice_shapes[i].vao);
		//trice_materials[materialID].BindTexture(0);
		//trice_materials[materialID].BindTextureNormal(1);
		//trice_materials[materialID].BindDispTexture(2);
		bindTexture(trice_materials[materialID], TEX);
		bindTexture(trice_materials[materialID], NOR);
		bindTexture(trice_materials[materialID], DISP);
		glDrawElements(GL_TRIANGLES, trice_shapes[i].drawCount, GL_UNSIGNED_INT, 0);
	}
	glUseProgram(0);
}

void drawDeferredChose() {
	glUseProgram(deferred_program);
	glViewport(0, 0, window_width, window_height);
	glBindVertexArray(deferred_canvas.vao);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}

void bindShadowTexture() {
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadow_buffer.tex);
}

void bindPtShadowTexture() {
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ptshadow_buffer.tex);
}
// ------------------------------------------------------------------------------------------------------


// SSBO--------------------------------------------------------------------------------------------------
void setKernalUbo() {
	glGenBuffers(1, &Ssao.kernal_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, Ssao.kernal_ubo);
	vec4 kernals[128];

	srand(time(NULL));

	for (int i = 0; i < 128; ++i)
	{
		float scale = i / 128.f;
		scale = 0.1f + 0.9f * scale * scale;
		kernals[i] = vec4(normalize(vec3(
			rand() / (float)RAND_MAX * 2.f - 1.f,
			rand() / (float)RAND_MAX * 2.f - 1.f,
			rand() / (float)RAND_MAX * 0.85f + 0.15f) * scale),
			0.f);
	}
	glBufferData(GL_UNIFORM_BUFFER, 128 * sizeof(vec4), &kernals[0][0], GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, Ssao.kernal_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void setNoiseMap() {
	vec3 noiseData[16];
	for (int i = 0; i < 16; ++i)
	{
		noiseData[i] = normalize(vec3(
			rand() / (float)RAND_MAX,
			rand() / (float)RAND_MAX,
			0.f));
	}
	glGenTextures(1, &Ssao.noise_map);
	glBindTexture(GL_TEXTURE_2D, Ssao.noise_map);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 4, 4, 0, GL_RGB, GL_FLOAT, &noiseData[0][0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glActiveTexture(GL_TEXTURE31);
	glBindTexture(GL_TEXTURE_2D, Ssao.noise_map);
	glActiveTexture(0);
}
// ------------------------------------------------------------------------------------------------------

// Main function
void My_Init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//framebuffer = new FrameBuffer(0, 0);
	setFrameBuffer(0, 0);

	//scene = new Scene();
	//deferred = new Deferred();
	//bloom = new Bloom();
	//window = new Window();
	
	//scene->Initialize();
	setProgram(colormap_program, "colormap.vs.glsl", "colormap.fs.glsl", nullptr);
	setProgram(shadowmap_program, "shadowmap.vs.glsl", "shadowmap.fs.glsl", nullptr);
	setProgram(ptshadowmap_program, "ptShadowmap.vs.glsl", "ptShadowmap.fs.glsl", "ptShadowmap.gs.glsl");
	setSceneUniformLocation();
	glUseProgram(colormap_program);
	loadScene();

	//deferred->Initialize();
	setProgram(deferred_program, "deferred.vs.glsl", "deferred.fs.glsl", nullptr);
	setDeferredUniformLocation();
	glUseProgram(deferred_program);
	loadDeferredCanvas();

	//bloom->Initialize();
	setProgram(bloom_program, "bloom.vs.glsl", "bloom.fs.glsl", nullptr);
	setBloomUniformLocation();
	glUseProgram(bloom_program);
	loadBloomCanvas();

	//window->Initialize();
	setProgram(window_program, "window.vs.glsl", "window.fs.glsl", nullptr);
	setWindowUniformLocation();
	glUseProgram(window_program);
	loadScreen();

	My_Reshape(600, 600);
	glUseProgram(0);
	glBindVertexArray(0);

	//viewToward(viewEye, viewCenter);

}

void My_Display()
{

	viewCenter = viewEye + vec3(cos(vVertical) * cos(vHorizontal), tan(vHorizontal), sin(vVertical) * cos(vHorizontal));
	view_matrix = lookAt(viewEye, viewCenter, viewUp);
	vector<mat4> ptLightVP;
	ptLightVP.push_back(ptLight_proj_matrix * lookAt(ptLightPos, ptLightPos + vec3(1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f)));
	ptLightVP.push_back(ptLight_proj_matrix * lookAt(ptLightPos, ptLightPos + vec3(-1.f, 0.f, 0.f), vec3(0.f, -1.f, 0.f)));
	ptLightVP.push_back(ptLight_proj_matrix * lookAt(ptLightPos, ptLightPos + vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f)));
	ptLightVP.push_back(ptLight_proj_matrix * lookAt(ptLightPos, ptLightPos + vec3(0.f, -1.f, 0.f), vec3(0.f, 0.f, -1.f)));
	ptLightVP.push_back(ptLight_proj_matrix * lookAt(ptLightPos, ptLightPos + vec3(0.f, 0.f, 1.f), vec3(0.f, -1.f, 0.f)));
	ptLightVP.push_back(ptLight_proj_matrix * lookAt(ptLightPos, ptLightPos + vec3(0.f, 0.f, -1.f), vec3(0.f, -1.f, 0.f)));

	//framebuffer->BindShadowFrame();
	bindShadowBuffer();
	//scene->SetLightVP(light_proj_matrix * light_view_matrix);
	setLightVP(light_proj_matrix * light_view_matrix);
	//scene->ShadowRenderPass();
	drawShadowMap();

	//framebuffer->BindPtLightShadowBuffer();
	bindPtShadowBuffer();
	if (isPtLightShowed) {
		//scene->SetPtLightVP(ptLightVP);
		setPtLightVP(ptLightVP);
		//scene->SetPtLightPos(ptLightPos);
		//scene->ptShadowRenderPass();
		drawPtShadowMap();
	}

	//framebuffer->BindGbufferFrame();
	bindGBuffer();
	//scene->BindShadowTexture(&framebuffer->shadowbuffer.tex);
	bindShadowTexture();
	//scene->BindPtShadowTexture(&framebuffer->ptLightShadowbuffer.tex);
	bindPtShadowTexture();

	//scene->SetVP(view_matrix, proj_matrix);
		//scene->SetVP(light_view_matrix, light_proj_matrix);
	glUseProgram(colormap_program);
	glUniformMatrix4fv(scene_uniform.um4v, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(scene_uniform.um4p, 1, GL_FALSE, glm::value_ptr(proj_matrix));
	glUniform1i(scene_uniform.isPtLightShowed, isPtLightShowed);
	//scene->SetBumpCase(isBump);
	glUniform1i(scene_uniform.isBump, isBump);
	//scene->SetShadowMatrix(shadow_matrix);
	glUniformMatrix4fv(scene_uniform.shadow_matrix, 1, GL_FALSE, glm::value_ptr(shadow_matrix));
	glUseProgram(0);

	if (isPtLightShowed) {
		//scene->RenderPointLight();
		drawPtLight();
	}
	//scene->SpecColorRenderPass();
	drawColorMap();

	//framebuffer->BindDeferredBufferFrame();
	bindDeferredBuffer();
	//deferred->BindSpecTexture(framebuffer->gbuffer.tex, 9);
	glUseProgram(deferred_program);
	for (unsigned int i = 0; i < 9; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gbuffer.tex[i]);
	}
	//deferred->SetShadingCase(ShadingCase);
	glUseProgram(deferred_program);
	glUniform1i(deferred_uniform.shadingCase, ShadingCase);
	//deferred->SetSSAOCase(isSSAO);
	glUniform1i(deferred_uniform.SSAOCase, isSSAO);
	//deferred->SetDirectLightVec(directLightVec);
	glUniform3fv(deferred_uniform.directLightVec, 1, glm::value_ptr(directLightVec));
	//deferred->SetPointLightPosition(ptLightPos);
	glUniform3fv(deferred_uniform.pointLightPosition, 1, glm::value_ptr(ptLightPos));
	//deferred->SetEyePosition(viewEye);
	glUniform3fv(deferred_uniform.eyePosition, 1, glm::value_ptr(viewEye));
	//deferred->SetVP(view_matrix, proj_matrix);
	glUniformMatrix4fv(deferred_uniform.view_matirx, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(deferred_uniform.proj_matrix, 1, GL_FALSE, glm::value_ptr(proj_matrix));
	glUniformMatrix4fv(deferred_uniform.inv_proj_matrix, 1, GL_FALSE, glm::value_ptr(inverse(proj_matrix)));
	glUseProgram(0);
	//deferred->RenderPass();
	drawDeferredChose();

	//framebuffer->BindBloomBufferFrame();
	bindBloomBuffer();
	//bloom->BindTexture(framebuffer->deferredbuffer.tex, 2);
	for (unsigned int i = 0; i < 2; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, deferred_buffer.tex[i]);
	}
	//bloom->SetBloomCase(isBloom);
	glUseProgram(bloom_program);
	glUniform1i(bloom_uniform.isBloom, isBloom);
	//bloom->RenderPass();
	glViewport(0, 0, window_width, window_height);
	glBindVertexArray(bloom_canvas.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glUseProgram(0);

	//framebuffer->BindWindowBufferFrame();
	bindWindowBuffer();
	//window->BindTexture(&framebuffer->bloombuffer.tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bloom_buffer.tex);
	//window->SetFxaa(isFxaa);
	glUseProgram(window_program);
	glUniform1i(window_uniform.isFxaa, isFxaa);
	//window->RenderPass();
	glViewport(0, 0, window_width, window_height);
	glBindVertexArray(screen.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	float viewportAspect = (float)width / (float)height;
	//framebuffer->Resize(width, height);
	framebufferReshape(width, height);
	//scene->SetViewport(width, height);
	//deferred->SetViewport(width, height);
	//deferred->SetNoiseScale(vec2((float)width/4.f, (float)height/4.f));
	glUseProgram(deferred_program);
	glUniform2fv(deferred_uniform.noise_scale, 1, glm::value_ptr(vec2((float)width / 4.f, (float)height / 4.f)));
	//deferred->ssao->SetKernalUbo();
	setKernalUbo();
	//deferred->ssao->SetNoiseMap();
	setNoiseMap();
	//bloom->SetViewport(width, height);
	//window->SetViewport(width, height);


	window_width = width;
	window_height = height;

	//printf("reshape");

	// camara view
	proj_matrix = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);
	view_matrix = lookAt(viewEye, viewCenter, viewUp);

	// light view
	light_proj_matrix = ortho(-shadowRange, shadowRange, -shadowRange, shadowRange, 0.1f, 100.f);
	//light_view_matrix = lookAt(directLightPos, vec3(0.f), viewUp);
	light_view_matrix = lookAt(directLightPos, directLightPos + directLightVec, viewUp);
	scale_bias = translate(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));
	shadow_matrix = scale_bias * light_proj_matrix * light_view_matrix;

	//	point light view
	ptLight_proj_matrix = perspective(glm::radians(90.0f), 1.f, 0.22f, 10.f);
}

void My_Timer(int val)
{
	timer_cnt++;
	glutPostRedisplay();
	if (timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
	}
}

void My_Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_UP)
			mouseUp = true;
	}
}

void My_Motion(int x, int y) {


	if (x > 0.0f && abs(x) < glutGet(GLUT_WINDOW_WIDTH) && y > 0.0f && abs(y) < glutGet(GLUT_WINDOW_HEIGHT))
	{
		vec2 temp = vec2(x, y) - mousePos;

		if (mouseUp == false) {
			vVertical -= radians(temp.x) * 0.2f;
			vHorizontal += radians(temp.y) * 0.2f;

			if (vHorizontal > radians(89.0f))
				vHorizontal = radians(89.0f);
			if (vHorizontal < radians(-89.0f))
				vHorizontal = radians(-89.0f);
		}
		mousePos = vec2(x, y);
		mouseUp = false;
	}

}

void My_Keyboard(unsigned char key, int x, int y)
{
	float length = 0.05f;

	switch (key)
	{
		// eye move
		case 'W': case 'w':
			viewEye += normalize(viewCenter - viewEye) * length;
			viewCenter += normalize(viewCenter - viewEye) * length;
			//viewToward(viewEye, viewCenter);
			break;
		case 'S': case 's':
			viewEye -= normalize(viewCenter - viewEye) * length;
			viewCenter -= normalize(viewCenter - viewEye) * length;
			break;
		case 'A': case 'a':
			viewEye += normalize(cross(viewUp, (viewCenter - viewEye))) * length;
			viewCenter += normalize(cross(viewUp, (viewCenter - viewEye))) * length;
			break;
		case 'D': case 'd':
			viewEye -= normalize(cross(viewUp, (viewCenter - viewEye))) * length;
			viewCenter -= normalize(cross(viewUp, (viewCenter - viewEye))) * length;
			break;
		case 'Q': case 'q':
			viewEye += viewUp * length;
			break;
		case 'E': case 'e':
			viewEye -= viewUp * length;
			break;
		// pt light move
		case 'I': case 'i':
			if (isPtLightShowed) 
				ptLightPos -= vec3(1.0, 0.0, 0.0) * length;
			break;
		case 'K': case 'k':
			if (isPtLightShowed) 
				ptLightPos += vec3(1.0, 0.0, 0.0) * length;
			break;
		case 'J': case 'j':
			if (isPtLightShowed) 
				ptLightPos += vec3(0.0, 0.0, 1.0) * length;
			break;
		case 'L': case 'l':
			if (isPtLightShowed) 
				ptLightPos -= vec3(0.0, 0.0, 1.0) * length;
			break;
		case 'U': case 'u':
			if (isPtLightShowed) 
				ptLightPos += vec3(0.0, 1.0, 0.0) * length;
			break;
		case 'O': case 'o':
			if(isPtLightShowed) 
				ptLightPos -= vec3(0.0, 1.0, 0.0) * length;
			break;
		case 'P': case 'p':
			isPtLightShowed = !isPtLightShowed;
			break;
		// change texture
		case 'Z': case 'z':
			ShadingCase += 1;
			ShadingCase %= 7;
			break;
		case 'X': case 'x':
			isBump = !isBump;
			break;
		case 'C': case 'c':
			isBloom = !isBloom;
			break;
		case 'V': case 'v':
			isSSAO = !isSSAO;
			break;
		case 'B': case 'b':
			isFxaa = !isFxaa;
			break;
		default:
			printf("Key %c is pressed at (%d, %d)\n", key, x, y);
			break;
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch(id)
	{
	case MENU_TIMER_START:
		if(!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
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
	glutInitWindowSize(800, 600);
	glutCreateWindow("Final_Indoor"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	dumpInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	int menu_comparison = glutCreateMenu(My_Menu);
	int menu_isNormal = glutCreateMenu(My_Menu);
	int menu_shader = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutMotionFunc(My_Motion);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0); 

	glutMainLoop();

	

	return 0;
}
