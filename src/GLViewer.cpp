#include "GLViewer.hpp"



std::string float2string(float f, int round = 3) {
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(round) << f;
	std::string str = oss.str();
	return str;
}

glm::mat4 calculateRotationMatrix(const glm::vec3& from, const glm::vec3& to) {
	// 步驟1：計算旋轉軸和旋轉角
	glm::vec3 axis = glm::cross(from, to);
	float angle = acos(glm::dot(from, to) / (glm::length(from) * glm::length(to)));

	// 如果兩個向量平行，則旋轉軸可能為零向量
	if (glm::length(axis) < 1e-6) {
		// 如果向量方向相同，返回單位矩陣
		if (glm::dot(from, to) > 0) {
			return glm::mat4(1.0f);
		}
		else {
			// 如果向量方向相反，找到一個正交的旋轉軸
			axis = glm::vec3(1.0f, 0.0f, 0.0f);
			if (std::abs(from.x) > std::abs(from.z)) {
				axis = glm::vec3(0.0f, 1.0f, 0.0f);
			}
		}
	}

	axis = glm::normalize(axis);

	// 步驟2：生成旋轉矩陣
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);

	return rotationMatrix;
}

glm::mat4 computeRotationMatrix(const glm::vec3& from, const glm::vec3& to) {
	// 
	glm::vec3 axis = glm::cross(from, to);
	float angle = glm::acos(glm::dot(glm::normalize(from), glm::normalize(to)));

	// 
	if (glm::length(axis) < 1e-6) {
		return glm::mat4(1.0f);
	}

	axis = glm::normalize(axis);
	glm::quat quaternion = glm::angleAxis(angle, axis);

	// 
	glm::mat4 rotationMatrix = glm::toMat4(quaternion);
	return rotationMatrix;
}

glm::mat4 vec32vec3_getRotationMatrix(const glm::vec3& src, const glm::vec3& dst) {
	glm::vec3 v = glm::cross(src, dst);
	float s = glm::length(v);
	float c = glm::dot(src, dst);

	glm::mat4 vx = glm::mat4(
		0.0f, v.z, -v.y, 0.0f,
		-v.z, 0.0f, v.x, 0.0f,
		v.y, -v.x, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	return glm::mat4(1.0f) + vx + (vx * vx) * ((1.0f - c) / (s * s));
}

glm::mat4 vec32vec3_getScalingMatrix(const glm::vec3& src, const glm::vec3& dst) {
	float scaleFactors = length(dst) / length(src);
	glm::mat4 output;

	return glm::scale(output, glm::vec3(scaleFactors, scaleFactors, scaleFactors));
}


GLViewer* GLViewer::pThis = NULL;

GLViewer::GLViewer() {
	pThis = this;
}



void GLViewer::init() {
	//lastX = SCR_WIDTH / 2.0f;
	//lastY = SCR_HEIGHT / 2.0f;
	//firstMouse = true;
	//scaleFactor = 1.0f;
	//is_pressed = false;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "haptic simulation v2", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_btn_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// build and compile our shader zprogram
	// ------------------------------------
	shader_list.push_back(Shader("..\\src\\shader\\objectShader.vs", "..\\src\\shader\\objectShader.fs"));
	shader_list.push_back(Shader("..\\src\\shader\\textShader.vs", "..\\src\\shader\\textShader.fs"));
	shader_list.push_back(Shader("..\\src\\shader\\modelLoading.vs", "..\\src\\shader\\modelLoading.fs"));
	shader_list.push_back(Shader("..\\src\\shader\\colorChangeShader.vs", "..\\src\\shader\\colorChangeShader.fs"));
	shader_list.push_back(Shader("..\\src\\shader\\textureShader.vs", "..\\src\\shader\\textureShader.fs"));
	shader_list.push_back(Shader("..\\src\\shader\\screenShader.vs", "..\\src\\shader\\screenShader.fs"));
	shader_list.push_back(Shader("..\\src\\shader\\materialShader.vs", "..\\src\\shader\\materialShader.fs"));
	shader_list.push_back(Shader("..\\src\\shader\\modelLoadingLightAlpha.vs", "..\\src\\shader\\modelLoadingLightAlpha.fs"));


	cube = createCube();
	cube.pushToGPU(3, 3, 3, 0);

	grid = createGrid();
	grid.pushToGPU(3, 0, 3, 0);

	point = createPoint();
	point.pushToGPU(3, 0, 3, 0);

	line = createLine();
	line.pushToGPU(3, 0, 3, 0);

	circle = createCircle();
	circle.pushToGPU(3, 0, 3, 0);

	arrow = createArrow();
	arrow.pushToGPU(3, 0, 3, 0);

	square = createSquare();
	square.pushToGPU(3, 0, 3, 0);

	screen = createScreenSmall();
	screen.pushToGPU(3, 0, 0, 2);

	light = createCube();
	light.pushToGPU(3, 3, 3, 0);

	texture_success.init();
	texture_success.image1 = cv::imread("..\\src\\image\\complete.jpg");
	texture_success.readTexture();

	texture_fail.init();
	texture_fail.image1 = cv::imread("..\\src\\image\\die.jpg");
	texture_fail.readTexture();


	shader_list[1].use();
	font_proxy.init();



	incision = Model(("..\\src\\model\\PDA_noPDA_3\\PDA_noPDA_3_thin.obj"));
	PDA = Model(("..\\src\\model\\PDA_with_aorta_no_PA_2\\PDA_with_aorta_no_PA_2_new.obj"));
	//Ball = Model(("..\\src\\model\\TestBall\\TestBall.obj"));

	toolModel = Model(("..\\src\\model\\forceps\\shaft.obj"));
	toolModel_Rjaw = Model(("..\\src\\model\\forceps\\forceps_left_jaw.obj"));
	toolModel_Ljaw = Model(("..\\src\\model\\forceps\\forceps_right_jaw.obj"));


	shader_list[5].use();
	shader_list[5].setInt("screenTexture", 0);

	// screen quad VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	// framebuffer configuration
	// -------------------------
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return;
}



void GLViewer::draw() {

	frame = frame + 1;
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	frame_rate = 1 / deltaTime;

	// input
	// -----
	processInput(window);
	// render
	// ------

	// bind to framebuffer and draw scene as we normally would to color texture
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Off-screen Rendering
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//glClearColor(0.87f, 1.0f, 1.0f, 1.0f);
	//glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec4 inputcolor;

	// light shader
	shader_list[7].use();
	shader_list[7].setInt("material.diffuse", 0);
	glm::vec3 lightPos(CAMERA_X, CAMERA_Y, CAMERA_Z);
	shader_list[7].setVec3("light.position", lightPos);
	shader_list[7].setVec3("viewPos", camera.Position);
	// light properties
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
	glm::vec3 ambientColor = diffuseColor * glm::vec3(1.0f); // low influence
	shader_list[7].setVec3("light.ambient", ambientColor);
	shader_list[7].setVec3("light.diffuse", diffuseColor);
	shader_list[7].setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	// material properties
	shader_list[7].setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
	shader_list[7].setFloat("material.shininess", 64.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// save the image then the alpha of tool = 0.5
	// but the user would see 1.0
	double ToolAlpha;

	ToolAlpha = 0.5;


	// Object
	shader_list[0].use();
	projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	shader_list[0].setMat4("projection", projection);
	view = camera.GetViewMatrix();


	shader_list[0].setMat4("view", view);


	shader_list[7].use();
	shader_list[7].setFloat("alpha", 1.0f);
	shader_list[7].setMat4("projection", projection);
	shader_list[7].setMat4("view", view);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(CAMERA_X, CAMERA_Y, 250));
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 3.0f));
	shader_list[7].setMat4("model", model);
	incision.Draw(shader_list[7]);



	// PDA
	// 
	shader_list[7].use();
	shader_list[7].setFloat("alpha", 1.0f);
	shader_list[7].setMat4("projection", projection);
	shader_list[7].setMat4("view", view);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(INITIALTARGET_X, INITIALTARGET_Y, INITIALTARGET_Z));


	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
	shader_list[7].setMat4("model", model);
	PDA.Draw(shader_list[7]);



	// model
	shader_list[7].use();
	shader_list[7].setFloat("alpha", ToolAlpha);
	shader_list[7].setMat4("projection", projection);
	shader_list[7].setMat4("view", view);

	//model = model_tool * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = model_tool * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = model * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	shader_list[7].setMat4("model", model);
	toolModel.Draw(shader_list[7]);

	float jaw_translation = 7.5;// 15.5;

	model = model_tool * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = model * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = model * glm::translate(glm::mat4(1.0f), glm::vec3(-jaw_translation, 0.0, 0.0));
	model = model * glm::rotate(glm::mat4(1.0f), glm::radians(OpenAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = model * glm::translate(glm::mat4(1.0f), glm::vec3(jaw_translation, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	shader_list[7].setMat4("model", model);
	toolModel_Ljaw.Draw(shader_list[7]);

	model = model_tool * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = model * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = model * glm::translate(glm::mat4(1.0f), glm::vec3(-jaw_translation, 0.0, 0.0));
	model = model * glm::rotate(glm::mat4(1.0f), glm::radians(-OpenAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = model * glm::translate(glm::mat4(1.0f), glm::vec3(jaw_translation, 0.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	shader_list[7].setMat4("model", model);
	toolModel_Rjaw.Draw(shader_list[7]);




	// Text
	shader_list[1].use();
	shader_list[1].setMat4("projection", font_proxy.projection);
	font_proxy.RenderText(shader_list[1], renderTextLlist[0], 25.0f, WINDOW_HEIGHT - 20, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
	font_proxy.RenderText(shader_list[1], renderTextLlist[1], 25.0f, WINDOW_HEIGHT - 60, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
	font_proxy.RenderText(shader_list[1], renderTextLlist[2], 25.0f, WINDOW_HEIGHT - 100, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));



	// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	// clear all relevant buffers
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
	glClear(GL_COLOR_BUFFER_BIT);

	shader_list[5].use();
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);


	glfwSwapBuffers(window);
	glfwPollEvents();

	return;
}

void GLViewer::screenshot() {


	char filename[256] = { 0 };
	sprintf(filename, "clip_%3f_output.jpg",
			(clock() - pOutput->startTime) / CLOCKS_PER_SEC);

	cv::Mat save_image(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
	cv::Mat save_image_flip(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);

	std::cout << "[INFO] save image " << filename << std::endl;
	glPixelStorei(GL_PACK_ALIGNMENT, (save_image.step & 3) ? 1 : 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, save_image.step / save_image.elemSize());
	glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_BGR_EXT, GL_UNSIGNED_BYTE, save_image.data);
	cv::flip(save_image, save_image_flip, 0);

	//cv::imwrite(pOutput->pCollector_joystick->output_dir + pOutput->pCollector_joystick->single_dirname + filename, save_image_flip);
	cv::imwrite(filename, save_image_flip);


}

void GLViewer::setIn(glm::mat4 pose) {
	model_tool = pose;
	return;
}

void GLViewer::setIn(std::vector<glm::mat4>pose, glm::mat4 model) {
	point_list.clear();
	for (int i = 0; i < pose.size(); i++) {
		point_list.push_back(model * pose[i]);
	}

	return;
}

void GLViewer::setIn(std::vector<glm::mat4>pose, std::vector<glm::mat4> model) {
	point_list.clear();
	for (int i = 0; i < pose.size(); i++) {
		point_list.push_back(model[i] * pose[i]);
	}

	return;
}

void GLViewer::setIn(glm::mat4 pose, glm::mat4 model) {
	point_list.clear();
	point_list.push_back(model * pose);

	return;
}

void GLViewer::setIn(glm::vec3 force) {
	force_mag = force;
	return;
}

void GLViewer::setIn(std::vector<std::string> text_list) {
	renderTextLlist.clear();
	renderTextLlist = text_list;
	return;
}

glm::vec3 GLViewer::HSVtoRGB(glm::vec3 hsv) {

	//  \param fR Red component, used as output, range: [0, 1]
	//  \param fG Green component, used as output, range: [0, 1]
	//	\param fB Blue component, used as output, range: [0, 1]
	//	\param fH Hue component, used as input, range: [0, 360]
	//	\param fS Hue component, used as input, range: [0, 1]
	//	\param fV Hue component, used as input, range: [0, 1]


	glm::vec3 rgb;

	float fC = hsv[2] * hsv[1]; // Chroma
	float fHPrime = fmod(hsv[0] / 60.0, 6);
	float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	float fM = hsv[2] - fC;

	if (0 <= fHPrime && fHPrime < 1) {
		rgb[0] = fC;
		rgb[1] = fX;
		rgb[2] = 0;
	}
	else if (1 <= fHPrime && fHPrime < 2) {
		rgb[0] = fX;
		rgb[1] = fC;
		rgb[2] = 0;
	}
	else if (2 <= fHPrime && fHPrime < 3) {
		rgb[0] = 0;
		rgb[1] = fC;
		rgb[2] = fX;
	}
	else if (3 <= fHPrime && fHPrime < 4) {
		rgb[0] = 0;
		rgb[1] = fX;
		rgb[2] = fC;
	}
	else if (4 <= fHPrime && fHPrime < 5) {
		rgb[0] = fX;
		rgb[1] = 0;
		rgb[2] = fC;
	}
	else if (5 <= fHPrime && fHPrime < 6) {
		rgb[0] = fC;
		rgb[1] = 0;
		rgb[2] = fX;
	}
	else {
		rgb[0] = 0;
		rgb[1] = 0;
		rgb[2] = 0;
	}

	rgb[0] += fM;
	rgb[1] += fM;
	rgb[2] += fM;

	return rgb;
}

glm::mat4 GLViewer::joystick2screenTransformation(glm::mat4 input) {
	float transX = input[3][0];
	float transY = input[3][1];
	float transZ = input[3][2];
	glm::mat4 output = input;
	output[3][0] = transX;
	output[3][1] = -transZ;
	output[3][2] = transY;
	return output;
}

void font_proxy::init() {

	projection = glm::ortho(0.0f, static_cast<GLfloat>(WINDOW_WIDTH), 0.0f, static_cast<GLfloat>(WINDOW_HEIGHT));

	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, "..\\src\\fonts\\arial.ttf", 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < 128; c++) {
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
		//std::cout << "c= " << c << std::endl;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return;
}



// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void GLViewer::processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void GLViewer::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void GLViewer::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	//printf("input = (%f, %f) \n", xposIn, yposIn);
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (pThis->firstMouse) {
		pThis->lastX = xpos;
		pThis->lastY = ypos;
		pThis->firstMouse = false;
	}

	float xoffset = xpos - pThis->lastX;
	float yoffset = pThis->lastY - ypos; // reversed since y-coordinates go from bottom to top

	pThis->lastX = xpos;
	pThis->lastY = ypos;

	if (pThis->is_pressed) {
		pThis->camera.ProcessMouseMovement(xoffset, yoffset);
		//pThis->camera.ProcessMouseMovement_ObjectCenter(xoffset, yoffset);
		//printf("mouse event: %f %f", xoffset, yoffset);
	}
	//camera.ProcessMouseMovement(xoffset, yoffset);

}

void GLViewer::mouse_btn_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		//prev_mouse.x = cur_mouse.x;
		//prev_mouse.y = cur_mouse.y;
		pThis->is_pressed = true;
	}
	else {
		pThis->is_pressed = false;
	}
}


void font_proxy::RenderText(Shader& shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
	// Activate corresponding render state	
	//std::cout << text << std::endl;

	shader.use();
	shader.setVec3("textColor", glm::vec3(color.x, color.y, color.z));
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	return;
}



Simple3DObject GLViewer::createCube() {
	Simple3DObject it;
	// vertex color normal
	std::vector<float> vertices = {
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
	};
	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	return it;
}

Simple3DObject GLViewer::createTube() {
	Simple3DObject it;
	//std::vector<float> vertices = {
	//	-1.0f, -0.025f, 0.0f,   0.0f, 0.0f, 0.0f,
	//	 1.0f, -0.025f, 0.0f,   0.0f, 1.0f, 0.0f,
	//	 1.0f,  0.0f, -0.025f,  0.0f, 1.0f, 1.0f,
	//	 1.0f,  0.0f, -0.025f,  0.0f, 1.0f, 1.0f,
	//	-1.0f,  0.0f, -0.025f,  0.0f, 0.0f, 1.0f,
	//	-1.0f, -0.025f, 0.0f,   0.0f, 0.0f, 0.0f,
	//
	//	-1.0f, -0.025f,  0.0f,  0.0f, 0.0f, 0.0f,
	//	 1.0f, -0.025f,  0.0f,  0.0f, 1.0f, 0.0f,
	//	 1.0f, 0.0f,  0.025f,   0.0f, 1.0f, 1.0f,
	//	 1.0f, 0.0f,  0.025f,   0.0f, 1.0f, 1.0f,
	//	-1.0f, 0.0f,  0.025f,   0.0f, 0.0f, 1.0f,
	//	-1.0f, -0.025f,  0.0f,  0.0f, 0.0f, 0.0f,
	//
	//	-1.0f, 0.025f, 0.0f,    0.0f, 0.0f, 0.0f,
	//	 1.0f, 0.025f, 0.0f,    0.0f, 1.0f, 0.0f,
	//	 1.0f,  0.0f, -0.025f,  0.0f, 1.0f, 1.0f,
	//	 1.0f,  0.0f, -0.025f,  0.0f, 1.0f, 1.0f,
	//	-1.0f,  0.0f, -0.025f,  0.0f, 0.0f, 1.0f,
	//	-1.0f, 0.025f, 0.0f,    0.0f, 0.0f, 0.0f,
	//
	//	-1.0f, 0.025f,  0.0f,   0.0f, 0.0f, 0.0f,
	//	 1.0f, 0.025f,  0.0f,   0.0f, 1.0f, 0.0f,
	//	 1.0f, 0.0f,  0.025f,   0.0f, 1.0f, 1.0f,
	//	 1.0f, 0.0f,  0.025f,   0.0f, 1.0f, 1.0f,
	//	-1.0f, 0.0f,  0.025f,   0.0f, 0.0f, 1.0f,
	//	-1.0f, 0.025f,  0.0f,   0.0f, 0.0f, 0.0f
	//
	//};

	// with vertex normal
	std::vector<float> vertices = {
		-0.25f, -0.025f, 0.0f,   0.8f, 0.0f, 0.0f,
		 0.25f, -0.025f, 0.0f,   0.8f, 0.0f, 0.0f,
		 0.25f,  0.0f, -0.025f,  0.8f, 0.0f, 0.0f,
		 0.25f,  0.0f, -0.025f,  0.8f, 0.0f, 0.0f,
		-0.25f,  0.0f, -0.025f,  0.8f, 0.0f, 0.0f,
		-0.25f, -0.025f, 0.0f,   0.8f, 0.0f, 0.0f,

		-0.25f, -0.025f,  0.0f,  0.8f, 0.0f, 0.0f,
		 0.25f, -0.025f,  0.0f,  0.8f, 0.0f, 0.0f,
		 0.25f, 0.0f,  0.025f,   0.8f, 0.0f, 0.0f,
		 0.25f, 0.0f,  0.025f,   0.8f, 0.0f, 0.0f,
		-0.25f, 0.0f,  0.025f,   0.8f, 0.0f, 0.0f,
		-0.25f, -0.025f,  0.0f,  0.8f, 0.0f, 0.0f,

		-0.25f, 0.025f, 0.0f,    0.8f, 0.0f, 0.0f,
		 0.25f, 0.025f, 0.0f,    0.8f, 0.0f, 0.0f,
		 0.25f,  0.0f, -0.025f,  0.8f, 0.0f, 0.0f,
		 0.25f,  0.0f, -0.025f,  0.8f, 0.0f, 0.0f,
		-0.25f,  0.0f, -0.025f,  0.8f, 0.0f, 0.0f,
		-0.25f, 0.025f, 0.0f,    0.8f, 0.0f, 0.0f,

		-0.25f, 0.025f,  0.0f,   0.8f, 0.0f, 0.0f,
		 0.25f, 0.025f,  0.0f,   0.8f, 0.0f, 0.0f,
		 0.25f, 0.0f,  0.025f,   0.8f, 0.0f, 0.0f,
		 0.25f, 0.0f,  0.025f,   0.8f, 0.0f, 0.0f,
		-0.25f, 0.0f,  0.025f,   0.8f, 0.0f, 0.0f,
		-0.25f, 0.025f,  0.0f,   0.8f, 0.0f, 0.0f

	};

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	return it;
}

Simple3DObject GLViewer::createTube2() {
	Simple3DObject it;

	// with vertex normal
	std::vector<float> vertices;

	int round = 12;
	for (int i = 0; i < 12; i++) {
		vertices.insert(vertices.end(), { -0.25,(float)(cos(((i) % round) * 360 / round) * 0.025),    (float)(sin(((i) % round) * 360 / round) * 0.025),    (float)((rand() % 500) / 1000.0 + 0.5f), 0.0f, 0.0f });
		vertices.insert(vertices.end(), { 0.25, (float)(cos(((i + 1) % round) * 360 / round) * 0.025),(float)(sin(((i + 1) % round) * 360 / round) * 0.025),(float)((rand() % 500) / 1000.0 + 0.5f), 0.0f, 0.0f });
		vertices.insert(vertices.end(), { -0.25,(float)(cos(((i + 1) % round) * 360 / round) * 0.025),(float)(sin(((i + 1) % round) * 360 / round) * 0.025),(float)((rand() % 500) / 1000.0 + 0.5f), 0.0f, 0.0f });
		vertices.insert(vertices.end(), { -0.25,(float)(cos(((i) % round) * 360 / round) * 0.025),    (float)(sin(((i) % round) * 360 / round) * 0.025),    (float)((rand() % 500) / 1000.0 + 0.5f), 0.0f, 0.0f });
		vertices.insert(vertices.end(), { 0.25, (float)(cos(((i) % round) * 360 / round) * 0.025),    (float)(sin(((i) % round) * 360 / round) * 0.025),    (float)((rand() % 500) / 1000.0 + 0.5f), 0.0f, 0.0f });
		vertices.insert(vertices.end(), { 0.25, (float)(cos(((i + 1) % round) * 360 / round) * 0.025),(float)(sin(((i + 1) % round) * 360 / round) * 0.025),(float)((rand() % 500) / 1000.0 + 0.5f), 0.0f, 0.0f });

	}

	// with vertex normal

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	return it;
}

Simple3DObject GLViewer::createArrow() {
	Simple3DObject it;

	std::vector<float> vertices = {
		0.0f, 0.01f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.75f, 0.01f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.75f, -0.01f, 0.0f, 0.0f, 1.0f, 0.0f,

		0.0f, 0.01f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.75f, -0.01f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -0.01f, 0.0f, 0.0f, 1.0f, 0.0f,

		0.75f, 0.02f, 0.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.75f, -0.02f, 0.0f, 0.0f, 1.0f, 0.0f
	};

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	return it;

}

Simple3DObject GLViewer::createSquare() {
	Simple3DObject it;
	std::vector<float> vertices = {
		//right
		1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		-1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		// right down
		-1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		-1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,

	};

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	return it;
}

Simple3DObject GLViewer::createSkin() {
	Simple3DObject it;
	std::vector<float> vertices = {
		//right
		1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		0.5f,  0.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		// right down
		0.5f,  0.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		0.0f,  -0.25f, 0.0f,  1.0f, 0.9f, 0.85f,
		//down
		0.0f,  -0.25f, 0.0f,  1.0f, 0.9f, 0.85f,
		1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		-1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		//left down
		0.0f,  -0.25f, 0.0f,  1.0f, 0.9f, 0.85f,
		-1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		-0.5f,  0.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		//left
		-0.5f,  0.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		-1.0f,  -1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		-1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		//left up
		-0.5f,  0.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		-1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		0.0f,  0.25f, 0.0f,  1.0f, 0.9f, 0.85f,
		//up
		-1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		0.0f,  0.25f, 0.0f,  1.0f, 0.9f, 0.85f,
		//right up
		1.0f,  1.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		0.5f,  0.0f, 0.0f,  1.0f, 0.9f, 0.85f,
		0.0f,  0.25f, 0.0f,  1.0f, 0.9f, 0.85f

	};

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	return it;

}

Simple3DObject GLViewer::createTool() {
	Simple3DObject it;
	std::vector<float> vertices = {
		//stick
		-0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,

		-0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,

		-0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,

		 0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,

		-0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  1.0f, -0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  0.5f, -0.025f,  0.72f, 0.72f, 0.72f,

		-0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		 0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  1.0f,  0.025f,  0.72f, 0.72f, 0.72f,
		-0.025f,  0.5f,  0.025f,  0.72f, 0.72f, 0.72f,

		// jaw left

		-0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,

		-0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,

		-0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		-0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,

		-0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,

		-0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		-0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		-0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		-0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,

		-0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		-0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,

		// jaw right

		0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,

		0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,

		0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,

		0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,

		0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.075f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,
		0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		0.075f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		0.025f, 0.0f,  0.025f,  0.62f, 0.62f, 0.62f,
		0.025f, 0.0f, -0.025f,  0.62f, 0.62f, 0.62f,

		0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		0.075f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,
		0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.075f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.025f,  0.5f,  0.025f, 0.62f, 0.62f, 0.62f,
		0.025f,  0.5f, -0.025f, 0.62f, 0.62f, 0.62f,

	};

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	return it;
}

Simple3DObject GLViewer::createGrid() {
	Simple3DObject it;
	std::vector<float> vertices;
	int dist = 1.0f;// 0.01f;
	int num = 10;
	for (int i = -num; i <= num; i++) {
		//X-axis
		vertices.push_back(-num); vertices.push_back(i * 0.1f + 0.001f); vertices.push_back(0.0f);
		vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
		vertices.push_back(num); vertices.push_back(i * 0.1f + 0.001f); vertices.push_back(0.0f);
		vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
		vertices.push_back(-num); vertices.push_back(i * 0.1f - 0.001f); vertices.push_back(0.0f);
		vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
		vertices.push_back(-num); vertices.push_back(i * 0.1f - 0.001f); vertices.push_back(0.0f);
		vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
		vertices.push_back(num); vertices.push_back(i * 0.1f + 0.001f); vertices.push_back(0.0f);
		vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
		vertices.push_back(num); vertices.push_back(i * 0.1f - 0.001f); vertices.push_back(0.0f);
		vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);

		//y-axis
		vertices.push_back(i * 0.1f + 0.001f); vertices.push_back(-num); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
		vertices.push_back(i * 0.1f + 0.001f); vertices.push_back(num); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
		vertices.push_back(i * 0.1f - 0.001f); vertices.push_back(-num); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
		vertices.push_back(i * 0.1f - 0.001f); vertices.push_back(-num); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
		vertices.push_back(i * 0.1f + 0.001f); vertices.push_back(num); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
		vertices.push_back(i * 0.1f - 0.001f); vertices.push_back(num); vertices.push_back(0.0f);
		vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);

	}

	it.vertices_ = vertices;
	//it.drawType_ = GL_LINES;
	it.drawType_ = GL_TRIANGLES;
	return it;
}

Simple3DObject GLViewer::createScreen() {
	Simple3DObject it;
	std::vector<float> vertices = {
		 -1.0f, -1.0f, 0.5f,  0.0f, 0.0f,
		  1.0f, -1.0f, 0.5f,  1.0f, 0.0f,
		  1.0f,  1.0f, 0.5f,  1.0f, 1.0f,
		  1.0f,  1.0f, 0.5f,  1.0f, 1.0f,
		 -1.0f,  1.0f, 0.5f,  0.0f, 1.0f,
		 -1.0f, -1.0f, 0.5f,  0.0f, 0.0f,

	};

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	it.isStatic = GL_DYNAMIC_DRAW;
	return it;
}

Simple3DObject GLViewer::createScreenSmall() {
	Simple3DObject it;
	std::vector<float> vertices = {
		 -0.54f, -0.84f, 0.5f,  0.0f, 0.0f,
		  0.54f, -0.84f, 0.5f,  1.0f, 0.0f,
		  0.54f,  0.84f, 0.5f,  1.0f, 1.0f,
		  0.54f,  0.84f, 0.5f,  1.0f, 1.0f,
		 -0.54f,  0.84f, 0.5f,  0.0f, 1.0f,
		 -0.54f, -0.84f, 0.5f,  0.0f, 0.0f,

	};

	it.vertices_ = vertices;
	it.drawType_ = GL_TRIANGLES;
	it.isStatic = GL_DYNAMIC_DRAW;
	return it;
}

Simple3DObject GLViewer::createPoint() {
	Simple3DObject it;
	std::vector<float> vertices = {
		0.0f,0.0f,0.0f,0.8f,0.8f,0.8f
	};
	it.vertices_ = vertices;
	it.drawType_ = GL_POINTS;
	return it;
}

Simple3DObject GLViewer::createLine() {
	Simple3DObject it;
	std::vector<float> vertices = {
		0.0f,0.0f,0.0f,0.5f,0.8f,0.5f,
		1.0f,0.0f,0.0f,0.5f,0.8f,0.5f
	};
	it.vertices_ = vertices;
	it.drawType_ = GL_LINES;
	return it;
}


Simple3DObject GLViewer::createCircle() {
	Simple3DObject it;
	std::vector<float> vertices;
	int cut = 24;
	for (int i = 0; i < cut; i++) {
		vertices.push_back(cos((i % cut) * 360 / cut * RAD)); vertices.push_back(sin((i % cut) * 360 / cut * RAD)); vertices.push_back(0.0f);
		vertices.push_back(cos(((i + 1) % cut) * 360 / cut * RAD)); vertices.push_back(sin(((i + 1) % cut) * 360 / cut * RAD)); vertices.push_back(0.0f);
	}

	it.vertices_ = vertices;
	it.drawType_ = GL_LINES;
	return it;
}

void Simple3DObject::pushToGPU(int vertex_p, int normal_p, int color_p, int tex_p) {
	glGenVertexArrays(1, &VAO_);
	glGenBuffers(1, &VBO_);
	glBindVertexArray(VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(float), &vertices_[0], isStatic);

	int set_order = 0;
	if (vertex_p > 0) {
		glVertexAttribPointer(set_order, vertex_p, GL_FLOAT, GL_FALSE, (vertex_p + color_p + tex_p + normal_p) * sizeof(float), (void*)0);
		glEnableVertexAttribArray(set_order);

		set_order = set_order + 1;
	}
	if (normal_p > 0) {
		glVertexAttribPointer(set_order, normal_p, GL_FLOAT, GL_FALSE, (vertex_p + color_p + tex_p + normal_p) * sizeof(float), (void*)((vertex_p) * sizeof(float)));
		glEnableVertexAttribArray(set_order);
		set_order = set_order + 1;
	}
	if (color_p > 0) {
		glVertexAttribPointer(set_order, color_p, GL_FLOAT, GL_FALSE, (vertex_p + color_p + tex_p + normal_p) * sizeof(float), (void*)((vertex_p + normal_p) * sizeof(float)));
		glEnableVertexAttribArray(set_order);
		set_order = set_order + 1;
	}
	if (tex_p > 0) {
		glVertexAttribPointer(set_order, tex_p, GL_FLOAT, GL_FALSE, (vertex_p + color_p + tex_p + normal_p) * sizeof(float), (void*)((vertex_p + normal_p + color_p) * sizeof(float)));
		glEnableVertexAttribArray(set_order);
		set_order = set_order + 1;
	}


	return;

}

Simple3DObject::Simple3DObject() {
	translate_ = glm::mat4(1.0f);
	rotate_ = glm::mat4(1.0f);
	scale_ = glm::mat4(1.0f);
}

glm::mat4 Simple3DObject::getModelMetrix() {
	//return   rotateFromOrigion(translate_) * translate_ * rotate_ * scale_;
	return  translate_ * rotate_ * scale_;

	//return scale_ * translate_ * rotate_;
}

glm::mat4 Simple3DObject::getTranslate() {
	return translate_;
}
glm::mat4 Simple3DObject::getRotate() {
	return rotate_;
}
glm::mat4 Simple3DObject::getScale() {
	return scale_;
}


void Simple3DObject::setScale(glm::vec3 input) {
	scale_ = glm::scale(glm::mat4(1.0f), input);
	return;
}

void Simple3DObject::setScale(glm::mat4 input) {
	scale_ = input;
	return;
}

void Simple3DObject::setRotate(float degree, glm::vec3 direction) {
	rotate_ = glm::rotate(glm::mat4(1.0f), glm::radians(degree), direction);
	return;
}

void Simple3DObject::setRotate(glm::mat4 input) {
	rotate_ = input;
	return;
}


void Simple3DObject::setTranslate(glm::vec3 input) {
	//input = input + translate_offset + glm::vec3(0.0f, 0.0f, z_offset);
	translate_ = glm::translate(glm::mat4(1.0f), input);
	return;
}

void Simple3DObject::addScale(glm::vec3 input) {
	scale_ = glm::scale(scale_, input);
	return;
}

void Simple3DObject::addRotate(float degree, glm::vec3 direction) {
	rotate_ = glm::rotate(rotate_, glm::radians(degree), direction);
	return;
}

void Simple3DObject::addTranslate(glm::vec3 input) {
	//input = input + translate_offset;
	translate_ = glm::translate(translate_, input);
	return;
}

void Simple3DObject::draw() {
	glBindVertexArray(VAO_);
	glDrawArrays(drawType_, 0, vertices_.size());
	glBindVertexArray(0);
}

void useTexture::init() {
	// load and create a texture 
	// -------------------------
	glGenTextures(1, &ID1);
}



void useTexture::readTexture() {
	//glGenTextures(1, &ID1);
	glBindTexture(GL_TEXTURE_2D, ID1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;

	cv::cvtColor(image1, image1, cv::COLOR_BGR2RGB);
	width = image1.cols;
	height = image1.rows;
	nrChannels = image1.channels();
	if (image1.data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image1.data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture1" << std::endl;
	}

	return;

}

void useTexture::use() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ID1);
}