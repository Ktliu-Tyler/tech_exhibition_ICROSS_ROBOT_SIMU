# pragma once
# ifndef __GLViewer__
# define __GLViewer__
// GL
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <camera.h>
#include <shader.h>
#include "ft2build.h"
#include <vector>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


#include <opencv2/opencv.hpp>

#include <iostream>
#include <cmath>
#include <cstdlib>


#include FT_FREETYPE_H  

#include "Mesh_m.h"
#include "Model.h"



#include "ForceCalculate.hpp"
#include "Teleoperation.hpp"
#include "definition.h"
#include "Connecter.hpp"




std::string float2string(float, int);

struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

class font_proxy {
public:
	font_proxy() {};
	void init();
	std::map<GLchar, Character> Characters;

	void RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

	glm::mat4 projection;

	GLuint VAO;
	GLuint VBO;
private:
};

class useTexture {
public:
	useTexture() {};
	void init();
	void readTexture();
	void use();
	unsigned int ID1;
	cv::Mat image1;

private:

};

class Simple3DObject {
public:
	Simple3DObject();
	~Simple3DObject() {};

	void pushToGPU(int vertex_p, int color_p, int tex_p, int normal_p);
	// vertex ->3
	// color ->3
	// tex coord ->2
	// normal ->3

	GLuint VAO_;
	GLuint VBO_;
	std::vector<float> vertices_; //vectice-color-texture...
	int drawType_;
	GLenum isStatic = GL_STATIC_DRAW;

	void draw();

	glm::mat4 getTranslate();
	glm::mat4 getRotate();
	glm::mat4 getScale();
	glm::mat4 getModelMetrix();
	void setScale(glm::vec3);
	void setScale(glm::mat4);
	void setRotate(float, glm::vec3);
	void setRotate(glm::mat4);
	void setTranslate(glm::vec3);

	void addScale(glm::vec3);
	void addRotate(float, glm::vec3);
	void addTranslate(glm::vec3);

private:

	glm::vec3 position_;
	glm::mat4 translate_;
	glm::mat4 rotate_;
	glm::mat4 scale_;

};

class GLViewer {
public:
	GLViewer();
	~GLViewer() {};
	void releaseOpenGL() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void init();
	void draw();
	void setIn(glm::mat4 pose);
	void setIn(std::vector<glm::mat4> position, glm::mat4 model); //for draw the point
	void setIn(glm::vec3 force);
	void setIn(glm::mat4 pose, glm::mat4 model);
	void setIn(std::vector<std::string> text_list);
	void setIn(std::vector<glm::mat4>pose, std::vector<glm::mat4> model);


	glm::vec3 HSVtoRGB(glm::vec3 hsv);

	glm::mat4 joystick2screenTransformation(glm::mat4);

	//Camera camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f));
	//Camera camera = Camera(glm::vec3(0.0f, 0.0f, 7.0f));
	//Camera camera = Camera(0.0f, -90.0f, glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	Camera camera = Camera(glm::vec3(CAMERA_X, CAMERA_Y, CAMERA_Z));


	Camera screenshotCamera = Camera(glm::vec3(0.0f, 0.0f, 7.0f));

	GLFWwindow* window;
	GLFWwindow* window_minor;
	std::vector<Shader> shader_list;
	// objectShader textShader modelLoading colorChangeShader textureShader screenShader materialShader modelLoadingLight

	font_proxy font_proxy;
	float lastX;
	float lastY;
	bool firstMouse = true;
	bool is_pressed = false;

	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrame = 0.0f;
	float frame_rate;

	useTexture texture_success;
	useTexture texture_fail;
	useTexture image_texture;

	//void RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color, GLint, GLint);

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_btn_callback(GLFWwindow* window, int button, int action, int mods);
	void processInput(GLFWwindow* window);

	GLuint vertexArray = 0;
	GLuint setFramebuffer = 0;

	Simple3DObject createCube();
	Simple3DObject createTool();
	Simple3DObject createTube();
	Simple3DObject createGrid();
	Simple3DObject createPoint();
	Simple3DObject createLine();
	Simple3DObject createCircle();
	Simple3DObject createTube2();
	Simple3DObject createSkin();
	Simple3DObject createArrow();
	Simple3DObject createSquare();
	Simple3DObject createScreen();
	Simple3DObject createScreenSmall();

	void screenshot();




	static GLViewer* pThis;

	int SCR_WIDTH = WINDOW_WIDTH;//1600
	int SCR_HEIGHT = WINDOW_HEIGHT;

	int SETPOINT_WIDTH = 300;
	int SETPOINT_HEIGHT = 300;

	std::vector<glm::mat4> point_list;
	glm::vec3 force_mag;

	Model toolModel;
	Model touchModel;
	Model incision;
	Model PDA;
	Model Ball;

	Model toolModel_Ljaw;
	Model toolModel_Rjaw;

	std::vector<std::string> renderTextLlist;


	bool breakLoop = false;

	std::vector<glm::vec3> force_list;

	int frame = 0;

	bool pig_die = false;
	bool mission_complete = false;

	glm::mat4 model_tool = glm::mat4(1.0f);

	OutputLayer* pOutput;



private:



	Simple3DObject cube;
	// Simple3DObject tool;
	// Simple3DObject tube;
	Simple3DObject grid;
	Simple3DObject point;
	Simple3DObject line;
	Simple3DObject circle;
	// Simple3DObject skin;
	Simple3DObject arrow;
	Simple3DObject square;
	Simple3DObject screen;
	Simple3DObject light;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;

	glm::mat4 model_tool_pos = glm::mat4(1.0f);
	glm::mat4 model_tool_rot = glm::mat4(1.0f);

	// input data
	cv::Mat inputImage_;
	cv::Mat inputDepthmap_;
	float* input_jstk_info;

	//frame buffer
	unsigned int framebuffer, textureColorbuffer, rbo;
	unsigned int quadVAO, quadVBO;
	float quadVertices[24] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	//light-three point lighting
	glm::vec3 lightPosition[3] = {
		glm::vec3(-10.0f,0.0f,10.0f),//key light
		glm::vec3(10.0f,0.0f,10.0f),//fill light
		glm::vec3(-10.0f,0.0f,-10.0f)//back light
	};
	glm::vec3 lightColor[3] = {
		glm::vec3(0.0f,0.0f,0.0f),
		glm::vec3(0.0f,0.0f,0.0f),
		glm::vec3(0.0f,0.0f,0.0f)
	};


};

# endif
