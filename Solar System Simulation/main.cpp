#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Shader.h"
#include "Texture.h"
#include "glMemory.hpp"

//physics

// io
#include "Camera.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "Joystick.h"

#include "Planet.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float dt);


unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;


Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 0.0f));



int main() {
	// Intialize GLFW

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System Simulation", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n" << std::endl;
		return -1;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, Keyboard::KeyCallBack);
	glfwSetCursorPosCallback(window, Mouse::cursorPosCallBack);


	glEnable(GL_DEPTH_TEST);

	// HDR FRAMEBUFFER
	GLuint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	GLuint colorBuffer;
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		SCR_WIDTH,
		SCR_HEIGHT,
		0,
		GL_RGBA,
		GL_FLOAT,
		nullptr
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		colorBuffer,
		0
	);

	// depth
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH_COMPONENT,
		SCR_WIDTH,
		SCR_HEIGHT
	);

	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		rboDepth
	);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "HDR framebuffer not complete" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLuint pingpongFBO[2];
	GLuint pingpongTex[2];

	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongTex);

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongTex[i]);

		glTexImage2D(
			GL_TEXTURE_2D, 0,
			GL_RGBA16F,
			SCR_WIDTH, SCR_HEIGHT,
			0, GL_RGBA, GL_FLOAT, nullptr
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// clamp to edge for blur
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			pingpongTex[i],
			0
		);
	}

	// SHADERS ====================================================
	Shader planetShader ("planets.vert", "planets.frag" );
	Shader starsShader  ("stars.vert",   "stars.frag"   );
	Shader glowShader   ("glow.vert",    "glow.frag"    );
	Shader brightShader ("screen.vert",  "bright.frag"  );
	Shader blurShader   ("screen.vert",  "blur.frag"    );
	Shader finalShader  ("screen.vert",  "final.frag"   );

	// Textures ==================================================
	Texture sunTex     ("assets", "sun.jpg"             );
	Texture mercuryTex ("assets", "mercury.jpg"         );
	Texture venusTex   ("assets", "mercury.jpg"         );
	Texture earthTex   ("assets", "earth.jpg"           );
	Texture moonTex    ("assets", "moon.jpg"            );
	Texture marsTex    ("assets", "mars.jpg"            );
	Texture jupiterTex ("assets", "jupiter.jpg"         );
	Texture saturnTex  ("assets", "saturn.jpg"          );
	Texture uranusTex  ("assets", "uranus.jpg"          );
	Texture neptuneTex ("assets", "neptune.jpg"         );
	Texture starsTex   ("assets", "stars_milky_way.jpg" );
	sunTex.load();
	mercuryTex.load();
	earthTex.load(false);
	moonTex.load();
	marsTex.load();
	jupiterTex.load();
	saturnTex.load();
	uranusTex.load();
	neptuneTex.load();
	starsTex.load();

	// MODELS  ====================================================
	Mesh* sphere = Mesh::CreateSphere(64, 64);

	GLuint quadVAO = 0, quadVBO = 0;

	float quadVertices[] = {
		// pos        // tex
		-1.f, -1.f,   0.f, 0.f,
		 1.f, -1.f,   1.f, 0.f,
		 1.f,  1.f,   1.f, 1.f,
					  
		-1.f, -1.f,   0.f, 0.f,
		 1.f,  1.f,   1.f, 1.f,
		-1.f,  1.f,   0.f, 1.f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
		(void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);


	auto sun = std::make_shared<Planet>("Sun", 2.5f, 0.0f, 0.0f, 5.0f, sunTex.id, nullptr);
	auto mercury = std::make_shared<Planet>("Mercury", 0.095f, 5.33f, 23.5f, 3.0f, mercuryTex.id, sun);
	auto venus = std::make_shared<Planet>("Venus", 0.235f, 7.33f, 17.5f, 2.0f, venusTex.id, sun);
	auto earth = std::make_shared<Planet>("Earth", 0.25f, 10.0f, 15.0f, 25.0f, earthTex.id, sun);
	auto moon = std::make_shared<Planet>("Moon", 0.07f, 1.0f, 40.0f, 5.0f, moonTex.id, earth);
	auto mars = std::make_shared<Planet>("Mars", 0.135f, 13.33f, 12.0f, 22.5f, marsTex.id, sun);
	auto jupiter = std::make_shared<Planet>("Jupiter", 0.75f, 20.0f, 6.5f, 50.0f, jupiterTex.id, sun);
	auto saturn = std::make_shared<Planet>("Saturn", 0.6f, 26.67f, 4.5f, 45.0f, saturnTex.id, sun);
	auto uranus = std::make_shared<Planet>("Uranus", 0.45f, 33.33f, 3.0f, 35.0f, uranusTex.id, sun);
	auto neptune = std::make_shared<Planet>("Neptune", 0.425f, 40.0f, 2.5f, 32.5f, neptuneTex.id, sun);

	sun->addChild(mercury);
	sun->addChild(venus);
	sun->addChild(earth);
	sun->addChild(mars);
	sun->addChild(jupiter);
	sun->addChild(saturn);
	sun->addChild(uranus);
	sun->addChild(neptune);

	earth->addChild(moon);

	std::vector<std::shared_ptr<Planet>> roots = { sun };

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float timeScale = 1.0f;

	// main window
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window, deltaTime);

		for (auto& r : roots) r->update(deltaTime, timeScale);

		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		starsShader.activate();

		float time = glfwGetTime();

		glm::mat4 model(1.0f);
		model = glm::translate(model, Camera::defaultCamera.cameraPos);
		model = glm::scale(model, glm::vec3(1000.0f));

		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = Camera::defaultCamera.getViewMatrix();

		projection = glm::perspective(
			glm::radians(Camera::defaultCamera.getZoom()),
			(float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5000.0f);

		starsShader.setMat4("view", view);
		starsShader.setMat4("projection", projection);
		starsShader.setMat4("model", model);

		starsShader.setInt("starsTex", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, starsTex.id);

		sphere->draw();

		glCullFace(GL_BACK);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glBindTexture(GL_TEXTURE_2D, 0);

		
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);

		planetShader.activate();

		planetShader.setMat4("view", view);
		planetShader.setMat4("projection", projection);
		planetShader.set3Float("viewPos", Camera::defaultCamera.cameraPos);

		// sun as lightSource
		planetShader.set3Float("sunLight.position", glm::vec3(0.0f, 0.0f, 0.0f));
		planetShader.set3Float("sunLight.ambient", glm::vec3(0.2f));
		planetShader.set3Float("sunLight.diffuse", glm::vec3(6.0f));
		planetShader.set3Float("sunLight.specular", glm::vec3(3.0f));
		planetShader.setFloat("shininess", 32.0f);


		for (auto& r : roots) r->draw(planetShader, glm::mat4(1.0f));


			glDisable(GL_DEPTH_TEST);

			// Bright pass -> extract bright parts into pingpongTex[0]
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			brightShader.activate();
			brightShader.setInt("scene", 0);
			// lower threshold so the sun gets picked up; tune between 0.3 - 1.0
			brightShader.setFloat("threshold", 0.6f);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorBuffer);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			// Blur pass (ping-pong)
			bool horizontal = true;
			bool first = true;
			int blurPasses = 10;

			blurShader.activate();
			blurShader.setInt("image", 0);

			for (int i = 0; i < blurPasses; i++) {
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal ? 1 : 0]);
				blurShader.setBool("horizontal", horizontal);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(
					GL_TEXTURE_2D,
					first ? pingpongTex[0] : pingpongTex[horizontal ? 0 : 1]
				);

				glBindVertexArray(quadVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				horizontal = !horizontal;
				if (first) first = false;
			}

			// final pass: combine HDR scene + blurred bloom
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			finalShader.activate();
			finalShader.setInt("scene", 0);
			finalShader.setInt("bloom", 1);
			finalShader.setFloat("exposure", 1.0f);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorBuffer);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, pingpongTex[!horizontal]);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//VAO.cleanup();
	delete sphere;
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);

	SCR_HEIGHT = height;
	SCR_WIDTH = width;
}

void processInput(GLFWwindow* window, float dt) {

	glm::vec3 move(0.0f);
	if (Keyboard::key(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);

	if (Keyboard::key(GLFW_KEY_W)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::FORWARD, dt);
	}

	if (Keyboard::key(GLFW_KEY_S)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::BACKWARD, dt);
	}

	if (Keyboard::key(GLFW_KEY_D)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::RIGHT, dt);
	}

	if (Keyboard::key(GLFW_KEY_A)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::LEFT, dt);
	}

	if (Keyboard::key(GLFW_KEY_SPACE)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::UP, dt);
	}

	if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::DOWN, dt);
	}


	//mouse
	double dx = Mouse::getDX(), dy = Mouse::getDY();
	if (dx != 0 || dy != 0) {
		Camera::defaultCamera.updateCameraDirection(dx, dy);
	}

	double scrollDY = Mouse::getScrollDY();
	if (scrollDY != 0) {
		Camera::defaultCamera.updateCameraZoom(scrollDY);
	}
}
