//SDL Libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_image.h>
//OpenGL Libraries
#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include <stack>

#include "Cube.h"
#include "Sphere.h"
//GML libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "logger.h"


#define WIDTH     1024
#define HEIGHT    768
#define FRAMERATE 60
#define TIME_PER_FRAME_MS  (1.0f/FRAMERATE * 1e3)
#define INDICE_TO_PTR(x) ((void*)(x))

float t = 0;
float t1 = 0.f;
float t2 = 0.f;
float t3 = 0.f;
float t4 = 0.f;
float t5 = 0.f;
float t6 = 0.f;
float t7 = 0.f;
float t8 = 0.f;

struct Material {
	glm::vec3 color;
	float ka;
	float kd;
	float ks;
	float alpha;
};

struct Light {
	glm::vec3 position;
	glm::vec3 color;
};

struct GameObject {
	GLuint VboID = 0;
	GLuint textureid = 0;
	Geometry* geometry = nullptr;
	glm::mat4 propagatedMatrix = glm::mat4(1.0f);
	glm::mat4 localMatrix = glm::mat4(1.0f);
	std::vector<GameObject*> children;
	Material mtl;
	bool visible = true;
	bool isSun = false;
};

Material sphereMtl{ {0.0f, 0.0f, 1.0f}, 0.3f, 0.5f, 0.2f, 50 };


void draw(GameObject& go, Shader* shader, std::stack<glm::mat4>& matrices, glm::mat4 projection, glm::mat4 view) {

	
	glm::mat4 model = matrices.top() * go.localMatrix;
	glm::mat4 mvp = projection*view*model;

	if (go.visible)
	{
		glUseProgram((shader->getProgramID()));
		{
			Material sphereMtl;
			if (go.isSun) sphereMtl = { {1,1,1},1,0,0 ,100};
			else sphereMtl = { {0.0f, 0.0f, 0.0f}, 0.2f, 0.0f, 0.8f, 100 };

			Light light{ {-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f} };
			GLint vNormal = glGetAttribLocation(shader->getProgramID(), "vNormal");
			glBindBuffer(GL_ARRAY_BUFFER, go.VboID);
			glBindTexture(GL_TEXTURE_2D, go.textureid);
			glActiveTexture(GL_TEXTURE0);
			glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, INDICE_TO_PTR(go.geometry->getNbVertices() * 3 * sizeof(float)));

			glEnableVertexAttribArray(vNormal);
			GLint uMVP = glGetUniformLocation(shader->getProgramID(), "uMVP");
			GLint UV = glGetAttribLocation(shader->getProgramID(), "vUV");
			glVertexAttribPointer(UV, 2, GL_FLOAT, GL_FALSE, 0, INDICE_TO_PTR(go.geometry->getNbVertices() * 6 * sizeof(float)));
			glEnableVertexAttribArray(UV);
			glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
			GLint uModel = glGetUniformLocation(shader->getProgramID(), "uModel");
			GLint uInvModel3x3 = glGetUniformLocation(shader->getProgramID(), "uInvModel3x3");
			glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix3fv(uInvModel3x3, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::inverse(model))));
			GLint uMtlColor = glGetUniformLocation(shader->getProgramID(), "uMtlColor");
			GLint uMtlCts = glGetUniformLocation(shader->getProgramID(), "uMtlCts");
			GLint uLightPos = glGetUniformLocation(shader->getProgramID(), "uLightPos");
			GLint uLightColor = glGetUniformLocation(shader->getProgramID(), "uLightColor");
			GLint uCameraPosition = glGetUniformLocation(shader->getProgramID(), "uCameraPosition");
			glUniform3f(uMtlColor, 1, 1, 1);
			glUniform4f(uMtlCts, sphereMtl.ka, sphereMtl.ks, sphereMtl.kd, sphereMtl.alpha);
			glUniform3f(uLightPos, 0,0,0);
			glUniform3f(uLightColor, 1, 1, 1);
			glUniform3f(uCameraPosition, 0,0,0);
			GLint vPosition = glGetAttribLocation(shader->getProgramID(), "vPosition");
			glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glEnableVertexAttribArray(vPosition);

			glDrawArrays(GL_TRIANGLES, 0, go.geometry->getNbVertices());


		}
		glUseProgram(0);
	}
	matrices.push(matrices.top() * go.propagatedMatrix);
	for (int i = 0; i < go.children.size(); i++)
		draw(*(go.children[i]), shader, matrices, projection, view);

	matrices.pop();

}

GLuint inittexture(std::string path) {
	SDL_Surface* img = IMG_Load(path.c_str());
	SDL_Surface* rgbImg = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
	SDL_FreeSurface(img);

	GLuint textureID;
	glGenTextures(1, &textureID);
	std::cout << glGetError() << std::endl;
	glBindTexture(GL_TEXTURE_2D, textureID);
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgbImg->w, rgbImg->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)rgbImg->pixels);
		glGenerateMipmap(textureID);

	}
	glBindTexture(GL_TEXTURE_2D, 0);
	SDL_FreeSurface(rgbImg);
	return textureID;
}


int main(int argc, char *argv[])
{
	////////////////////////////////////////
	//SDL2 / OpenGL Context initialization :
	////////////////////////////////////////

	//Initialize SDL2
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		ERROR("The initialization of the SDL failed : %s\n", SDL_GetError());
		return 0;
	}

	//Create a Window
	SDL_Window* window = SDL_CreateWindow("Systeme solaire navigable texturise a l'echelle 1/100",                           //Titre
		SDL_WINDOWPOS_UNDEFINED,               //X Position
		SDL_WINDOWPOS_UNDEFINED,               //Y Position
		WIDTH, HEIGHT,                         //Resolution
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN); //Flags (OpenGL + Show)

//Initialize OpenGL Version (version 3.0)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	//Initialize the OpenGL Context (where OpenGL resources (Graphics card resources) lives)
	SDL_GLContext context = SDL_GL_CreateContext(window);

	//Tells GLEW to initialize the OpenGL function with this version
	glewExperimental = GL_TRUE;
	glewInit();


	//Start using OpenGL to draw something on screen
	glViewport(0, 0, WIDTH, HEIGHT); //Draw on ALL the screen

	//The OpenGL background color (RGBA, each component between 0.0f and 1.0f)
	glClearColor(0.0, 0.0, 0.0, 1.0); //Full Black

	glEnable(GL_DEPTH_TEST); //Active the depth test

	//TODO
	//
	bool stop=false;
	Sphere sphere(32, 32);
	GLuint vboSphereID;
	glGenBuffers(1, &vboSphereID);
	glBindBuffer(GL_ARRAY_BUFFER, vboSphereID);
	glBufferData(GL_ARRAY_BUFFER, sphere.getNbVertices() * (3 + 3 +2) * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sphere.getNbVertices() * 3 * sizeof(float), sphere.getVertices());
	glBufferSubData(GL_ARRAY_BUFFER, sphere.getNbVertices() * 3 * sizeof(float), sphere.getNbVertices() * 3 * sizeof(float), sphere.getNormals());
	glBufferSubData(GL_ARRAY_BUFFER, sphere.getNbVertices() * 6 * sizeof(float), sphere.getNbVertices() * 2 * sizeof(float), sphere.getUVs());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GameObject sun;
	GameObject mercure;
	GameObject venus;
	GameObject earth;
	GameObject mars;
	GameObject jupiter;
	GameObject saturne;
	GameObject uranus;
	GameObject neptune;
	GameObject lune;
	GameObject star;

	sun.children.push_back(&mercure);
	sun.children.push_back(&venus);
	sun.children.push_back(&earth);
	sun.children.push_back(&mars);
	sun.children.push_back(&jupiter);
	sun.children.push_back(&saturne);
	sun.children.push_back(&uranus);
	sun.children.push_back(&neptune);
	earth.children.push_back(&lune);

	sun.geometry = &sphere;
	sun.VboID = vboSphereID;
	sun.isSun = true;
	sun.textureid = inittexture("textures/8k_sun.jpg");


	mercure.geometry = &sphere;
	mercure.VboID = vboSphereID;
	mercure.textureid = inittexture("textures/8k_mercury.jpg");

	venus.geometry = &sphere;
	venus.VboID = vboSphereID;
	venus.textureid = inittexture("textures/8k_venus_surface.jpg");

	earth.geometry = &sphere;
	earth.VboID = vboSphereID;
	earth.textureid = inittexture("textures/8k_earth.jpg");

	mars.geometry = &sphere;
	mars.VboID = vboSphereID;
	mars.textureid = inittexture("textures/8k_mars.jpg");

	jupiter.geometry = &sphere;
	jupiter.VboID = vboSphereID;
	jupiter.textureid = inittexture("textures/8k_jupiter.jpg");

	saturne.geometry = &sphere;
	saturne.VboID = vboSphereID;
	saturne.textureid = inittexture("textures/8k_saturn.jpg");

	uranus.geometry = &sphere;
	uranus.VboID = vboSphereID;
	uranus.textureid = inittexture("textures/2k_uranus.jpg");

	neptune.geometry = &sphere;
	neptune.VboID = vboSphereID;
	neptune.textureid = inittexture("textures/2k_neptune.jpg");

	lune.geometry = &sphere;
	lune.VboID = vboSphereID;
	lune.textureid = inittexture("textures/8k_moon.jpg");

	star.geometry = &sphere;
	star.VboID = vboSphereID;
	star.isSun = true;
	star.textureid = inittexture("textures/8k_stars.jpg");

	
	//
	//From here you can load your OpenGL objects, like VBO, Shaders, etc.
	//
	//TODO
	FILE* vertFile = fopen("Shaders/color.vert", "r");
	FILE* fragFile = fopen("Shaders/color.frag", "r");
	if (vertFile == NULL || fragFile == NULL)
	{
		std::cout << "Ouverture des fichiers Shaders impossible\n";
		return 1;
	}
	Shader* shader = Shader::loadFromFiles(vertFile, fragFile);
	fclose(vertFile);
	fclose(fragFile);
	if (shader == NULL)
	{
		return EXIT_FAILURE;
	}

	bool isOpened = true;
	float keyZ = 0.0f;
	float keyQ = 0.0f;
	float keyS = 0.0f;
	float keyD = 0.0f;
	float keySPACE = 0.0f;
	float keyLSHIFT = 0.0f;
	bool keyA = false;
	bool keyE = false;
	float positionX = 0.5f;
	float positionY = 0.0f;
	float positionZ = 0.0f;
	float delta = 0.01f;
	float cameraAngle = 0.0f;

	//Main application loop
	while (isOpened)
	{
		//Time in ms telling us when this frame started. Useful for keeping a fix framerate
		uint32_t timeBegin = SDL_GetTicks();

		//Fetch the SDL events
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					isOpened = false;
					break;
				default:
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					keyZ = 0.05f;
					break;
				case SDLK_LEFT:
					keyQ = 0.05f;
					break;
				case SDLK_DOWN:
					keyS = 0.05f;
					break;
				case SDLK_RIGHT:
					keyD = 0.05f;
					break;
				case SDLK_SPACE:
					keySPACE = 0.05f;
					break;
				case SDLK_LSHIFT:
					keyLSHIFT = 0.05f;
					break;
				case SDLK_a:
					stop = !stop;
					break;
				default:break;
				}
				break;
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
				case SDLK_UP:
					keyZ = 0.0f;
					break;
				case SDLK_LEFT:
					keyQ = 0.0f;
					break;
				case SDLK_DOWN:
					keyS = 0.0f;
					break;
				case SDLK_RIGHT:
					keyD = 0.0f;
					break;
				case SDLK_SPACE:
					keySPACE = 0.0f;
					break;
				case SDLK_LSHIFT:
					keyLSHIFT = 0.0f;
					break;
				case SDLK_a:
					break;
				default:break;
				}
				break;
			}
		}

		positionX = positionX + keyD - keyQ;
		positionZ = positionZ - keyZ + keyS;
		positionY = positionY + keySPACE - keyLSHIFT;



		//Clear the screen : the depth buffer and the color buffer
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);



		//TODO rendering

		
		glm::mat4 camera = glm::translate(glm::mat4(1.f), { 0,0,25 });
		glm::vec3 cameraPosition(positionX, positionY, positionZ);
		camera = glm::translate(camera, cameraPosition);
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f)); 
		model = glm::rotate(model, cameraAngle, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 view = glm::inverse(camera);
		//glm::mat4 view = glm::translate(glm::mat4(1.f), {0,0,-1});
		glm::mat4 projection = glm::perspective(3.1415 / 2, 800. / 600., 0.001, 10000.);
		std::stack<glm::mat4> matrices;
		matrices.push(glm::mat4(1.0f));


		sun.localMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(3.f, 3.f, 3.f));
		star.localMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(100.f, 100.f, 100.f));

		if(!stop) {
			t1 += 0.02f;
			t2 += 0.016f;
			t3 += 0.012f;
			t4 += 0.008f;
			t5 += 0.004f;
			t6 += 0.002f;
			t7 += 0.0010f;
			t8 += 0.00050f;
		}

		earth.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t3, glm::vec3(0.0f, 1.0f, 0.0f)) *glm::translate(glm::mat4(1.0f), glm::vec3(+7.50f, 0.0f, 0.0f));
		earth.localMatrix = glm::rotate(glm::mat4(1.0f), t3, glm::vec3(0.0f, 1.0f, 0.0f)) *glm::translate(glm::mat4(1.0f), glm::vec3(+7.50f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(0.92f, 0.92f, 0.92f));

		mercure.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t1, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+2.25f, 0.0f, 0.0f));
		mercure.localMatrix = glm::rotate(glm::mat4(1.0f), t1, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+2.25f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(0.35f, 0.35f, 0.35f));

		venus.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t2, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+5.00f, 0.0f, 0.0f));
		venus.localMatrix = glm::rotate(glm::mat4(1.0f), t2, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+5.00f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(0.87f, 0.87f, 0.87f));

		mars.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t4, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+12.40f, 0.0f, 0.0f));
		mars.localMatrix = glm::rotate(glm::mat4(1.0f), t4, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+12.40f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(0.49f, 0.49f, 0.49f));

		saturne.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t6, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+27.50f, 0.0f, 0.0f));
		saturne.localMatrix = glm::rotate(glm::mat4(1.0f), t6, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+27.50f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(4.87f, 4.87f, 4.87f));

		jupiter.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t5, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+17.45f, 0.0f, 0.0f));
		jupiter.localMatrix = glm::rotate(glm::mat4(1.0f), t5, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+17.45f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(5.026f, 5.026f, 5.026f));

		uranus.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t7, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+33.55f, 0.0f, 0.0f));
		uranus.localMatrix = glm::rotate(glm::mat4(1.0f), t7, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+33.55f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(2.37f, 2.37f, 2.37f));

		neptune.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t8, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+38.60f, 0.0f, 0.0f));
		neptune.localMatrix = glm::rotate(glm::mat4(1.0f), t8, glm::vec3(0.0f, 1.0f, 0.0f))*glm::translate(glm::mat4(1.0f), glm::vec3(+38.60f, 0.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(2.36f, 2.36f, 2.36f));

		lune.propagatedMatrix = glm::rotate(glm::mat4(1.0f), t3, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(+1.0f, 0.0f, 0.0f));
		lune.localMatrix = glm::rotate(glm::mat4(1.0f), t3, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(+1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.15f, 0.15f, 0.15f));

		draw(sun, shader, matrices, projection, view);
		draw(star, shader, matrices, projection, view);

		 //Display on screen (swap the buffer on screen and the buffer you are drawing on)
		SDL_GL_SwapWindow(window);

		//Time in ms telling us when this frame ended. Useful for keeping a fix framerate
		uint32_t timeEnd = SDL_GetTicks();

		//We want FRAMERATE FPS
		if (timeEnd - timeBegin < TIME_PER_FRAME_MS)
			SDL_Delay((uint32_t)(TIME_PER_FRAME_MS)-(timeEnd - timeBegin));
	}

	//Free everything
	if (context != NULL)
		SDL_GL_DeleteContext(context);
	if (window != NULL)
		SDL_DestroyWindow(window);

	return 0;
}
