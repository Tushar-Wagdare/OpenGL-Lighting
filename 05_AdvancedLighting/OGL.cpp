#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <gl/GL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "WindowManager.h"
#include "Logger.h"
#include "Timer.h"
#include "camera.h"
#include "Shader.h"
#include "model.h"



//*** Globle Function Declarations ***
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//*** Global Variable Declaration ***
WindowManager* pWindow = NULL;
Camera* camera = NULL;
float lastX = WindowManager::SCR_WIDTH / 2.0f;
float lastY = WindowManager::SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



///==================== OpenGL Variables =======================///
GLuint vaoCube = 0;
GLuint vbo_position_cube = 0;
GLuint vbo_normal_cube = 0;

GLfloat angleCube = 0.0f;
// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
bool blinn = false;
bool blinnKeyPressed = false;




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	unsigned int loadTexture(char const* path);
	pWindow = new WindowManager();
	MSG msg = { 0 };
	camera = new Camera();

	Logger::Init();

	TIMER_INIT("Window");
	pWindow->initialize();
	TIMER_END(); 
	LOG_INFO("Window Initialized in %.6f seconds", TIMER_GET("Window"));



	///======================== OpenGL INIT ==============================///
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// build and compile shaders
	// -------------------------
	Shader shader("shaders/advanced_lighting.vs", "shaders/advanced_lighting.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float planeVertices[] = {
		// positions            // normals         // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};
	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load textures
	// -------------
	unsigned int floorTexture = loadTexture("resources/container2.png");

	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("texture1", 0);

	// lighting info
	// -------------
	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);





	//*** Game LOOP ***
	while (pWindow->isRunning == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				pWindow->isRunning = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			///=========================== DISPLAY ==================================//
			// per-frame time logic
			// --------------------
			float currentFrame = static_cast<float>(Timer::getAppRunTime() * 0.05f);
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;


			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// draw objects
			shader.use();
			glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)WindowManager::SCR_WIDTH / (float)WindowManager::SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = camera->GetViewMatrix();
			shader.setMat4("projection", projection);
			shader.setMat4("view", view);
			// set light uniforms
			shader.setVec3("viewPos", camera->Position);
			shader.setVec3("lightPos", lightPos);
			shader.setInt("blinn", blinn);
			// floor
			glBindVertexArray(planeVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, floorTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			std::cout << (blinn ? "Blinn-Phong" : "Phong") << std::endl;


			pWindow->swapDisplayBuffer();

			///================== UPDATE =======================//
			//angleCube = angleCube + 0.02f;

		}
	}


	return((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//*** Function Declaration ***
	void ToggleFullscreen(void);
	void resize(int, int);


	//*** Code ***
	switch (iMsg)
	{
	case WM_SETFOCUS:
		break;

	case WM_KILLFOCUS:
		break;

	case WM_SIZE:
        glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_MOUSEMOVE:
		{
			float xpos = static_cast<float>(GET_X_LPARAM(lParam));
			float ypos = static_cast<float>(GET_Y_LPARAM(lParam));

			if (firstMouse)
			{
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

			lastX = xpos;
			lastY = ypos;
			camera->ProcessMouseMovement(xoffset, yoffset);
		}
		
		break;

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_CHAR:
		switch (LOWORD(wParam))
		{
		case 'F':
		case 'f':
			
			break;

		case 'W':
		case 'w':
			camera->ProcessKeyboard(FORWARD, deltaTime);
			break;

		case 'A':
		case 'a':
			camera->ProcessKeyboard(LEFT, deltaTime);
			break;

		case 'S':
		case 's':
			camera->ProcessKeyboard(BACKWARD, deltaTime);
			break;

		case 'D':
		case 'd':
			camera->ProcessKeyboard(RIGHT, deltaTime);
			break;

		
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}


	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

