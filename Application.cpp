#define NUM_PARTICLES 12800
#define WIDTH 200
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <chrono>
//#include <thread>
#include "Renderer.h"

#include "ComputeShader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"
#include "tests/TestClearColor.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
//3rd party not mine

#define PI 3.14159265358979323846264338327950288f

struct position
{
	float x, y, z, w;
};

struct velocity
{
	float vx, vy, vz, vw;
};

struct color
{
	float r, g, b, a;
};
union floatUintConversion
{
	float f;
	unsigned int u;
};

float randFloat()
{
	return ((float)(rand() % 10000)) / 10000.0f;
}

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
//bool vibrate = false;

void bucketPoints(unsigned int* buckets, unsigned int* hash, unsigned int pointCount, unsigned int hashDim, uint8_t* indArray)
{
	//for (unsigned int i = 0; i < pointCount; i++)
	//{
		//buckets[125 * hash[i] + (indArray[hash[i]] % 125)] = i + 1;
		//indArray[hash[i]]++;
	//}
	//memset(indArray, 0, (WIDTH / 10) * (WIDTH / 10) * (WIDTH / 10));

}

int main(void)
{
	//uint8_t* indexArray = new uint8_t[(WIDTH / 10) * (WIDTH / 10) * (WIDTH / 10)];
	//memset(indexArray, 0, (WIDTH / 10) * (WIDTH / 10) * (WIDTH / 10));
	GLFWwindow* window;
	srand(time(NULL));
	std::cout << "THIS FAR1\n";
	/* Initialize the library */
	if (!glfwInit())
	{
		std::cout << "glfwInit() failed";
		return -1;
	}
		
	std::cout << "THIS FAR2\n";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1920, 1080, "OpenGL", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	std::cout << "THIS FAR3\n";
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);//0 no fps cap

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glewInit() != GLEW_OK)
	{
		std::cout << "GLEW NOT OK\n";
	}

	uint8_t* bucketIndex = new uint8_t[(WIDTH / 10) * (WIDTH / 10) * (WIDTH / 10)];
	for (int i = 0; i < (WIDTH / 10) * (WIDTH / 10) * (WIDTH / 10); i++)
	{
		bucketIndex[i] = 0;
	}


	unsigned int positionSSbo;
	unsigned int positionOldSSbo;
	unsigned int velocitySSbo;
	unsigned int colorSSbo;

	unsigned int bucketSSbo;

	float floorOffset = 0;
	float floorMaxOffset = 7;
	float floorFrequency = 60; //hzf
	float floorAcceleration = 0;
	float floorSpeed = 0;
	float deltaTime = 1 / 215.0f;
	float time = 0;
	float length = WIDTH;
	float width = length;
	float height = length;
	float speedMax = 35.6471663187;

	unsigned int oddEven = 0;


	int bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	GLCall(glGenBuffers(1, &bucketSSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, bucketSSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, 200 * ((unsigned int)length / 10) * ((unsigned int)length / 10) * ((unsigned int)length / 10) * sizeof(unsigned int), 0, GL_DYNAMIC_READ));

	//GLCall(unsigned int* buckets = (unsigned int*)glMapNamedBuffer(bucketSSbo,  bufMask | GL_MAP_PERSISTENT_BIT));
	GLCall( glBindBuffer(GL_SHADER_STORAGE_BUFFER, bucketSSbo));
	GLCall(unsigned int* buckets = (unsigned int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE));
	//GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));

	GLCall(glGenBuffers(1, &positionOldSSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionOldSSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct position), NULL, GL_STATIC_READ));
	
	//GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
	
		GLCall(glGenBuffers(1, &positionSSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionSSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct position), NULL, GL_DYNAMIC_READ));


	GLCall(GL_SHADER_STORAGE_BUFFER,glBindBuffer(positionSSbo));
	GLCall(struct position* points = (struct position*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE));

	points[0].x = 0.0f;
	points[0].y = 0.0f;
	points[0].z = 0.0f;
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		unsigned int hashDim = ((unsigned int)width) / 10;

		//points[i].x = (3 * i % (WIDTH / 3 * WIDTH / 3)) % WIDTH;

		if(i < 2000)
		{
		points[i].x = 1.0f + randFloat() * width / 8.6f;
		points[i].y = 10.0f + randFloat() * height / 1.0;

		points[i].z = 1.0f + randFloat() * length / 8.6f;

		points[i].w = 1;
		}
		else
		{
			points[i].x = 2.0f + randFloat() * width / 1.2f;
			points[i].y = 01.0f + randFloat() * height / 1.0;

			points[i].z = 4.0f + randFloat() * length / 1.1f;

			points[i].w = 1;
		}
		/*unsigned int boxHash = ((unsigned int)points[i].x / 10) * hashDim * hashDim + ((unsigned int)points[i].y / 10) * hashDim + ((unsigned int)points[i].z / 10);
		std::cout << "Hash: " << boxHash << " X Coordinate: " << points[i].x << " X Box: " << boxHash / (hashDim * hashDim) << '\n';
		std::cout << "Hash: " << boxHash << " Y Coordinate: " << points[i].y << " Y Box: " << (boxHash % (hashDim * hashDim)) / hashDim << '\n';
		std::cout << "Hash: " << boxHash << " Z Coordinate: " << points[i].z << " Y Box: " << (boxHash % (hashDim)) << '\n';
		std::cout << "\n\n";*/
	}




	//GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));




	GLCall(glGenBuffers(1, &velocitySSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, velocitySSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct velocity), NULL, GL_STATIC_DRAW));

	GLCall(struct velocity* velocities = (struct velocity*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct velocity), bufMask));
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		if (i < 1000)
		{
			velocities[i].vx = 12.0f * (randFloat() - 0.5f) * speedMax + 12.0 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vy = 12.0f * (randFloat() - 0.5f) * speedMax + 12.0 * (randFloat() - 0.5f) * speedMax;;// (randFloat() - 0.5f)* speedMax * 1.0f;
			velocities[i].vz = 12.0f * (randFloat() - 0.5f) * speedMax + 12.0 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vw = 1.0f;
		}
		else
		{
			velocities[i].vx = 0.5f * (randFloat() - 0.5f) * speedMax + 0.5 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vy = 0.5f * (randFloat() - 0.5f) * speedMax + 0.5 * (randFloat() - 0.5f) * speedMax;;// (randFloat() - 0.5f)* speedMax * 1.0f;
			velocities[i].vz = 0.5f * (randFloat() - 0.5f) * speedMax + 0.5 * (randFloat() - 0.5f) * speedMax;
			velocities[i].vw = 1.0f;
		}
	}
	//GLCall(glUnmapNamedBuffer(velocitySSbo));
	GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));

	GLCall(glGenBuffers(1, &colorSSbo));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorSSbo));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct color), NULL, GL_STATIC_DRAW));

	GLCall(struct color* colors = (struct color*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct color), bufMask));
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		unsigned int cubeSide = ((unsigned int)width) / 10;
		colors[i].r = 0.2 + 0.6 * randFloat();//; randFloat();
		colors[i].g = 0.0;// +0.8 * randFloat();
		colors[i].b = 0.2 + 0.8 * randFloat();
		colors[i].a = 1;


	}
	//GLCall(glUnmapNamedBuffer(colorSSbo));
GLCall(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));








	/*ComputeShader bucketCollision("res/shaders/BucketCollision.shader");
	bucketCollision.SSBOPOS.push_back(6);
	bucketCollision.SSBOID.push_back(colorSSbo);
	bucketCollision.SSBOPOS.push_back(4);
	bucketCollision.SSBOID.push_back(positionSSbo);
	bucketCollision.SSBOPOS.push_back(5);
	bucketCollision.SSBOID.push_back(velocitySSbo);
	bucketCollision.SSBOPOS.push_back(7);
	bucketCollision.SSBOID.push_back(bucketSSbo);*/
	ComputeShader VelocityGravityWallEuler("res/shaders/VelocityGravityWallEuler.shader");
	VelocityGravityWallEuler.SSBOID.push_back(positionSSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(4);
	VelocityGravityWallEuler.SSBOID.push_back(velocitySSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(5);
	VelocityGravityWallEuler.SSBOID.push_back(colorSSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(6);
	VelocityGravityWallEuler.SSBOID.push_back(positionOldSSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(7);
	VelocityGravityWallEuler.SSBOID.push_back(bucketSSbo);
	VelocityGravityWallEuler.SSBOPOS.push_back(8);

	ComputeShader BucketResolver("res/shaders/BucketResolver.shader");
	BucketResolver.SSBOID.push_back(positionSSbo);
	BucketResolver.SSBOPOS.push_back(4);
	BucketResolver.SSBOID.push_back(velocitySSbo);
	BucketResolver.SSBOPOS.push_back(5);
	BucketResolver.SSBOID.push_back(positionOldSSbo);
	BucketResolver.SSBOPOS.push_back(7);
	BucketResolver.SSBOID.push_back(bucketSSbo);
	BucketResolver.SSBOPOS.push_back(8);

	Shader shader("res/shaders/Basic.shader");




	GLCall(std::cout << glGetString(GL_VERSION) << "\n");
	GLCall(glEnable(GL_DEPTH_TEST));
	{

#define CIRCLE_POINTS 32
		float circlePoints[CIRCLE_POINTS * 6 + 6];
		circlePoints[0] = 0.0f;
		circlePoints[0 + 1] = 0.0f;
		circlePoints[0 + 2] = 0.0f;
		circlePoints[0 + 3] = 0.0f;
		circlePoints[0 + 4] = 0.0f;
		circlePoints[0 + 5] = 1.0f;
		for (int i = 0; i < CIRCLE_POINTS; i++)
		{
			circlePoints[(i + 1) * 6] = 0;
			circlePoints[(i + 1) * 6 + 1] = 0;
			circlePoints[(i + 1) * 6 + 2] = 0;
			circlePoints[(i + 1) * 6 + 3] = 0;
			circlePoints[(i + 1) * 6 + 4] = 0;
			circlePoints[(i + 1) * 6 + 5] = 0;
		}
		for (int i = 0; i < CIRCLE_POINTS; i++)
		{
			circlePoints[(i + 1) * 6] = glm::cos(i * 2 * PI / CIRCLE_POINTS);
			circlePoints[(i + 1) * 6 + 1] = glm::sin(i * 2 * PI / CIRCLE_POINTS);
			circlePoints[(i + 1) * 6 + 2] = 0.0f;
			circlePoints[(i + 1) * 6 + 3] = (rand() % 100) / (float)100;
			circlePoints[(i + 1) * 6 + 4] = (rand() % 100) / (float)100;
			circlePoints[(i + 1) * 6 + 5] = (rand() % 100) / (float)100;
		}
		std::cout << circlePoints[6 * 1] << "\n";
		std::cout << circlePoints[6 * 1 + 1] << "\n";
		std::cout << circlePoints[6 * 1 + 2] << "\n";
		std::cout << circlePoints[6 * 2] << "\n";
		std::cout << circlePoints[6 * 2 + 1] << "\n";
		std::cout << circlePoints[6 * 2 + 2] << "\n";
		std::cout << circlePoints[6 * CIRCLE_POINTS] << "\n";
		std::cout << circlePoints[6 * CIRCLE_POINTS + 1] << "\n";
		std::cout << circlePoints[6 * CIRCLE_POINTS + 2] << "\n";
		unsigned int circleIndices[3 * CIRCLE_POINTS];
		for (int i = 0; i < CIRCLE_POINTS; i++)
		{
			circleIndices[i * 3] = 0;
			circleIndices[i * 3 + 1] = 0;
			circleIndices[i * 3 + 2] = 0;
		}
		for (int i = 0; i < CIRCLE_POINTS; i++)
		{
			circleIndices[i * 3] = 0;
			circleIndices[i * 3 + 1] = 1 + i;
			circleIndices[i * 3 + 2] = 2 + i;
			if (i == CIRCLE_POINTS - 1)
				circleIndices[i * 3 + 2] = 1;
		}
		IndexBuffer circleIb(circleIndices, 3 * (CIRCLE_POINTS));


		VertexArray circleVa;
		VertexBuffer circleVb(circlePoints, (CIRCLE_POINTS * 6 + 6) * sizeof(float));

		VertexBufferLayout circleLayout;
		circleLayout.Push<float>(3);
		circleLayout.Push<float>(3);
		circleVa.AddBuffer(circleVb, circleLayout);

		float positions[8 * 6] = {
			-0.5f,-0.5f,-0.5f, 1.0,0.0,0.0,
			 -0.5f,-0.5f,0.5f, 1.0,1.0,0.0,
			 -0.5f,0.5f,-0.5f, 1.0,0.0,1.0,
			 -0.5f,0.5f,0.5f, 0.0,0.0,1.0,
			0.5f,-0.5f,-0.5f, 0.0,1.0,1.0,
			0.5f,-0.5f,0.5f, 0.0,1.0,0.0,
			0.5f,0.5f,-0.5f, 0.0,0.0,1.0,
			0.5f,0.5f,0.5f, 0.0,1.0,0.0

		};
		unsigned int indices[36]
		{
			2,3,7,
			2,6,7,

			3,1,5,
			3,7,5,

			1,5,4,
			1,4,0,

			6,4,0,
			6,2,0,

			5,7,4,
			6,7,4,

			1,2,3,
			0,2,1

		};
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		IndexBuffer ib(indices, 36);


		VertexArray va;
		VertexBuffer vb(positions, 8 * 6 * sizeof(float));

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		va.AddBuffer(vb, layout);


		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		glm::mat4 model2 = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
		glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
		glm::mat4 projection = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-85.0f), glm::vec3(-10.0f, 0.0f, 0.0f));
		model2 = glm::rotate(model2, glm::radians(-85.0f), glm::vec3(1.0f, 0.6f, 0.2f));
		model2 = glm::translate(model2, glm::vec3(1.0f, 1.0f, 1.0f));
		model = glm::translate(model, glm::vec3(-0.8f, -0.2f, 0.3f));
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		cameraPos = cameraPos + glm::vec3(1.0f, 0.0f, 0.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)1920 / (float)1080, 0.1f, 100000000.0f);



		/*unsigned long lastTime = 0;
		{
			using namespace std::chrono;
			lastTime = duration_cast<milliseconds>(
				system_clock::now().time_since_epoch()).count();
		}*/


		Renderer renderer;

		ImGui::CreateContext();
		ImGui_ImplGlfwGL3_Init(window, true);
		ImGui::StyleColorsDark();


		glm::mat4 mvp;

		/* Loop until the user closes the window */
		long long int lastTime = glfwGetTime();
		while (!glfwWindowShouldClose(window))
		{
			//deltaTime = 1.0f / ImGui::GetIO().Framerate;
			ImGui_ImplGlfwGL3_NewFrame();

			/* Render here */
			renderer.Clear();



			{

				deltaTime = 0.07f / ImGui::GetIO().Framerate ;
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}
			
			GLCall(glUseProgram(VelocityGravityWallEuler.m_RendererID));
			VelocityGravityWallEuler.SetUniform1f("deltaTime", deltaTime);
			VelocityGravityWallEuler.SetUniform1f("sideLength", width);
			VelocityGravityWallEuler.Compute(NUM_PARTICLES / 128, 1, 1);
			GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
			//2097152
			GLCall(glFinish());

			GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, positionSSbo, 4));
			GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bucketSSbo, 8));
			unsigned int hashDim = ((unsigned int)width) / 10;
			for (int i = 0; i < NUM_PARTICLES; i++)
			{
				unsigned int hash = hashDim * hashDim * ((unsigned int)(points[i].x) / hashDim) + hashDim * ((unsigned int)(points[i].y) / hashDim) + ((unsigned int)(points[i].z) / hashDim);
				if (hash >= hashDim * hashDim * hashDim)
				{
					continue;
				}
				buckets[hash * 200 + bucketIndex[hash]] = i + 1;
				//buckets[hash * 200 + bucketIndex[hash] + 1] = 0;
				bucketIndex[hash]++;
				//std::cout << (unsigned int)bucketIndex[14] << "  IND \n";
			}
			memset(bucketIndex,0,hashDim*hashDim*hashDim);
			GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));

			GLCall(glUseProgram(BucketResolver.m_RendererID));
			BucketResolver.SetUniform1f("deltaTime", deltaTime);
			BucketResolver.SetUniform1f("sideLength", width);
			BucketResolver.Compute(((WIDTH / 10)* (WIDTH / 10)* (WIDTH / 10)) / 100, 1, 1);
			GLCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
			
			GLCall(glFinish());
			//GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, bucketSSbo));
			//GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, hashSSbo));
			//bucketPoints(buckets, hashes, NUM_PARTICLES, WIDTH / 10, indexArray);
			time += deltaTime;
			//void bucketPoints(unsigned int* buckets,position* points,unsigned int pointCount,unsigned int hashDim,uint8_t* indArray)


			GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, positionSSbo));
			GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, colorSSbo));
			GLCall(glUseProgram(shader.m_RendererID));
			mvp = projection * view * model;
			//shader.SetUniformMat4f("u_M", model);
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

			float cameraSpeed = 700.0f; // adjust accordingly
		//	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
			//	vibrate = ! vibrate;
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				cameraPos += cameraSpeed * cameraFront / ImGui::GetIO().Framerate;;
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				cameraPos -= cameraSpeed * cameraFront / ImGui::GetIO().Framerate;;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)) / ImGui::GetIO().Framerate;;
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp)) / ImGui::GetIO().Framerate;;
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
			//	glfwSetWindowTitle(window, (std::to_string(((double)1000.0 / (double)(currentTime - lastTime)))).c_str());
			lastTime = glfwGetTime();
		}
	}
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}
