
#include "../include/main.h"

struct Particle{
	glm::vec3 pos, speed;
	unsigned char r,g,b,a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};
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
	int no_particles = 200;
	Mesh mlights[no_particles];
	Particle particles[no_particles];



	mesh[0].loadOBJ("models/Earth_Triangulated.obj");

	std::vector<glm::vec3> light_pos;
	for(int i = 0; i < no_particles; i++ ){
		mlights[i].loadOBJ("models/square.obj");
		light_pos.push_back(glm::vec3(rand() % 10, 0.0 , 0.0));

		int particleIndex = i;
		particles[particleIndex].life = 5.0f; // This particle will live 5 seconds.
		particles[particleIndex].pos = glm::vec3(4,0,0.0f);

		float spread = 1.5f;
		glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);

		glm::vec3 randomdir = glm::vec3(
			(rand()%2000 - 1000.0f)/1000.0f,
			(rand()%2000 - 1000.0f)/1000.0f,
			(rand()%2000 - 1000.0f)/1000.0f
		);
		
		particles[particleIndex].speed = maindir + randomdir*spread;


		// Very bad way to generate a random color
		particles[particleIndex].r = rand() % 256;
		particles[particleIndex].g = rand() % 256;
		particles[particleIndex].b = rand() % 256;
		particles[particleIndex].a = (rand() % 256) / 3;
		// std::cout << glm::to_string(light_pos[i]);
	}
	// mlights[0].loadOBJ("models/square.obj");
	// mlights[1].loadOBJ("models/square.obj");

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
		// std::cout << rand_num(10, 20) << std::endl;
		// // Point Light 1
		// lightingShader.setUniform("pointLights[0].ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		// lightingShader.setUniform("pointLights[0].diffuse", glm::vec3(0.0f, 1.0f, 0.1f)); // green-ish light
		// lightingShader.setUniform("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		// lightingShader.setUniform("pointLights[0].position", pointLightPos[0]);
		// lightingShader.setUniform("pointLights[0].constant", 1.0f);
		// lightingShader.setUniform("pointLights[0].linear", 0.0022f);
		// lightingShader.setUniform("pointLights[0].exponent", 0.0020f);


		// Spot light
		glm::vec3 spotlightPos = fpsCamera.getPosition();
		// std::cout << "POS : " << glm::to_string(fpsCamera.getPosition()) << std::endl;
		// std::cout << "LOOK : " << glm::to_string(fpsCamera.getLook()) << std::endl;
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

		for(int i =0; i< no_particles; i++){
			bulb.use();
			Particle& p = particles[i]; 
			p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)deltaTime * 0.5f;
			p.pos += p.speed * (float)deltaTime;
			p.life -= deltaTime;


			// bulb.setUniform("lightColor", glm::vec3(particles[i].r, particles[i].g, particles[i].b));
			bulb.setUniform("lightColor", glm::vec3(1.0f, 1.0f, 0.0f));
			bulb.setUniform("model", glm::translate(glm::mat4(1.0), particles[i].pos) * glm::scale(glm::mat4(1.0), glm::vec3(0.02, 0.02, 0.02)));
			bulb.setUniform("view", view);
			bulb.setUniform("projection", projection);
			mlights[0].draw();
		}


		// Swap front and back buffers
		glfwSwapBuffers(gWindow);
		mac_patch(gWindow);
		lastTime = currentTime;
	}

	glfwTerminate();

	return 0;
}

