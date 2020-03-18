// Ben Schreyer OpenGL collision compute




//The Cherno Youtube has good videos on OpenGL graphics



//constants
#define CIRCLE_POINTS 64 // for circle drawing
#define PI 3.14159265358979323846264338327950288f
#define NUM_PARTICLES 100000
#define DELTA_TIME 1/300.0f

#define CELLSIZE (5)
//measured length of the sides of the containing box
#define XSIZE (40 * CELLSIZE)//4
#define YSIZE (60 * CELLSIZE)
#define ZSIZE (60 * CELLSIZE)

//dimensions of the 3d cube hash
#define XCELLS (XSIZE / CELLSIZE)
#define YCELLS (YSIZE / CELLSIZE)
#define ZCELLS (ZSIZE / CELLSIZE)

//use an overestimate of sphere packing density to calculate the amount of spheres a single hash bucket can hold
#define SPHERESPERCELL  ((unsigned int)((float)CELLSIZE * (float)CELLSIZE * (float)CELLSIZE * 0.81 / 4.0f * 3.0f / PI))



//OpenGL and window creation includes
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//utility includes
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <chrono>


//The Cherno Youtube Series OpenGL utilities with expanded uniform setters for uniform types not used in his early videos
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"

//New OpenGL utility based off of Shader.h
#include "ComputeShader.h"

//glm for matrix math 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

//user interface tools
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
//3rd party not mine


//structure representing a 4d position equivalent to vec4 in OpenGL, w is not used
struct position
{
	float x, y, z, w;
};
//structure representing a 4d velocity equivalent to vec4 in OpenGL, w is not used
struct velocity
{
	float vx, vy, vz, vw;
};
//structure representing a 4d color equivalent to vec4 in OpenGL first three values represent red green and blue intensity and the last represents alpha or opacity
struct color
{
	float r, g, b, a;
};
//basically a macro to generate a random number 0.0-1.0
float randFloat()
{
	return ((float)(rand() % 10000)) / 10000.0f;
}


//not mine based off of an online OpenGL control tutorial, handles mouse input changing camera rotation
bool firstMouse = false;
float lastX = 0.0f;
float lastY = 0.0f;
float yaw = 0.0f;
float pitch = 0.0f;
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}


int main(void)
{
	

	//initialize glfw window creator
	if (!glfwInit())
	{
		std::cout << "glfwInit() failed";
		return -1;
	}
	//window object, 1 window
	GLFWwindow* window;




	//OpenGL version 4.3 indicator 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1920, 1080, "OpenGL Compute Shaders For Physics", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);//0 no cap on fps,1 will cap fps to refresh rate 

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glewInit() != GLEW_OK)
	{
		std::cout << "GLEW NOT OK\n";
	}

	//make pseudo randoms change for each launch
	srand(time(NULL));




	//track how full each bucket is allocation and 0
	unsigned int* bucketIndex = new unsigned int[(XCELLS + 1) * (YCELLS + 1) * (ZCELLS + 1)];
	for (int i = 0; i < (XCELLS + 1) * (YCELLS + 1) * (ZCELLS + 1); i++)
	{
		bucketIndex[i] = 0;
	}



	//stores the ID OpenGL tracks each of the 4 used buffers by. color velocity position and buckets
	unsigned int positionSSbo;
	unsigned int velocitySSbo;
	unsigned int colorSSbo;
	unsigned int bucketSSbo;


	//disables/enables physics simulation
	bool run = false;
	

	//time step for simulation
	float deltaTime = DELTA_TIME;

	float length = XSIZE;
	float width = YSIZE;
	float height = ZSIZE;

	//maximum speed particles will start with
	float speedMax = 00.0000006471663187;



	//
	//how are we going to use our access to gpu memory buffers
	int bufMask = GL_MAP_WRITE_BIT;


	//generate a buffer for the buckets 
	GLCall(glGenBuffers(1, &bucketSSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, bucketSSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, (XCELLS + 1) * (YCELLS + 1) * (ZCELLS + 1) * SPHERESPERCELL * sizeof(unsigned int), 0, GL_DYNAMIC_READ));

	
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, bucketSSbo));
	//get a pointer to gpu memory for use cpu side
	GLCall(unsigned int* buckets = (unsigned int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE));

	//generate a buffer for the buckets and fill it with random positions within the bounding box
	GLCall(glGenBuffers(1, &positionSSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionSSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct position), NULL, GL_DYNAMIC_READ));


	GLCall(GL_SHADER_STORAGE_BUFFER, glBindBuffer(positionSSbo));
	//get a pointer to gpu memory for use cpu side
	GLCall(struct position* points = (struct position*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE));

	for (int i = 0; i < NUM_PARTICLES; i++)
	{


		if (i < 10)
		{
			points[i].x =0.32f + randFloat() * width / 1.16f;
			points[i].y = 1.1f + randFloat() * height / 1.0;

			points[i].z = 0.2f + randFloat() * length / 1.16f;

			points[i].w = 1;
		}
		else
		{
			points[i].x = 0.2f + randFloat() * XSIZE / 3.3f + randFloat() * XSIZE / 3.3f;
			points[i].y = 05.0f + randFloat() * YSIZE / 2.3 + randFloat() * YSIZE / 2.6;

			points[i].z = 0.4f + randFloat() * ZSIZE / 4.22f + randFloat() * ZSIZE / 4.22f;

			points[i].w = 1;
		}
		
	}







	//generate a buffer so velocity data and generate random velocitys based off specified max speed
	GLCall(glGenBuffers(1, &velocitySSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitySSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct velocity), NULL, GL_STATIC_DRAW));
	//get a pointer to velocities in gpu memory
	GLCall(struct velocity* velocities = (struct velocity*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct velocity), bufMask));
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		if (i < 10)
		{
			velocities[i].vx = 42.0f * (randFloat() - 0.5f) * speedMax + 32.0 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vy = 42.0f * (randFloat() - 0.5f) * speedMax + 32.0 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vz = 42.0f * (randFloat() - 0.5f) * speedMax + 32.0 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vw = 1.0f;
		}
		else
		{
			velocities[i].vx = 4.5f * (randFloat() - 0.5f) * speedMax + 5.5 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vy = 1.5f * (randFloat() - 0.5f) * speedMax + 0.5 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vz = 4.5f * (randFloat() - 0.5f) * speedMax + 5.5 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vw = 1.0f;
		}
	}
	
	GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
	//generate a buffer for color data of spheres and give them one of 3 colors 
	GLCall(glGenBuffers(1, &colorSSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorSSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct color), NULL, GL_STATIC_DRAW));
	//get a pointer to gpu buffer storing color data
	GLCall(struct color* colors = (struct color*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct color), bufMask));
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		float rand = randFloat();
			if (rand < 0.33)
			{
				colors[i].r = 0.00 + 0.1 * randFloat();
				colors[i].g = 0.61 + 0.1 * randFloat();
				colors[i].b = 0.0 + 0.1 * randFloat();
				colors[i].a = 1.0f;
			}
			else if (rand < 0.66)
			{
				colors[i].r = 0.60 + 0.1 * randFloat();
				colors[i].g = 0.01 + 0.1 * randFloat();
				colors[i].b = 0.0 + 0.1 * randFloat();
				colors[i].a = 1.0f;
			}
			else {
				colors[i].r = 0.00 + 0.1 * randFloat();
				colors[i].g = 0.01 + 0.1 * randFloat();
				colors[i].b = 0.6 + 0.1 * randFloat();
				colors[i].a = 1.0f;
			}

	}

	GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));







	//use the compute shader utility to compile an OpenGL compute shader and give it access to the data buffers on gpu that it needs, this one is for eulers method and bounding box collisions
	ComputeShader VelocityGravityWallEuler("res/shaders/VelocityGravityWallEuler.shader");
	VelocityGravityWallEuler.SSBOID.push_back(positionSSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(4);
	VelocityGravityWallEuler.SSBOID.push_back(velocitySSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(5);
	VelocityGravityWallEuler.SSBOID.push_back(colorSSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(6);
	VelocityGravityWallEuler.SSBOID.push_back(bucketSSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(8);

	//use the compute shader utility to compile an OpenGL compute shader and give it access to the data buffers on gpu that it needs, this one is for resolving collisions within a single bucket
	ComputeShader BucketResolver("res/shaders/BucketResolver.shader");
	BucketResolver.SSBOID.push_back(positionSSbo);
	BucketResolver.SSBOPOS.push_back(4);
	BucketResolver.SSBOID.push_back(velocitySSbo);
	BucketResolver.SSBOPOS.push_back(5);
	BucketResolver.SSBOID.push_back(bucketSSbo);
	BucketResolver.SSBOPOS.push_back(8);

	//create a vertex and fragment shader similar to compute for drawing spheres
	Shader shader("res/shaders/Basic.shader");




	GLCall(std::cout << glGetString(GL_VERSION) << "\n");
	GLCall(glEnable(GL_DEPTH_TEST));
	{


		//each point of a circle mesh centered at 0,0 with radius one is defined by xyz coordinates and the 3 color values rgb
		//6 floats for each vertex, there are n+1 vertexs to draw a circle with triangles where n is the amount of sides on the circle
		float circlePoints[CIRCLE_POINTS * 6 + 6];
		//center of the circle manualy set as it doesnt follow a simple pattern as the outside points do
		circlePoints[0] = 0.0f;
		circlePoints[0 + 1] = 0.0f;
		circlePoints[0 + 2] = 0.0f;
		circlePoints[0 + 3] = 0.0f;
		circlePoints[0 + 4] = 0.0f;
		circlePoints[0 + 5] = 1.0f;
		
		//generate the edge points of a circle and give each point a color which is not used in this program a parametric for a circle is y=sin(t) x=cos(t) z=0
		for (int i = 0; i < CIRCLE_POINTS; i++)
		{
			circlePoints[(i + 1) * 6] = glm::cos(i * 2 * PI / CIRCLE_POINTS);
			circlePoints[(i + 1) * 6 + 1] = glm::sin(i * 2 * PI / CIRCLE_POINTS);
			circlePoints[(i + 1) * 6 + 2] = 0.0f;
			circlePoints[(i + 1) * 6 + 3] = (rand() % 100) / (float)100;
			circlePoints[(i + 1) * 6 + 4] = (rand() % 100) / (float)100;
			circlePoints[(i + 1) * 6 + 5] = (rand() % 100) / (float)100;
		}
		//create an array of indices into our circle points array that tells OpenGL what triangles to draw, each triangles drawn for the circle uses the center point and then 2 points along the edge, hence 	circleIndices[i * 3] = 0;
		unsigned int circleIndices[3 * CIRCLE_POINTS];
		for (int i = 0; i < CIRCLE_POINTS; i++)
		{
			circleIndices[i * 3] = 0;
			circleIndices[i * 3 + 1] = 1 + i;
			circleIndices[i * 3 + 2] = 2 + i;
			if (i == CIRCLE_POINTS - 1)
				circleIndices[i * 3 + 2] = 1;
		}

		//use The Chernos utilitys to send the geometry to OpenGL
		IndexBuffer circleIb(circleIndices, 3 * (CIRCLE_POINTS));


		VertexArray circleVa;
		VertexBuffer circleVb(circlePoints, (CIRCLE_POINTS * 6 + 6) * sizeof(float));

		VertexBufferLayout circleLayout;
		circleLayout.Push<float>(3);
		circleLayout.Push<float>(3);
		circleVa.AddBuffer(circleVb, circleLayout);

	
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));



		//define camera matrices and a projection form 1080p with far rear culling 
		glm::mat4 view = glm::mat4(1.0f);
		glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
		glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
		glm::mat4 projection = glm::mat4(1.0f);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		cameraPos = cameraPos + glm::vec3(1.0f, 0.0f, 0.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)1920 / (float)1080, 0.1f, 100000000.0f);



	

		//The Cherno drawing utility
		Renderer renderer;
		//initialize user interface drawing lib
		ImGui::CreateContext();
		ImGui_ImplGlfwGL3_Init(window, true);
		ImGui::StyleColorsDark();


		glm::mat4 mvp;
	
	//simulation and rendering loop
		while (!glfwWindowShouldClose(window))
		{
			//start simulation once the Z Key is pressed
			if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
				run = true;
			
			//user interface utility
				ImGui_ImplGlfwGL3_NewFrame();

				/* Render here */
				//clear screen buffer
				renderer.Clear();
	


				{

			
			//in its own scope create an fps debug window
					ImGui::Text("Application average %.6f ms/frame (%.1f FPS)\n Number Of Particles:%d", 1.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate, NUM_PARTICLES);
				}
				//simulate physics
				if (run)
				{
					//bind to position buffer and bucket buffer to do cpu side hnashing
					GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, positionSSbo, 4));
					GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bucketSSbo, 8));
					//hash every particle into a grid aligned with the bounding box
					for (int i = 0; i < NUM_PARTICLES; i++)
					{


						//find out which cell the sphere is in
						unsigned int hash = YCELLS * ZCELLS * ((unsigned int)(points[i].x) / CELLSIZE) + ZCELLS * ((unsigned int)(points[i].y) / CELLSIZE) + ((unsigned int)(points[i].z) / CELLSIZE) % ((ZCELLS + 0) * (XCELLS + 0) * (YCELLS + 0));

				
				//place the id of the particle + 1 into the right bucket, +1 becuase 0 represents the end of particle ids in a bucket
						buckets[hash * SPHERESPERCELL + (bucketIndex[hash] % SPHERESPERCELL)] = i + 1;
				//move to next sub index for the bucket used
						bucketIndex[hash]++;
				
					}
					//wait for OpenGL to deal with memory setting doen by cpu
					GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
				

					//decale that we are using the collision resolver with buckets program and give it the information it needs
					GLCall(glUseProgram(BucketResolver.m_RendererID));
					BucketResolver.SetUniform1f("cellSize", (float)CELLSIZE);
					BucketResolver.SetUniform1f("xSizeB", (float)XSIZE);
					BucketResolver.SetUniform1f("ySizeB", (float)YSIZE);
					BucketResolver.SetUniform1f("zSizeB", (float)ZSIZE);
					BucketResolver.SetUniform1f("spheresPerCell", (float)SPHERESPERCELL);
					BucketResolver.SetUniform1f("deltaTime", (float)deltaTime);
					//compute XCELLS * YCELLS * ZCELLS buckets
					BucketResolver.Compute((XCELLS * YCELLS * ZCELLS) / 1, 1, 1);
					GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));

					GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
					//clear the sub index array
					memset(bucketIndex, 0, XCELLS * YCELLS * ZCELLS);
					//wait for compute to finnish
					GLCall(glFinish());

					//this is the exact same as above but the particles are offset by half the cell size in x y and z an rehashed and recollided to allow for all possible collisions to be detected, increases cell edge stability
					for (int i = 0; i < NUM_PARTICLES; i++)
					{

						unsigned int hash = ((YCELLS + 1) * (ZCELLS + 1) * (((unsigned int)(points[i].x + (float)CELLSIZE / 2.0f)) / (CELLSIZE)) + (ZCELLS + 1) * (((unsigned int)(points[i].y + (float)CELLSIZE / 2.0f)) / (CELLSIZE)) + (((unsigned int)(points[i].z + (float)CELLSIZE / 2.0f) / (CELLSIZE)))) % ((ZCELLS + 1) * (XCELLS + 1) * (YCELLS + 1));
					
					
						buckets[hash * SPHERESPERCELL + (bucketIndex[hash] % SPHERESPERCELL)] = i + 1;

	
						bucketIndex[hash]++;

					}

					GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
					BucketResolver.Compute(((XCELLS + 1) * (YCELLS + 1) * (ZCELLS + 1)), 1, 1);
					GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
					memset(bucketIndex, 0, (XCELLS + 1) * (YCELLS + 1) * (ZCELLS + 1));
					GLCall(glFinish());
					//call the compute program to apply gravity and eulers method physics
					GLCall(glUseProgram(VelocityGravityWallEuler.m_RendererID));
					VelocityGravityWallEuler.SetUniform1f("deltaTime", deltaTime);

					VelocityGravityWallEuler.SetUniform1f("xSizeB", (float)XSIZE);
					VelocityGravityWallEuler.SetUniform1f("ySizeB", (float)YSIZE);
					VelocityGravityWallEuler.SetUniform1f("zSizeB", (float)ZSIZE);
					VelocityGravityWallEuler.Compute(NUM_PARTICLES / 1, 1, 1);
					//compute physics for each particle
					GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
					GLCall(glFinish());
				}
				//bind the buffers needed to draw spheres and draw them for each ball simulated
				GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, positionSSbo));
				GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, colorSSbo));
				GLCall(glUseProgram(shader.m_RendererID));
			
				shader.SetUniformMat4f("u_V", view);
				shader.SetUniformMat4f("u_P", projection);

				circleVa.Bind();
	
				circleIb.Bind();

				GLCall(glDrawElementsInstanced(GL_TRIANGLES, 3 + 3 * CIRCLE_POINTS, GL_UNSIGNED_INT, nullptr, NUM_PARTICLES));
			
				
				ImGui::Render();
				ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
				/* Swap front and back buffers */
				glfwSwapBuffers(window);

				/* Poll for and process events */

				glfwPollEvents();
				//move camera based off user input and update camera angle based off of mouse
				float cameraSpeed = 700.0f; 
		
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
					cameraPos += cameraSpeed * cameraFront / ImGui::GetIO().Framerate;;
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
					cameraPos -= cameraSpeed * cameraFront / ImGui::GetIO().Framerate;;
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
					cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)) / ImGui::GetIO().Framerate;;
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
					cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)) / ImGui::GetIO().Framerate;;
				view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);



		}
		//glfw input
		glfwPollEvents();
	}
	//"clean" exit
		glfwDestroyWindow(window);
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
		return 0;
	}
