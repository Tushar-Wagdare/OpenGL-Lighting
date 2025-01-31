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
#include "Sphere.h"



//*** Globle Function Declarations ***
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//*** Global Variable Declaration ***
WindowManager* pWindow = NULL;
Camera* camera = NULL;
float lastX = WindowManager::SCR_WIDTH / 2.0f;
float lastY = WindowManager::SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;



///==================== OpenGL Variables =======================///
GLuint vao = 0;
GLuint vbo_position = 0;
GLuint vbo_normal = 0;
GLuint vbo_element = 0;
GLuint vbo_texcord = 0;

GLfloat angleCube = 0.0f;
GLint W = 0;
GLint H = 0;
///Sphere
GLint gNumVertices = 0;///
GLuint numSphereElements = 0;
GLfloat lightAmbient[] = { 0.1f,0.1f,0.1f,1.0f };//Grey Light
GLfloat lightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };//White Light
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 100.0f,100.0f,100.0f,1.0f };//Positional Light

glm::vec4 materialAmbient = { 0.0f,0.0f,0.0f,1.0f };
glm::vec4 materialDiffuse = { 1.0f,1.0f,1.0f,1.0f };
glm::vec4 materialSpecular = { 1.0f,1.0f,1.0f,1.0f };
GLfloat materialShininess = 128.0f;




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
	glEnable(GL_DEPTH_TEST);

	
	Shader ourShader("shaders/phong.vs", "shaders/phong.fs");
	

	//Declare Position And Color Arrays
	///SPHERE
	float sphere_positions[1146];
	float sphere_normals[1146];
	float sphere_texcoords[764];
	unsigned short sphere_elements[2280];
	getSphereVertexData(sphere_positions, sphere_normals, sphere_texcoords, sphere_elements);
	numSphereElements = getNumberOfSphereElements();

	/// SPHERE 
	/// vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/// position vbo
	glGenBuffers(1, &vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_positions), sphere_positions, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/// normal vbo
	glGenBuffers(1, &vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/// texcoords vbo
	glGenBuffers(1, &vbo_texcord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_texcord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_texcoords), sphere_texcoords, GL_STATIC_DRAW);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/// element vbo
	glGenBuffers(1, &vbo_element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	/// unbind vao
	glBindVertexArray(0);
	//-------------------------------------------------------------------------------------//

	//07 - 2nd step Enabling Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);


	//08 - Set the Clear Color of Window To Blue
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);//dark grey



	glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)WindowManager::SCR_WIDTH / (float)WindowManager::SCR_HEIGHT, 0.1f, 100.0f);




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
			float currentFrame = static_cast<float>(Timer::getAppRunTime() * 0.05f);
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;


			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			ourShader.use();


			glm::mat4 modelMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = glm::mat4(1.0f);


			modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -5.0f));
			//modelMatrix = glm::rotate(modelMatrix, angleCube, glm::vec3(1.0f, 0.0f, 0.0f));
			//modelMatrix = glm::rotate(modelMatrix, angleCube, glm::vec3(0.0f, 1.0f, 0.0f));
			//modelMatrix = glm::rotate(modelMatrix, angleCube, glm::vec3(0.0f, 0.0f, 1.0f));
			viewMatrix = camera->GetViewMatrix();

			ourShader.setMat4("uModelMatrix", modelMatrix);
			ourShader.setMat4("uViewMatrix", viewMatrix);
			ourShader.setMat4("uProjectionMatrix", projection);

			ourShader.setVec3("uLightAmbient", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
			ourShader.setVec3("uLightDiffuse", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			ourShader.setVec3("uLightSpecular", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			ourShader.setVec3("uLightPosition", glm::vec4(1.2f, 1.0f, 2.0f, 1.0f));

			ourShader.setVec3("uMaterialAmbient", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			ourShader.setVec3("uMaterialDiffuse", glm::vec4(0.5f, 0.2f, 0.7f, 1.0f));
			ourShader.setVec3("uMaterialSpecular", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
			ourShader.setFloat("uMaterialShininess", 128.0f);
			
			
			
			///COLOM 1---------------------------------------------------------------
			///first colomn precius stone 1
			///emerald material
			{
				materialAmbient[0] = 0.0215;
				materialAmbient[1] = 0.1745;
				materialAmbient[2] = 0.0215;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.07568;
				materialDiffuse[1] = 0.61424;
				materialDiffuse[2] = 0.07568;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.633;
				materialSpecular[1] = 0.727811;
				materialSpecular[2] = 0.633;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.6 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(0, 0, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}

			///first colomn precius stone 2
			///jade material
			{
				materialAmbient[0] = 0.135;
				materialAmbient[1] = 0.2225;
				materialAmbient[2] = 0.1575;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.54;
				materialDiffuse[1] = 0.89;
				materialDiffuse[2] = 0.63;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.316228;
				materialSpecular[1] = 0.316228;
				materialSpecular[2] = 0.316228;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.1 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(0, (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///first colomn precius stone 3
			///obsidean material
			{
				materialAmbient[0] = 0.05375;
				materialAmbient[1] = 0.05;
				materialAmbient[2] = 0.06625;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.18275;
				materialDiffuse[1] = 0.17;
				materialDiffuse[2] = 0.22525;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.332741;
				materialSpecular[1] = 0.328634;
				materialSpecular[2] = 0.346435;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.3 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(0, (GLsizei)H / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///first colomn precius stone 4
			///pearl material
			{
				materialAmbient[0] = 0.25;
				materialAmbient[1] = 0.20725;
				materialAmbient[2] = 0.20725;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 1.0;
				materialDiffuse[1] = 0.829;
				materialDiffuse[2] = 0.829;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.296648;
				materialSpecular[1] = 0.296648;
				materialSpecular[2] = 0.296648;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.088 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(0, (GLsizei)H / 2, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///first colomn precius stone 5
			///ruby material
			{
				materialAmbient[0] = 0.1745;
				materialAmbient[1] = 0.01175;
				materialAmbient[2] = 0.01175;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.61424;
				materialDiffuse[1] = 0.04136;
				materialDiffuse[2] = 0.04136;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.727811;
				materialSpecular[1] = 0.626959;
				materialSpecular[2] = 0.626959;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.6 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(0, (2 * (GLsizei)H) / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///first colomn precius stone 6
			///turquoise material
			{
				materialAmbient[0] = 0.1;
				materialAmbient[1] = 0.18725;
				materialAmbient[2] = 0.1745;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.396;
				materialDiffuse[1] = 0.74151;
				materialDiffuse[2] = 0.69102;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.297254;
				materialSpecular[1] = 0.30829;
				materialSpecular[2] = 0.306678;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.1 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(0, 5 * (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}



			///COLOM 2---------------------------------------------------------------
			///second colomn precius stone 1
			///brass material
			{
				materialAmbient[0] = 0.329412;
				materialAmbient[1] = 0.223529;
				materialAmbient[2] = 0.027451;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.780392;
				materialDiffuse[1] = 0.568627;
				materialDiffuse[2] = 0.113725;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.992157;
				materialSpecular[1] = 0.941176;
				materialSpecular[2] = 0.807843;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.21794872 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 18, 0, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///second colomn precius stone 2
			///bronze material
			{
				materialAmbient[0] = 0.2125;
				materialAmbient[1] = 0.1275;
				materialAmbient[2] = 0.054;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.714;
				materialDiffuse[1] = 0.4284;
				materialDiffuse[2] = 0.18144;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.393548;
				materialSpecular[1] = 0.271906;
				materialSpecular[2] = 0.166721;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.2 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 18, (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///second colomn precius stone 3
			///chrome material
			{
				materialAmbient[0] = 0.25;
				materialAmbient[1] = 0.25;
				materialAmbient[2] = 0.25;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.4;
				materialDiffuse[1] = 0.4;
				materialDiffuse[2] = 0.4;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.774597;
				materialSpecular[1] = 0.774597;
				materialSpecular[2] = 0.774597;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.6 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 18, (GLsizei)H / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///second colomn precius stone 4
			///copper material
			{
				materialAmbient[0] = 0.19125;
				materialAmbient[1] = 0.0735;
				materialAmbient[2] = 0.0225;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.7038;
				materialDiffuse[1] = 0.27048;
				materialDiffuse[2] = 0.0828;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.256777;
				materialSpecular[1] = 0.137622;
				materialSpecular[2] = 0.086014;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.1 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 18, (GLsizei)H / 2, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///second colomn precius stone 5
			///gold material
			{
				materialAmbient[0] = 0.24725;
				materialAmbient[1] = 0.1995;
				materialAmbient[2] = 0.0745;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.75164;
				materialDiffuse[1] = 0.60648;
				materialDiffuse[2] = 0.22648;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.628281;
				materialSpecular[1] = 0.555802;
				materialSpecular[2] = 0.366065;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.4 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 18, (2 * (GLsizei)H) / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///second colomn precius stone 6
			///silver material
			{
				materialAmbient[0] = 0.19225;
				materialAmbient[1] = 0.19225;
				materialAmbient[2] = 0.19225;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.50754;
				materialDiffuse[1] = 0.50754;
				materialDiffuse[2] = 0.50754;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.508273;
				materialSpecular[1] = 0.508273;
				materialSpecular[2] = 0.508273;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.4 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 18, 5 * (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}



			///COLOM 3---------------------------------------------------------------
			///third colomn precius stone 1
			///black material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.0;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.01;
				materialDiffuse[1] = 0.01;
				materialDiffuse[2] = 0.01;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.50;
				materialSpecular[1] = 0.50;
				materialSpecular[2] = 0.50;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.25 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 9, 0, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///third colomn precius stone 2
			///cyan material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.1;
				materialAmbient[2] = 0.06;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.0;
				materialDiffuse[1] = 0.50980392;
				materialDiffuse[2] = 0.50980392;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.50196078;
				materialSpecular[1] = 0.50196078;
				materialSpecular[2] = 0.50196078;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.25 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 9, (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///third colomn precius stone 3
			///green material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.0;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.1;
				materialDiffuse[1] = 0.35;
				materialDiffuse[2] = 0.1;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.45;
				materialSpecular[1] = 0.45;
				materialSpecular[2] = 0.45;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.25 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 9, (GLsizei)H / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///third colomn precius stone 4
			///red material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.0;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.5;
				materialDiffuse[1] = 0.0;
				materialDiffuse[2] = 0.0;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.7;
				materialSpecular[1] = 0.6;
				materialSpecular[2] = 0.6;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.25 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 9, (GLsizei)H / 2, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///third colomn precius stone 5
			///white material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.0;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.55;
				materialDiffuse[1] = 0.55;
				materialDiffuse[2] = 0.55;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.70;
				materialSpecular[1] = 0.70;
				materialSpecular[2] = 0.70;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.25 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 9, (2 * (GLsizei)H) / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///third colomn precius stone 6
			///yellow plastic material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.0;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.5;
				materialDiffuse[1] = 0.5;
				materialDiffuse[2] = 0.0;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.60;
				materialSpecular[1] = 0.60;
				materialSpecular[2] = 0.50;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.25 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 9, 5 * (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}



			///COLOM 4---------------------------------------------------------------
			///fourth colomn precius stone 1
			///black material
			{
				materialAmbient[0] = 0.02;
				materialAmbient[1] = 0.02;
				materialAmbient[2] = 0.02;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.01;
				materialDiffuse[1] = 0.01;
				materialDiffuse[2] = 0.01;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.4;
				materialSpecular[1] = 0.4;
				materialSpecular[2] = 0.4;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.078125 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 6, 0, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///fourth colomn precius stone 2
			///cyan material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.05;
				materialAmbient[2] = 0.05;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.4;
				materialDiffuse[1] = 0.5;
				materialDiffuse[2] = 0.5;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.04;
				materialSpecular[1] = 0.7;
				materialSpecular[2] = 0.7;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.078125 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 6, (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///fourth colomn precius stone 3
			///green material
			{
				materialAmbient[0] = 0.0;
				materialAmbient[1] = 0.05;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.4;
				materialDiffuse[1] = 0.5;
				materialDiffuse[2] = 0.4;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.04;
				materialSpecular[1] = 0.7;
				materialSpecular[2] = 0.04;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.078125 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 6, (GLsizei)H / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///fourth colomn precius stone 4
			///red material
			{
				materialAmbient[0] = 0.05;
				materialAmbient[1] = 0.0;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.5;
				materialDiffuse[1] = 0.4;
				materialDiffuse[2] = 0.4;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.7;
				materialSpecular[1] = 0.04;
				materialSpecular[2] = 0.04;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.078125 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 6, (GLsizei)H / 2, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///fourth colomn precius stone 5
			///white material
			{
				materialAmbient[0] = 0.05;
				materialAmbient[1] = 0.05;
				materialAmbient[2] = 0.05;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.05;
				materialDiffuse[1] = 0.05;
				materialDiffuse[2] = 0.05;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.7;
				materialSpecular[1] = 0.7;
				materialSpecular[2] = 0.7;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.078125 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 6, (2 * (GLsizei)H) / 3, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}


			///fourth colomn precius stone 6
			///yellow rubber material
			{
				materialAmbient[0] = 0.05;
				materialAmbient[1] = 0.05;
				materialAmbient[2] = 0.0;
				materialAmbient[3] = 1.0f;

				materialDiffuse[0] = 0.5;
				materialDiffuse[1] = 0.5;
				materialDiffuse[2] = 0.4;
				materialDiffuse[3] = 1.0f;

				materialSpecular[0] = 0.7;
				materialSpecular[1] = 0.7;
				materialSpecular[2] = 0.04;
				materialSpecular[3] = 1.0f;

				materialShininess = 0.078125 * 128;

				ourShader.setVec3("uMaterialAmbient", materialAmbient);
				ourShader.setVec3("uMaterialDiffuse", materialDiffuse);
				ourShader.setVec3("uMaterialSpecular", materialSpecular);
				ourShader.setFloat("uMaterialShininess", materialShininess);
				glViewport(5 * (GLsizei)W / 6, 5 * (GLsizei)H / 6, (GLsizei)W / 6, (GLsizei)H / 6);

				glBindVertexArray(vao);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element);
				glDrawElements(GL_TRIANGLES, numSphereElements, GL_UNSIGNED_SHORT, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}

			glBindVertexArray(0);
			glUseProgram(0);


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
		W = LOWORD(lParam);
		H = HIWORD(lParam);
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