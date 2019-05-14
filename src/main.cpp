#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <fstream>
#include <sstream>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Shader_Helper.h"

enum VAO_IDs {
	Triangles, NumVAOs
};
enum Buffer_IDs {
	ArrayBuffer, NumBuffers
};
enum Attrib_IDs {
	vPosition = 0
};
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 6;

void init() {
	using namespace gl;
	static const GLfloat vertices[NumVertices][2] =
			{
					{-0.90f, -0.90f},  // Triangle 1
					{0.85f,  -0.90f},
					{-0.90f, 0.85f},
					{0.90f,  -0.85f},  // Triangle 2
					{0.90f,  0.90f},
					{-0.85f, 0.90f}
			};
	glCreateVertexArrays(NumVAOs, VAOs);
	glCreateBuffers(NumBuffers, Buffers);
	glNamedBufferStorage(Buffers[ArrayBuffer], sizeof(vertices), vertices, BufferStorageMask::GL_NONE_BIT);

	auto vs = Shader_Helper::FromFile("shaders/triangles.vert", GL_VERTEX_SHADER);
	auto fs = Shader_Helper::FromFile("shaders/triangles.frag", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	glUseProgram(program);
	glBindVertexArray(VAOs[Triangles]);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[ArrayBuffer]);
	glVertexAttribPointer(vPosition, 2, static_cast<gl::GLenum>(GL_FLOAT), GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(vPosition);
}

void display() {
	static const float black[] = {0.0f, 0.0f, 0.0f, 0.0f};
	static const float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
	gl::glClearBufferfv(static_cast<gl::GLenum>(GL_COLOR), 0, white);
	gl::glBindVertexArray(VAOs[Triangles]);
	gl::glDrawArrays(static_cast<gl::GLenum>(GL_LINES), 0, NumVertices);

}


int main(int, char *[]) {
//	using namespace gl;
	using namespace std;
	GLFWwindow *window;
	if (!glfwInit()) {
		return -1;
	}

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_VISIBLE, true);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	window = glfwCreateWindow(640, 480, "Hello", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glbinding::initialize(glfwGetProcAddress);
	init();
	while (!glfwWindowShouldClose(window)) {
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
