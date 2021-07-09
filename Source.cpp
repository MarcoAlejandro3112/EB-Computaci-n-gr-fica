#include <math.h>
#include <vector>
#include <time.h>
#include <glutil.h>
#include <figures.h>
#include <camera.h>


i32 n = 4;
bool reposicionar = false;
bool disparar = false;
const u32 FSIZE = sizeof(f32);
const u32 ISIZE = sizeof(u32);
const u32 SCRWIDTH = 1280;
const u32 SCRHEIGHT = 720;
const f32 ASPECT = (f32)SCRWIDTH / (f32)SCRHEIGHT;

//posición de la luz
glm::vec3 lightPos(50.0f, 20.0f, 100.0f); 
//posición nave
glm::vec3 nave = glm::vec3(2.0f,2.0f, 18.0f);
//posición bala
glm::vec3 bala = glm::vec3(1.0f, 1.0f, 17.23f);

Cam* cam;

f32 lastx;
f32 lasty;
f32 deltaTime = 0.0f;
f32 lastFrame = 0.0f;
bool firstmouse = true;
bool wireframe = false;

/**
 * keyboard input processing
 **/
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		reposicionar = true;	
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam->processKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam->processKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam->processKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam->processKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		wireframe = true;
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		wireframe = false;
	}
	//nave
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (nave.y + deltaTime * 5 <= n*2 - 1) {
			nave.y += deltaTime * 5;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (nave.y - deltaTime * 5 >= 0) {
			nave.y -= deltaTime * 5;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (nave.x - deltaTime * 5 >= 0) {
			nave.x -= deltaTime * 5;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (nave.x + deltaTime * 5 < n*2 - 1) {
			nave.x += deltaTime * 5;
		}
	}
}

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (firstmouse) {
			lastx = xpos;
			lasty = ypos;
			firstmouse = false;
			return;
		}
		cam->processMouse((f32)(xpos - lastx), (f32)(lasty - ypos));
		lastx = xpos;
		lasty = ypos;
	}
	else {
		firstmouse = true;
	}
}

void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) {
	cam->processScroll((f32)yoffset);
}

i32 main() {
	srand(time(NULL));
	GLFWwindow* window = glutilInit(3, 3, SCRWIDTH, SCRHEIGHT, "Space Invaders");
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	Shader* lightingShader = new Shader("lightingmaps.vert", "lightingmaps.frag", "", "resources/textures");
	Shader* lightCubeShader = new Shader("lightcube.vert", "lightcube.frag","", "resources/textures");

	cam = new Cam();
	glm::vec3 lightColor = glm::vec3(1.0f);

	Cube* cubex = new Cube();

	int count = 0;
	std::vector<glm::vec3> positions(n * n * n);
	std::vector<bool> state(n * n * n);
	for (u32 i = 0; i < n; ++i) {
		for (u32 j = 0; j < n; ++j) {
			for (u32 k = 0; k < n; ++k) {
				f32 x = i + 1 * i;
				f32 z = 1;
				f32 y = k + 1 * k;
				positions[count] = glm::vec3(x, y, z);
				state[count] = true;
				count++;
			}
		}
	}

	auto rndb = [](f32 a, f32 b) -> f32 {
		f32 d = b - a;
		return (rand() % 1000) / 1000.0f * d + a;
	};

	bala.x = rndb(1.0, n - 1.0);
	bala.y = rndb(1.0, n - 1.0);

	u32 bx = rand() % 2;
	u32 by = rand() % 2;


	u32 cubeVao, lightCubeVao, vbo, ebo;
	glGenVertexArrays(1, &cubeVao);
	glGenVertexArrays(1, &lightCubeVao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ARRAY_BUFFER, cubex->getVSize() * FSIZE,
		cubex->getVertices(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubex->getISize() * ISIZE,
		cubex->getIndices(), GL_STATIC_DRAW);

	// posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);
	// normales: ojo que es el 3er comp, por eso offset es 6
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(6));
	glEnableVertexAttribArray(1);
	// textures
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(9));
	glEnableVertexAttribArray(2);

	glBindVertexArray(lightCubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);

	unsigned int txAliens = lightingShader->loadTexture("alien.png");
	unsigned int txBala = lightingShader->loadTexture("fireball.png");
	unsigned int txNave = lightingShader->loadTexture("spaceship.jpg");

	while (!glfwWindowShouldClose(window)) {
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::perspective(cam->getZoom(), ASPECT, 0.1f, 100.0f);
		lightingShader->useProgram();
		glm::mat4 model = glm::mat4(1.0f);
		glBindVertexArray(cubeVao);

		i32 count = 0;
		for (u32 i = 0; i < positions.size(); ++i) {
			if (state[i] == false) {
				count++;
			}
		}
		if (count == positions.size()) {
			glfwSetWindowShouldClose(window, true);
		}
		for (u32 i = 0; i < positions.size(); ++i) {
			positions[i] = glm::vec3(positions[i].x, positions[i].y, positions[i].z + 0.001); //para que los aliens se acerquen
			if (positions[i].z > 18) {
				glfwSetWindowShouldClose(window, true);
			}
			if (!state[i]) continue;

			glBindTexture(GL_TEXTURE_2D,txAliens);
			lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
			lightingShader->setF32("xyzMat.shininess", 64.0f);

			lightingShader->setVec3("xyzLht.position", lightPos);
			lightingShader->setVec3("xyz", cam->getPos());

			lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
			lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
			lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

			lightingShader->setMat4("proj", proj);
			lightingShader->setMat4("view", cam->getViewM4());

			model = glm::mat4(1.0f);
			model = glm::translate(model, positions[i]);
			lightingShader->setMat4("model", model);
			glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);


			bool colisionx = (positions[i].x + 0.5 > bala.x - 0.5 && positions[i].x - 0.5 < bala.x + 0.5);
			bool colisiony = (positions[i].y + 0.5 > bala.y - 0.5 && positions[i].y - 0.5 < bala.y + 0.5);
			bool colisionz = (positions[i].z + 0.5 > bala.z - 0.5 && positions[i].z - 0.5 < bala.z + 0.5);

			if (colisionx && colisiony && colisionz) {
				state[i] = false;
				reposicionar = false;
			}
		}

		//nave
		glBindTexture(GL_TEXTURE_2D, txNave);
		lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
		lightingShader->setF32("xyzMat.shininess", 64.0f);

		lightingShader->setVec3("xyzLht.position", lightPos);
		lightingShader->setVec3("xyz", cam->getPos());

		lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
		lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
		lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

		lightingShader->setMat4("proj", proj);
		lightingShader->setMat4("view", cam->getViewM4());

		model = glm::mat4(1.0f);
		model = glm::translate(model, nave);
		model = glm::scale(model, glm::vec3(2.0, 2.0, 0.5));
		lightingShader->setMat4("model", model);
		glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);


		//bala
		if (reposicionar) {
			bala.x = nave.x;
			bala.y = nave.y;
			bala.z = nave.z - 1;
			disparar = true;
			reposicionar = false;
		}
		if (disparar) {
			bala.z -= 0.1;

			glBindTexture(GL_TEXTURE_2D, txBala);
			lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
			lightingShader->setF32("xyzMat.shininess", 64.0f);

			lightingShader->setVec3("xyzLht.position", lightPos);
			lightingShader->setVec3("xyz", cam->getPos());

			lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
			lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
			lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

			lightingShader->setMat4("proj", proj);
			lightingShader->setMat4("view", cam->getViewM4());

			model = glm::mat4(1.0f);
			model = glm::translate(model, bala);
			lightingShader->setMat4("model", model);
			glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);
		}

		//luz
		lightCubeShader->useProgram();
		lightCubeShader->setMat4("proj", proj);
		lightCubeShader->setMat4("view", cam->getViewM4());
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.05));
		lightCubeShader->setMat4("model", model);

		glBindVertexArray(lightCubeVao);
		glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	};

	glDeleteVertexArrays(1, &cubeVao);
	glDeleteVertexArrays(1, &lightCubeVao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	delete lightingShader;
	delete lightCubeShader;
	delete cubex;
	delete cam;

	return 0;
}