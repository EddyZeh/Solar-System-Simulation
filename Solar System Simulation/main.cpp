#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

#include "Shader.h"
#include "Texture.h"
#include "glMemory.hpp"

//physics
#include "Physics.h"

// io
#include "Camera.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "Joystick.h"

#include "Planet.h"
#include "Bloom.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float dt);

Bloom* g_bloom = nullptr;   // so the resize callback can rebuild the HDR/bloom targets


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

	// the framebuffer can differ from the window size (high-DPI screens), so ask GLFW
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	SCR_WIDTH = fbWidth;
	SCR_HEIGHT = fbHeight;
	glViewport(0, 0, fbWidth, fbHeight);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, Keyboard::KeyCallBack);
	glfwSetCursorPosCallback(window, Mouse::cursorPosCallBack);


	glEnable(GL_DEPTH_TEST);

	// HDR FRAMEBUFFER + BLOOM (see Bloom.h; replaces the hand-written hdrFBO / pingpong code)
	Bloom bloom;
	bloom.init(fbWidth, fbHeight);
	bloom.threshold = 1.0f;   // only things brighter than 1.0 in HDR (the Sun) glow
	bloom.strength = 0.9f;
	bloom.exposure = 1.0f;
	g_bloom = &bloom;

	// SHADERS ====================================================
	Shader planetShader("planets.vert", "planets.frag");
	Shader starsShader("stars.vert", "stars.frag");
	Shader glowShader("glow.vert", "glow.frag");

	// Textures ==================================================
	Texture sunTex("assets", "sun.jpg");
	Texture mercuryTex("assets", "mercury.jpg");
	Texture venusTex("assets", "venus.jpg");
	Texture earthTex("assets", "earth.jpg");
	Texture moonTex("assets", "moon.jpg");
	Texture marsTex("assets", "mars.jpg");
	Texture jupiterTex("assets", "jupiter.jpg");
	Texture saturnTex("assets", "saturn.jpg");
	Texture uranusTex("assets", "uranus.jpg");
	Texture neptuneTex("assets", "neptune.jpg");
	Texture starsTex("assets", "stars_milky_way.jpg");
	sunTex.load();
	mercuryTex.load();
	venusTex.load();
	earthTex.load();
	moonTex.load();
	marsTex.load();
	jupiterTex.load();
	saturnTex.load();
	uranusTex.load();
	neptuneTex.load();
	starsTex.load();

	// MODELS  ====================================================
	Mesh* sphere = Mesh::CreateSphere(64, 64);

	const double sunMass = 1500.0;
	const double earthMass = 20.0;
	const double moonMass = 0.25;
	const float  moonDist = 2.0f;

	const float moonSpin = (float)glm::degrees(
		physics::circularAngularRate(physics::G * (earthMass + moonMass), moonDist));

	//                                      name       radius  mass                    dist    spin  tilt    texture
	auto sun = std::make_shared<Planet>("Sun", 2.5f, sunMass, 0.0f, 5.0f, 7.25f, sunTex.id, nullptr, 8.0f);
	auto mercury = std::make_shared<Planet>("Mercury", 0.2375f, sunMass * 1.66e-7, 13.33f, 3.0f, 0.03f, mercuryTex.id, sun);
	auto venus = std::make_shared<Planet>("Venus", 0.5875f, sunMass * 2.45e-6, 18.33f, 2.0f, 177.36f, venusTex.id, sun);
	auto earth = std::make_shared<Planet>("Earth", 0.625f, earthMass, 25.0f, 25.0f, 23.44f, earthTex.id, sun);
	auto moon = std::make_shared<Planet>("Moon", 0.17f, moonMass, moonDist, moonSpin, 1.54f, moonTex.id, earth);
	auto mars = std::make_shared<Planet>("Mars", 0.3375f, sunMass * 3.23e-7, 33.33f, 22.5f, 25.19f, marsTex.id, sun);
	auto jupiter = std::make_shared<Planet>("Jupiter", 1.875f, sunMass * 9.54e-4, 50.0f, 50.0f, 3.13f, jupiterTex.id, sun);
	auto saturn = std::make_shared<Planet>("Saturn", 1.5f, sunMass * 2.86e-4, 66.67f, 45.0f, 26.73f, saturnTex.id, sun);
	auto uranus = std::make_shared<Planet>("Uranus", 1.125f, sunMass * 4.37e-5, 83.33f, 35.0f, 97.77f, uranusTex.id, sun);
	auto neptune = std::make_shared<Planet>("Neptune", 1.0625f, sunMass * 5.15e-5, 100.0f, 32.5f, 28.32f, neptuneTex.id, sun);

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
	double physicsAccumulator = 0.0;

	// main window
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window, deltaTime);


		physicsAccumulator += std::min(deltaTime, 0.25f) * timeScale;
		int physicsSteps = 0;
		while (physicsAccumulator >= physics::kFixedDt && physicsSteps < 64) {
			for (auto& r : roots) r->update(physics::kFixedDt);
			physicsAccumulator -= physics::kFixedDt;
			physicsSteps++;
		}
		if (physicsSteps == 64) physicsAccumulator = 0.0;

		bloom.bindScene();   // render into the floating-point HDR buffer

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
		planetShader.set3Float("sunLight.ambient", glm::vec3(0.03f));
		planetShader.set3Float("sunLight.diffuse", glm::vec3(3.0f));
		planetShader.set3Float("sunLight.specular", glm::vec3(0.3f));
		planetShader.setFloat("shininess", 32.0f);


		for (auto& r : roots) r->draw(planetShader);


		bloom.render();   // bright pass -> multi-level blur -> tone map + gamma -> screen

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//VAO.cleanup();
	delete sphere;
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	if (width == 0 || height == 0) return;   // minimised
	glViewport(0, 0, width, height);
	if (g_bloom) g_bloom->resize(width, height);   // the HDR / bloom targets must match the window

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
