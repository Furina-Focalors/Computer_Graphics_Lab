#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <SOIL2\soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm\glm.hpp>
#include <glm\gtc\type_ptr.hpp> // glm::value_ptr
#include <glm\gtc\matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Sphere.h"
#include "Utils.h"
#include "Cylinder.h"
using namespace std;

#define numVAOs 2
#define numVBOs 6

//float cameraX, cameraY, cameraZ;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);// �������
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);// �������
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);// ����Ϸ���

float yaw = -90.0f;// ƫ���� ��y����ת
float pitch = 0.0f;// ������ ��x����ת
float lastX, lastY;// ָ���ϴ�ͣ����λ��
bool firstMouse = true;// ��ʼ��

GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
GLuint earthTexture;
float rotAmt = 0.0f;

float fovy = 1.0472f;// �ӳ���ֱ�Ƕȣ�Խ�����ʾ�ķ�ΧԽ��
float near = 0.1f;// �����������Ŀɼ�����룬��������Ⱦ
float far = 1000.0f;// ���������Զ�Ŀɼ�����룬��Զ����Ⱦ

// variable allocation for display
GLuint mvLoc, projLoc, viewLoc;
int width, height;
float aspect;// ��߱�
glm::mat4 pMat, vMat, mMat, mvMat, view;

int prec = 48;
Sphere mySphere = Sphere(prec);
Cylinder myCylinder = Cylinder(prec);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void setupVertices(void);
void init(GLFWwindow* window);
void display(GLFWwindow* window, double currentTime);
void window_size_callback(GLFWwindow* win, int newWidth, int newHeight);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 0.05f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraUp;
}

/*
 * @brief ָ��λ�ûص�����
 * 
*/
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw = yaw + xoffset;
	pitch = pitch + yoffset;

	// ��ֹ pitch ������Χ��������������
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	// ������ͼ����
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}


/**
 * �����ص�����
*/
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_LEFT_ALT:
			glfwSetCursorPosCallback(window, nullptr);// ȡ���ӽ���ת
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);// �ָ����ָ��
			glfwSetKeyCallback(window, nullptr);// ȡ���ӽ�ƽ��
			glfwSetScrollCallback(window, nullptr);// ȡ��������Ұ�任
			break;
		default:
			break;
		}
	}
}

/**
 * @brief ��갴���ص�����
 * 
*/
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		firstMouse = true;
		glfwSetCursorPosCallback(window, mouse_callback);// �������ָ��ص�����
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);// �������ָ��
		glfwSetKeyCallback(window, key_callback);// �������̻ص�����
		glfwSetScrollCallback(window, scroll_callback);// ����������Ұ�任
	}
}

/**
 * @brief ���ֻص�����
 * @param yoffset		��ֱ��������Ĵ�С
*/
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	const float sensitivity = 0.1f;
	// �����Ӿ�
	if (fovy >= 1.0f && fovy <= 2.0f)
		fovy -= sensitivity * yoffset;
	if (fovy <= 1.0f)
		fovy = 1.0f;
	if (fovy >= 2.0f)
		fovy = 2.0f;
	pMat = glm::perspective(fovy, aspect, near, far);
}

void setupVertices(void) {
	// Sphere
	std::vector<int> ind = mySphere.getIndices();
	std::vector<glm::vec3> vert = mySphere.getVertices();
	std::vector<glm::vec2> tex = mySphere.getTexCoords();
	std::vector<glm::vec3> norm = mySphere.getNormals();

	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;

	int numIndices = mySphere.getNumIndices();
	for (int i = 0; i < numIndices; i++) {
		pvalues.push_back((vert[ind[i]]).x);
		pvalues.push_back((vert[ind[i]]).y);
		pvalues.push_back((vert[ind[i]]).z);
		tvalues.push_back((tex[ind[i]]).s);
		tvalues.push_back((tex[ind[i]]).t);
		nvalues.push_back((norm[ind[i]]).x);
		nvalues.push_back((norm[ind[i]]).y);
		nvalues.push_back((norm[ind[i]]).z);
	}

	glGenVertexArrays(numVAOs, vao);
	//glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);// �󶨲�����vao[0]
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size()*4, &pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size()*4, &tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size()*4, &nvalues[0], GL_STATIC_DRAW);

	//Cylinder
	ind = myCylinder.getIndices();
	vert = myCylinder.getVertices();
	tex = myCylinder.getTexCoords();
	norm = myCylinder.getNormals();

	std::vector<float> cy_pvalues;
	std::vector<float> cy_tvalues;
	std::vector<float> cy_nvalues;

	numIndices = myCylinder.getNumIndices();
	for (int i = 0; i < numIndices; i++) {
		cy_pvalues.push_back((vert[ind[i]]).x);
		cy_pvalues.push_back((vert[ind[i]]).y);
		cy_pvalues.push_back((vert[ind[i]]).z);
		cy_tvalues.push_back((tex[ind[i]]).s);
		cy_tvalues.push_back((tex[ind[i]]).t);
		cy_nvalues.push_back((norm[ind[i]]).x);
		cy_nvalues.push_back((norm[ind[i]]).y);
		cy_nvalues.push_back((norm[ind[i]]).z);
	}
	//std::cout << "aaa";
	//glGenVertexArrays(1, vao + 1);
	glBindVertexArray(vao[1]);// �󶨲�����vao[1]

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, cy_pvalues.size() * 4, &cy_pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, cy_tvalues.size() * 4, &cy_tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, cy_nvalues.size() * 4, &cy_nvalues[0], GL_STATIC_DRAW);
}

void init(GLFWwindow* window) {
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	//cameraX = 0.0f; cameraY = 0.0f; cameraZ = 3.0f;
	//sphLocX = 0.0f; sphLocY = 0.0f; sphLocZ = -1.0f;
	//cameraPos.x = 0.0f; cameraPos.y = 0.0f; cameraPos.z = 3.0f;
	
	// ��ʼ��view����
	/*glUseProgram(renderingProgram);
	viewLoc = glGetUniformLocation(renderingProgram, "view_matrix");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));*/

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(fovy, aspect, near, far);

	glEnable(GL_DEPTH_TEST); //������Ȳ���

	setupVertices();
	earthTexture = Utils::loadTexture("namecard.png");
}

void display(GLFWwindow* window, double currentTime) 
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(renderingProgram, "view_matrix");

	// �۲����ʹ���λ��cameraPos������Pos+Front
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	// ���ݸ���ɫ��
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// ͷ��
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -1.0)); //����������ĸ������ƶ�
	mMat = glm::rotate(glm::mat4(1.0f), (float)currentTime * 50.0f, glm::vec3(0.0, 1.0, 0.0)); //������ʱ����ת
	glm::mat4 sMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));//����
	glm::mat4 upwards = glm::translate(glm::mat4(1.0f), glm::vec3(0.0,0.5f,0.0));//����0.5
	//glm::mat4 mMat2 = glm::rotate(glm::mat4(1.0f), (float)currentTime * 50.0f, glm::vec3(0.0, -1.0, 0.0));//��ת
	//mvMat = vMat2 * mMat * vMat * mMat2 * sMat;
	mvMat = upwards * vMat * mMat * sMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	//glBindVertexArray(vao[0]); // Ϊʲô���ܶ�ΰ󶨣�����
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	// ˮ��
	glm::mat4 waterScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 2.0f));//����
	glm::mat4 waterVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mvMat = upwards * vMat * mMat * waterVMat * waterScaleMat;
	//mvMat = vMat2 * vMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTexture);

	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);// �رձ����޳���������Ա��棨���������ڣ���������
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());

	// ����
	//glm::mat4 mMat3 = glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 1.0, 0.0));//��ת
	mMat = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));// ��x����ת90��
	glm::mat4 bodyScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.25f, 0.6f, 1.25f));//����
	glm::mat4 bodyVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0f));
	mvMat = bodyVMat * vMat * bodyScaleMat * mMat;
	//mvMat = vMat2 * vMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTexture);

	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);// �رձ����޳���������Ա��棨���������ڣ���������
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());
}

void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) 
{
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	//pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);// 1.0472 rad=60 degrees
	pMat = glm::perspective(fovy, aspect, near, far);
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	//glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);// ����˫����

	GLFWwindow* window = glfwCreateWindow(600, 600, "YuanShen", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, window_size_callback);

	// ע��ص�����
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	init(window);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);// �����ӽ�ƽ��
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();

		/*GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cerr << "OpenGL Error: " << error << std::endl;
		}*/
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}