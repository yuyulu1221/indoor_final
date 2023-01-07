#pragma once

#include "../Externals/Include/GLEW/glew.h"

#include "../Externals/Include/GLM/glm.hpp"
#include "../Externals/Include/GLM/gtc/type_ptr.hpp"
//#include "../Externals/Include/GLM/gtc/type_ptr.inl"
#include "../Externals/Include/FreeGLUT/freeglut.h"

#include "../Externals/Include/assimp/scene.h"
#include "../Externals/Include/assimp/cimport.h"
#include "../Externals/Include/assimp/postprocess.h"

#include "../Externals/Include/STB/stb_image.h"
#include "../Externals/Include/assimp/types.h"
#include "../Externals/Include/GLM/vec3.hpp"

#include "../Externals/Include/imgui/imgui.h"
#include "../Externals/Include/imgui/backends/imgui_impl_glut.h"

#include "../Externals/Include/Common.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
/////////////////////////////////////////
//#define GL_SILENCE_DEPRECATION
//#ifdef _MSC_VER
//#pragma warning (disable: 4505) // unreferenced local function has been removed
//#endif
/////////////////////////////////////////
//// Our state
//static bool show_demo_window = true;
//static bool show_another_window = false;
//static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

using namespace glm;
using namespace std;

enum { TEX, NOR, DISP };

static const GLfloat window_positions[] =
{
	1.0f,-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,0.0f,
	-1.0f,1.0f,0.0f,1.0f,
	1.0f,1.0f,1.0f,1.0f
};

typedef struct _TextureData
{
	//_TextureData() : width(0), height(0), data(0) {}
	int width;
	int height;
	unsigned char* data;
} TextureData;

typedef struct _Material
{
	aiString Name;
	glm::vec3 Ka;
	glm::vec3 Kd;
	glm::vec3 Ks;
	glm::vec3 Ke;
	float Ns;
	bool isTextureAttached;
	bool isTextureNormal;
	GLuint tex = NULL;
	GLuint nor = NULL;
	GLuint disp = NULL;
	TextureData texture;
} Material;

typedef struct _Shape
{
	GLuint vao;
	GLuint vbo;
	GLuint vbo_vertex;
	GLuint vbo_texcoord;
	GLuint vbo_normal;
	GLuint vbo_tangent;
	GLuint vbo_bitangent;
	GLuint ebo;
	int drawCount;
	int materialID;
} Shape;

struct VertexAL {
	vec3 position;
	vec3 normal;
	vec2 texcoord;
};

typedef struct _GBuffer
{
	GLuint fbo;
	GLuint tex[9];
} GBuffer;

typedef struct _ShadowBuffer
{
	GLuint fbo;
	GLuint tex;
} SBuffer;

typedef struct _DeferredBuffer
{
	GLuint fbo;
	GLuint tex[2];
} DBuffer;

typedef struct _BloomBuffer
{
	GLuint fbo;
	GLuint tex;
}BBuffer;

typedef struct _WindowBuffer
{
	GLuint fbo;
	GLuint rbo;
} WBuffer;

typedef struct _TestBuffer
{
	GLuint fbo;
	GLuint rbo;
	GLuint tex;
} TBuffer;

// scene uniform
typedef struct _SceneUniform 
{
	GLint tex_diffuse;
	GLint tex_normal;
	GLint tex_disp;
	GLint tex_shadow;
	GLint tex_ptShadow;
	GLint shadow_matrix;
	GLint isTexAttached;
	GLint isTextureNormal;
	GLint isBump;

	GLint um4m;
	GLint um4v;
	GLint um4p;

	GLint u3fvKd;
	GLint u3fvKa;
	GLint u3fvKs;
	GLint u3fvKe;

	GLint lightViewVP;
	GLint lightViewM;

	GLint ptLightPos;
	GLint ptLightTex;
	GLint ptLightViewVP;
	GLint ptLightViewM;
	
	GLint isAreaLightShowed;
	GLint areaLightPos;
	GLint areaLightDir;
	GLint areaLightViewVP;
	GLint areaLightViewM;

} SUniform;

// deferred uniform
typedef struct _DeferredUniform {
	GLint shadingCase;
	GLint SSAOCase;
	GLint directLightVec;
	GLint isPtLightShowed;
	GLint pointLightPosition;
	GLint isAreaLightShowed;
	GLint areaLightPosition;
	GLint areaLightDir;
	GLint eyePosition;
	GLint eyeCenter;
	GLint view_matrix;
	GLint proj_matrix;
	GLint inv_proj_matrix;
	GLint noise_scale;
} DUniform;

// bloom uniform
typedef struct _BloomUniform {
	GLint isBloom;
} BUniform;

// window uniform
typedef struct _WindowUniform {
	GLint isFxaa;
} WUniform;

typedef struct _SsaoHandle {
	GLuint kernal_ubo;
	GLuint noise_map;
} SsaoHandle;


int window_width = 1200;
int window_height = 700;

// framebuffer
GBuffer gbuffer;
SBuffer shadow_buffer;
SBuffer ptshadow_buffer;
SBuffer areashadow_buffer;
DBuffer deferred_buffer;
BBuffer bloom_buffer;
WBuffer window_buffer;

// texture program
GLuint colormap_program;
GLuint shadowmap_program;
GLuint ptshadowmap_program;
GLuint areashadowmap_program;
GLuint deferred_program;
GLuint bloom_program;
GLuint window_program;

SUniform scene_uniform;
DUniform deferred_uniform;
BUniform bloom_uniform;
WUniform window_uniform;

SsaoHandle Ssao;

vector<Material> room_materials;
vector<Shape> room_shapes;
vector<Material> trice_materials;
vector<Shape> trice_shapes;
vector<Material> sphere_materials;
vector<Shape> sphere_shapes;
vector<Material> plane_materials;
vector<Shape> plane_shapes;

Shape area;

Shape deferred_canvas;
Shape bloom_canvas;
Shape screen;

// area light
//const GLfloat psize = 10.0f;
//VertexAL planeVertices[6] = {
//	{ {-psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
//	{ {-psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
//	{ { psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
//	{ {-psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
//	{ { psize, 0.0f,  psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
//	{ { psize, 0.0f, -psize}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} }
//};
//VertexAL areaLightVertices[6] = {
//	{ {-8.0f, 2.4f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, // 0 1 5 4
//	{ {-8.0f, 2.4f,  1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
//	{ {-8.0f, 0.4f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
//	{ {-8.0f, 2.4f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
//	{ {-8.0f, 0.4f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
//	{ {-8.0f, 0.4f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }
//};

// buffer -------------------------------------------------------------------------------------------------
void setFrameBuffer(int, int);

void setGBuffer(int, int);

void setShadowBuffer();

void setPtShadowBuffer();

void setDeferredBuffer(int, int);

void setBloomBuffer(int, int);

void setWindowBuffer(int, int);

void bindGBuffer();

void bindShadowBuffer();

void bindPtShadowBuffer();

void bindDeferredBuffer();

void bindBloomBuffer();

void bindWindowBuffer();

void framebufferReshape(int, int);
// --------------------------------------------------------------------------------------------------------


// Program -------------------------------------------------------------------------------------------------
char** loadShaderSource(const char*);

void freeShaderSource(char**);

void setProgram(GLuint&, const char*, const char*, const char*);

void loadScene();

void loadObj(const char*, vector<Material>&, vector<Shape>&);

TextureData LoadImg(const char*);

void setTexture(Material&, int);

void bindTexture(Material&, int);

void loadDeferredCanvas();

void loadBloomCanvas();

void loadScreen();
// --------------------------------------------------------------------------------------------------------


// Uniform ------------------------------------------------------------------------------------------------
void setSceneUniformLocation();

void setDeferredUniformLocation();

void setBloomUniformLocation();

void setWindowUniformLocation();

void setLightVP(mat4);

void setPtLightVP(std::vector<mat4>);
// ------------------------------------------------------------------------------------------------------



// Draw -------------------------------------------------------------------------------------------------
void drawShadowMap();

void drawPtShadowMap();

void drawPtLightSource();

void drawAreaLightSource();

void drawColorMap();

void drawDeferredChose();

void bindShadowTexture();

void bindPtShadowTexture();
// ------------------------------------------------------------------------------------------------------


// SSBO--------------------------------------------------------------------------------------------------
void setKernalUbo();

void setNoiseMap();
// ------------------------------------------------------------------------------------------------------


// Main function -----------------------------------------------------------------------------------------
void My_Init();

void My_Display();

void My_Reshape(int, int);

void My_Timer(int);

void My_Mouse(int, int, int, int);

void My_Motion(int, int);

void My_Keyboard(unsigned char, int, int);

void My_SpecialKeys(int, int, int);

void My_Menu(int);
// ------------------------------------------------------------------------------------------------------
