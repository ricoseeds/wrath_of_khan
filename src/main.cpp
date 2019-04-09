#include <iostream>
#include <sstream>
#include <string>
#define GLEW_STATIC
#include "GL/glew.h" // Important - this header must come before glfw3 header
#include "GLFW/glfw3.h"
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "../include/ShaderProgram.h"
#include "../include/Texture2D.h"
#include "../include/Camera.h"
#include "../include/Mesh.h"

// Global Variables
const char *APP_TITLE = "Introduction to Modern OpenGL - Multiple Lights";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow *gWindow = NULL;
bool gWireframe = false;
bool gFlashlightOn = true;
glm::vec4 gClearColor(0.06f, 0.06f, 0.07f, 1.0f);
static bool mac_moved = false;

FPSCamera fpsCamera(glm::vec3(0.0f, 3.5f, 10.0f));
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 5.0; // units per second
const float MOUSE_SENSITIVITY = 0.1f;

// Function prototypes
void glfw_onKey(GLFWwindow *window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow *window, int width, int height);
void glfw_onMouseScroll(GLFWwindow *window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow *window);
bool initOpenGL();
void mac_patch(GLFWwindow *window);
//-----------------------------------------------------------------------------
// Main Application Entry Point
//-----------------------------------------------------------------------------
int main()
{
	if (!initOpenGL())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram lightingShader;
	lightingShader.loadShaders("shaders/lighting_dir_point_spot.vert", "shaders/lighting_dir_point_spot.frag");
	ShaderProgram bulb;
	bulb.loadShaders("shaders/bulb.vert", "shaders/bulb.frag");

	// Load meshes and textures
	const int numModels = 1;
	Mesh mesh[numModels];
	Texture2D texture[numModels];
	int no_particles = 1;
	Mesh mlights[no_particles];

	mesh[0].loadOBJ("models/Earth_Triangulated.obj");
	mlights[0].loadOBJ("models/square.obj");

	texture[0].loadTexture("textures/Earth_TEXTURE_CM.tga", true);
	// texture[1].loadTexture("textures/light_diffuse.jpg", true);

	// Model positions
	glm::vec3 modelPos[] = {
		glm::vec3(0.0f, 0.0f, 0.0f) // barrel

	};

	// Model scale
	glm::vec3 modelScale[] = {
		glm::vec3(1.0f, 1.0f, 1.0f) // barrel

	};

	// Point Light positions
	glm::vec3 pointLightPos[3] = {
		glm::vec3(-5.0f, 3.8f, 0.0f),
		glm::vec3(0.5f, 3.8f, 0.0f),
		glm::vec3(5.0f, 3.8, 0.0f)};

	double lastTime = glfwGetTime();

	// Rendering loop
	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;

		// Poll for and process events
		glfwPollEvents();
		update(deltaTime);

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0), view(1.0), projection(1.0);

		// Create the View matrix
		view = fpsCamera.getViewMatrix();

		// Create the projection matrix
		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 200.0f);

		// update the view (camera) position
		glm::vec3 viewPos;
		viewPos.x = fpsCamera.getPosition().x;
		viewPos.y = fpsCamera.getPosition().y;
		viewPos.z = fpsCamera.getPosition().z;

		// Must be called BEFORE setting uniforms because setting uniforms is done
		// on the currently active shader program.
		lightingShader.use();
		lightingShader.setUniform("model", glm::mat4(1.0)); // do not need to translate the models so just send the identity matrix
		lightingShader.setUniform("view", view);
		lightingShader.setUniform("projection", projection);
		lightingShader.setUniform("viewPos", viewPos);

		// Directional light
		lightingShader.setUniform("sunLight.direction", glm::vec3(0.0f, -0.9f, -0.17f));
		lightingShader.setUniform("sunLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		lightingShader.setUniform("sunLight.diffuse", glm::vec3(0.1f, 0.1f, 0.1f)); // dark
		lightingShader.setUniform("sunLight.specular", glm::vec3(0.1f, 0.1f, 0.1f));

		// // Point Light 1
		// lightingShader.setUniform("pointLights[0].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		// lightingShader.setUniform("pointLights[0].diffuse", glm::vec3(0.0f, 1.0f, 0.1f)); // green-ish light
		// lightingShader.setUniform("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		// lightingShader.setUniform("pointLights[0].position", pointLightPos[0]);
		// lightingShader.setUniform("pointLights[0].constant", 1.0f);
		// lightingShader.setUniform("pointLights[0].linear", 0.0022f);
		// lightingShader.setUniform("pointLights[0].exponent", 0.0020f);

		// // Point Light 2
		// lightingShader.setUniform("pointLights[1].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		// lightingShader.setUniform("pointLights[1].diffuse", glm::vec3(1.0f, 0.1f, 0.0f)); // red-ish light
		// lightingShader.setUniform("pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		// lightingShader.setUniform("pointLights[1].position", pointLightPos[1]);
		// lightingShader.setUniform("pointLights[1].constant", 1.0f);
		// lightingShader.setUniform("pointLights[1].linear", 0.0022f);
		// lightingShader.setUniform("pointLights[1].exponent", 0.0020f);

		// // Point Light 3
		// lightingShader.setUniform("pointLights[2].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		// lightingShader.setUniform("pointLights[2].diffuse", glm::vec3(0.0f, 0.1f, 1.0f)); // blue-ish light
		// lightingShader.setUniform("pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		// lightingShader.setUniform("pointLights[2].position", pointLightPos[2]);
		// lightingShader.setUniform("pointLights[2].constant", 1.0f);
		// lightingShader.setUniform("pointLights[2].linear", 0.0022f);
		// lightingShader.setUniform("pointLights[2].exponent", 0.0020f);

		// Spot light
		glm::vec3 spotlightPos = fpsCamera.getPosition();
		std::cout << "POS : " << glm::to_string(fpsCamera.getPosition()) << std::endl;
		std::cout << "LOOK : " << glm::to_string(fpsCamera.getLook()) << std::endl;
		// offset the flash light down a little
		spotlightPos.y -= 0.5f;

		lightingShader.setUniform("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		lightingShader.setUniform("spotLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		lightingShader.setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("spotLight.position", glm::vec3(0.982347, 3.500000, 10.248156));
		lightingShader.setUniform("spotLight.direction", glm::vec3(-0.202902, -0.470038, -0.859008));
		lightingShader.setUniform("spotLight.cosInnerCone", glm::cos(glm::radians(15.0f)));
		lightingShader.setUniform("spotLight.cosOuterCone", glm::cos(glm::radians(20.0f)));
		lightingShader.setUniform("spotLight.constant", 1.0f);
		lightingShader.setUniform("spotLight.linear", 0.07f);
		lightingShader.setUniform("spotLight.exponent", 0.017f);
		lightingShader.setUniform("spotLight.on", gFlashlightOn);

		// Render the scene
		for (int i = 0; i < numModels; i++)
		{
			// if (i==1)
			// {
			// 	model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i]) * glm::rotate(glm::mat4(1.0), glm::radians((float)(glfwGetTime() * 100.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
			// }
			// else
			// {
				model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i]); // * glm::rotate(glm::mat4(1.0), glm::radians((float)(glfwGetTime() * 100.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
			// }
			// model = glm::translate(glm::mat4(1.0), modelPos[i]) * glm::scale(glm::mat4(1.0), modelScale[i]); // * glm::rotate(glm::mat4(1.0), glm::radians((float)(glfwGetTime() * 100.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
			// ;
			lightingShader.setUniform("model", model);

			// Set material properties
			lightingShader.setUniform("material.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
			lightingShader.setUniformSampler("material.diffuseMap", 0);
			lightingShader.setUniform("material.specular", glm::vec3(0.8f, 0.8f, 0.8f));
			lightingShader.setUniform("material.shininess", 32.0f);

			texture[i].bind(0); // set the texture before drawing.  Our simple OBJ mesh loader does not do materials yet.
			mesh[i].draw();		// Render the OBJ mesh
			texture[i].unbind(0);
		}
		bulb.use();
        bulb.setUniform("lightColor", glm::vec3(1.0f, 1.0f, 0.0f));
        bulb.setUniform("model", glm::translate(glm::mat4(1.0), glm::vec3(4.0f, 0.0f, 0.0f)));
        bulb.setUniform("view", view);
        bulb.setUniform("projection", projection);
        mlights[0].draw();

		// Swap front and back buffers
		glfwSwapBuffers(gWindow);
		mac_patch(gWindow);
		lastTime = currentTime;
	}

	glfwTerminate();

	return 0;
}

//-----------------------------------------------------------------------------
// Initialize GLFW and OpenGL
//-----------------------------------------------------------------------------
bool initOpenGL()
{
	// Intialize GLFW
	// GLFW is configured.  Must be called before calling any GLFW functions
	if (!glfwInit())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // forward compatible with newer versions of OpenGL as they become available but not backward compatible (it will not run on devices that do not support OpenGL 3.3

	// Create an OpenGL 3.3 core, forward compatible context window
	gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
	if (gWindow == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	// Make the window's context the current one
	glfwMakeContextCurrent(gWindow);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// Set the required callback functions
	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);

	// // Hides and grabs cursor, unlimited movement
	// glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	glClearColor(gClearColor.r, gClearColor.g, gClearColor.b, gClearColor.a);

	// Define the viewport dimensions
	glViewport(0, 0, gWindowWidth, gWindowHeight);
	glEnable(GL_DEPTH_TEST);

	return true;
}

//-----------------------------------------------------------------------------
// Is called whenever a key is pressed/released via GLFW
//-----------------------------------------------------------------------------
void glfw_onKey(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		gWireframe = !gWireframe;
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		// toggle the flashlight
		gFlashlightOn = !gFlashlightOn;
	}
}

//-----------------------------------------------------------------------------
// Is called when the window is resized
//-----------------------------------------------------------------------------
void glfw_onFramebufferSize(GLFWwindow *window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;
	glViewport(0, 0, gWindowWidth, gWindowHeight);
}

//-----------------------------------------------------------------------------
// Called by GLFW when the mouse wheel is rotated
//-----------------------------------------------------------------------------
void glfw_onMouseScroll(GLFWwindow *window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

//-----------------------------------------------------------------------------
// Update stuff every frame
//-----------------------------------------------------------------------------
void update(double elapsedTime)
{
	// Camera orientation
	double mouseX, mouseY;

	// Get the current mouse cursor position delta
	// glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	// // Rotate the camera the difference in mouse distance from the center screen.  Multiply this delta by a speed scaler
	// fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	// // Clamp mouse cursor to center of screen
	// glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	// Camera FPS movement

	// Forward/backward
	if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
	else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

	// Strafe left/right
	if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
	else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

	// Up/down
	if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * glm::vec3(0.0f, 1.0f, 0.0f));
	else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -glm::vec3(0.0f, 1.0f, 0.0f));
}

//-----------------------------------------------------------------------------
// Code computes the average frames per second, and also the average time it takes
// to render one frame.  These stats are appended to the window caption bar.
//-----------------------------------------------------------------------------
void showFPS(GLFWwindow *window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime(); // returns number of seconds since GLFW started, as double float

	elapsedSeconds = currentSeconds - previousSeconds;

	// Limit text updates to 4 times per second
	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		// The C++ way of setting the window title
		std::ostringstream outs;
		outs.precision(3); // decimal places
		outs << std::fixed
			 << APP_TITLE << "    "
			 << "FPS: " << fps << "    "
			 << "Frame Time: " << msPerFrame << " (ms)";
		glfwSetWindowTitle(window, outs.str().c_str());

		// Reset for next average.
		frameCount = 0;
	}

	frameCount++;
}
void mac_patch(GLFWwindow *window)
{
	if (glfwGetTime() > 3.0)
	{
		mac_moved = true;
	}
	// glfwGetTim

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE && !mac_moved)
	{
		int x, y;
		glfwGetWindowPos(window, &x, &y);
		glfwSetWindowPos(window, ++x, y);
	}
	else
	{
		mac_moved = true;
	}
}