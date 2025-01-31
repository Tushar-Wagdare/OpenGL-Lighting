#include <windows.h>
#include<windowsx.h>
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

#include "WindowManager.h"
#include "Logger.h"
#include "Timer.h"
#include "camera.h"
#include "Shader.h"



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

GLuint modelViewMatrixUniform = 0;
GLuint projectionMatrixUniform = 0;
GLuint ldUniform = 0;
GLuint kdUniform = 0;
GLuint lightPositionUniform = 0;
GLuint keyPressUniform = 0;

GLfloat angleCube = 0.0f;






int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	pWindow = new WindowManager();
	MSG msg = { 0 };
	camera = new Camera();

	Logger::Init();

	TIMER_INIT("Window");
	pWindow->initialize();
	TIMER_END(); 
	LOG_INFO("Window Initialized in %.6f seconds", TIMER_GET("Window"));



	///======================== OpenGL INIT ==============================///
	Shader ourShader("shaders/diffuse.vs", "shaders/diffuse.fs");

	///CUBE
	const GLfloat cube_position[] =
	{
		// top
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// bottom
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,

		 // front
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,

		 // back
		 1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		 // right
		 1.0f, 1.0f, -1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, -1.0f,

		 // left
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
	};


	const GLfloat cube_normals[] =
	{
		// front surface
		0.0f,  0.0f,  1.0f, // top-right of front
		0.0f,  0.0f,  1.0f, // top-left of front
		0.0f,  0.0f,  1.0f, // bottom-left of front
		0.0f,  0.0f,  1.0f, // bottom-right of front

		// right surface
		1.0f,  0.0f,  0.0f, // top-right of right
		1.0f,  0.0f,  0.0f, // top-left of right
		1.0f,  0.0f,  0.0f, // bottom-left of right
		1.0f,  0.0f,  0.0f, // bottom-right of right

		// back surface
		0.0f,  0.0f, -1.0f, // top-right of back
		0.0f,  0.0f, -1.0f, // top-left of back
		0.0f,  0.0f, -1.0f, // bottom-left of back
		0.0f,  0.0f, -1.0f, // bottom-right of back

		// left surface
	   -1.0f,  0.0f,  0.0f, // top-right of left
	   -1.0f,  0.0f,  0.0f, // top-left of left
	   -1.0f,  0.0f,  0.0f, // bottom-left of left
	   -1.0f,  0.0f,  0.0f, // bottom-right of left

	   // top surface
	   0.0f,  1.0f,  0.0f, // top-right of top
	   0.0f,  1.0f,  0.0f, // top-left of top
	   0.0f,  1.0f,  0.0f, // bottom-left of top
	   0.0f,  1.0f,  0.0f, // bottom-right of top

	   // bottom surface
	   0.0f, -1.0f,  0.0f, // top-right of bottom
	   0.0f, -1.0f,  0.0f, // top-left of bottom
	   0.0f, -1.0f,  0.0f, // bottom-left of bottom
	   0.0f, -1.0f,  0.0f, // bottom-right of bottom
	};

	/// CUBE 
	//VAO
	glGenVertexArrays(1, &vaoCube);
	glBindVertexArray(vaoCube);

	//VBO for Position 
	glGenBuffers(1, &vbo_position_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_position), cube_position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);


	///VBO for normal
	glGenBuffers(1, &vbo_normal_cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_normals), cube_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	

	//07 - 2nd step Enabling Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	//08 - Set the Clear Color of Window To Blue
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ourShader.use();
	glm::mat4 perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)WindowManager::SCR_WIDTH / (float)WindowManager::SCR_HEIGHT, 0.1f, 100.0f);
	

	// world space positions of our cubes
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};




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


			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ourShader.use();


			glm::mat4 modelMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = glm::mat4(1.0f);


			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -3.0f));
			modelMatrix = glm::rotate(modelMatrix, angleCube, glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, angleCube, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, angleCube, glm::vec3(0.0f, 0.0f, 1.0f));
			viewMatrix = camera->GetViewMatrix();

			ourShader.setMat4("uModelViewMatrix", viewMatrix * modelMatrix);
			ourShader.setMat4("uProjectionMatrix", perspectiveProjectionMatrix);
			ourShader.setVec3("uld", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			ourShader.setVec3("ukd", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
			ourShader.setVec3("uLightPosition", glm::vec4(0.0f, 0.0f, 2.0f, 1.0f));
			
			
			
			glBindVertexArray(vaoCube);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

			glBindVertexArray(0);
			glUseProgram(0);


			pWindow->swapDisplayBuffer();

			///================== UPDATE =======================//
			angleCube = angleCube + 0.02f;

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