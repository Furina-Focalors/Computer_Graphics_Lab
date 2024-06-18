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

#define numVAOs 3
#define numVBOs 8

//float cameraX, cameraY, cameraZ;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);// �������
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);// �������
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);// ����Ϸ���

float yaw = -90.0f;// ƫ���� ��y����ת
float pitch = 0.0f;// ������ ��x����ת
float lastX, lastY;// ָ���ϴ�ͣ����λ��
bool firstMouse = true;// ��ʼ��

GLuint renderingProgram, lightShaderProgram, skyboxShaderProgram, sphRenderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
GLuint sphereTexture;
GLuint cylinderTexture;
GLuint skyboxTexture;
float rotAmt = 0.0f;

float fovy = 1.0472f;// �ӳ���ֱ�Ƕȣ�Խ�����ʾ�ķ�ΧԽ��
float near = 0.1f;// �����������Ŀɼ�����룬��������Ⱦ
float far = 1000.0f;// ���������Զ�Ŀɼ�����룬��Զ����Ⱦ

glm::vec3 lightLoc = glm::vec3(0.0f, 5.0f, 2.0f);// ��Դλ��

// variable allocation for display
GLuint mvLoc, projLoc, viewLoc, nLoc, lLoc;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;// ���ա�����
int width, height;
float aspect;// ���߱�
glm::mat4 pMat, vMat, mMat, mvMat, view, invTrMat;
glm::vec3 currentLightPos, transformed;// ��Դλ��
glm::vec3 lightShift = glm::vec3(0.0f, 0.0f, 0.0f);// ��Դλ�ý���ƽ��
float lightPos[3];

// white light
float globalAmbient[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
float lightAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// gold material
float* matAmb = Utils::silverAmbient();
float* matDif = Utils::silverDiffuse();
float* matSpe = Utils::silverSpecular();
float matShi = Utils::silverShininess();

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
void installLights(glm::mat4 vMatrix, GLint rp);

void installLights(glm::mat4 vMatrix, GLint rp) {
	transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;

	// get the locations of the light and material fields in the shader
	globalAmbLoc = glGetUniformLocation(rp, "globalAmbient");
	ambLoc = glGetUniformLocation(rp, "light.ambient");
	diffLoc = glGetUniformLocation(rp, "light.diffuse");
	specLoc = glGetUniformLocation(rp, "light.specular");
	posLoc = glGetUniformLocation(rp, "light.position");
	mambLoc = glGetUniformLocation(rp, "material.ambient");
	mdiffLoc = glGetUniformLocation(rp, "material.diffuse");
	mspecLoc = glGetUniformLocation(rp, "material.specular");
	mshiLoc = glGetUniformLocation(rp, "material.shininess");

	//  set the uniform light and material values in the shader
	glProgramUniform4fv(rp, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(rp, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(rp, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(rp, specLoc, 1, lightSpecular);
	glProgramUniform3fv(rp, posLoc, 1, lightPos);
	glProgramUniform4fv(rp, mambLoc, 1, matAmb);
	glProgramUniform4fv(rp, mdiffLoc, 1, matDif);
	glProgramUniform4fv(rp, mspecLoc, 1, matSpe);
	glProgramUniform1f(rp, mshiLoc, matShi);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 0.05f;
	float lightSensitivity = 0.05f;
	// �������
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
	// ��Դ����
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		lightShift += lightSensitivity * glm::vec3(0.0f, 1.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		lightShift -= lightSensitivity * glm::vec3(0.0f, 1.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		lightShift -= lightSensitivity * glm::vec3(1.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		lightShift += lightSensitivity * glm::vec3(1.0f, 0.0f, 0.0f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
		lightShift -= lightSensitivity * glm::vec3(0.0f, 0.0f, 1.0f);
	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		lightShift += lightSensitivity * glm::vec3(0.0f, 0.0f, 1.0f);
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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
	std::vector<glm::vec3> tangents = mySphere.getTangents();

	std::vector<float> pvalues;
	std::vector<float> tvalues;
	std::vector<float> nvalues;
	std::vector<float> tgvalues;

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
		tgvalues.push_back(tangents[ind[i]].x);
		tgvalues.push_back(tangents[ind[i]].y);
		tgvalues.push_back(tangents[ind[i]].z);
	}

	glGenVertexArrays(numVAOs, vao);
	//glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);// �󶨲�����vao[0]
	glGenBuffers(numVBOs, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, tgvalues.size() * 4, &tgvalues[0], GL_STATIC_DRAW);

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

	//skybox
	float cubeVertexPositions[108] =// 12 triangles * 3 dims
	{ -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
	};
	glBindVertexArray(vao[2]);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);

}

void init(GLFWwindow* window) {
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");
	lightShaderProgram = Utils::createShaderProgram("lightVert.glsl", "lightFrag.glsl");
	skyboxShaderProgram = Utils::createShaderProgram("vertCShader.glsl", "fragCShader.glsl");
	sphRenderingProgram = Utils::createShaderProgram("vertShaderSph.glsl", "fragShaderSph.glsl");
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
	//glEnable(GL_LIGHT0);

	setupVertices();
	sphereTexture = Utils::loadTexture("download.jpg");
	cylinderTexture = Utils::loadTexture("alum.jpg");
	skyboxTexture = Utils::loadCubeMap("skybox");
}

void display(GLFWwindow* window, double currentTime)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// �۲����ʹ���λ��cameraPos������Pos+Front
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);


	// draw cube map
	glUseProgram(skyboxShaderProgram);

	viewLoc = glGetUniformLocation(skyboxShaderProgram, "view_matrix");
	projLoc = glGetUniformLocation(skyboxShaderProgram, "proj_matrix");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	// ������Ȳ���
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);	
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_DEPTH_TEST);














	// ��Դ
	glUseProgram(lightShaderProgram);

	mvLoc = glGetUniformLocation(lightShaderProgram, "mv_matrix");
	projLoc = glGetUniformLocation(lightShaderProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(lightShaderProgram, "view_matrix");

	glm::mat4 lightTrans = glm::translate(glm::mat4(1.0f), glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
	glm::mat4 lightScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(lightTrans * lightScale));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	
	// �������
	currentLightPos = glm::vec3(lightLoc.x, lightLoc.y, lightLoc.z);
	glm::mat4 lightTransl = glm::translate(glm::mat4(1.0f), lightShift);
	//currentLightPos = glm::vec3(rMat * glm::vec4(currentLightPos, 1.0f));

	// ����1
	matAmb = Utils::silverAmbient();
	matDif = Utils::silverDiffuse();
	matSpe = Utils::silverSpecular();
	matShi = Utils::silverShininess();
	installLights(lightTransl, renderingProgram);
	installLights(lightTransl, sphRenderingProgram);

	glUseProgram(sphRenderingProgram);

	mvLoc = glGetUniformLocation(sphRenderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(sphRenderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(sphRenderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(sphRenderingProgram, "norm_matrix");
	// view���ݸ���ɫ��
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// ���壨��
	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -1.0));
	mMat = glm::rotate(glm::mat4(1.0f), (float)currentTime * 50.0f, glm::vec3(0.0, 1.0, 0.0)); //������ʱ����ת
	glm::mat4 sMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));//����
	glm::mat4 upwards = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.5f, 0.0));//����0.5
	//glm::mat4 mMat2 = glm::rotate(glm::mat4(1.0f), (float)currentTime * 50.0f, glm::vec3(0.0, -1.0, 0.0));//��ת
	//mvMat = vMat2 * mMat * vMat * mMat2 * sMat;
	mvMat = upwards * vMat * sMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);// 0 = vertShader�� in ���±꣬3 = �������ԵĴ�С
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	//glBindVertexArray(vao[0]); // Ϊʲô���ܶ�ΰ󶨣�����
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	// ˮ��
	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(renderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	// view���ݸ���ɫ��
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 waterScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 2.0f));//����
	glm::mat4 waterVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mvMat = upwards * vMat * waterVMat * waterScaleMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// �رձ����޳���������Ա��棨���������ڣ���������
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());

	// ����������
	//glm::mat4 mMat3 = glm::rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, 1.0, 0.0));//��ת
	mMat = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));// ��x����ת90��
	glm::mat4 bodyScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.25f, 0.6f, 1.25f));//����
	glm::mat4 bodyVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0f));
	mvMat = bodyVMat * vMat * bodyScaleMat * mMat;
	//mvMat = vMat2 * vMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// �رձ����޳���������Ա��棨���������ڣ���������
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());









	// ����2
	matAmb = Utils::goldAmbient();
	matDif = Utils::goldDiffuse();
	matSpe = Utils::goldSpecular();
	matShi = Utils::goldShininess();
	installLights(lightTransl, renderingProgram);
	installLights(lightTransl, sphRenderingProgram);

	// ���壨��
	glUseProgram(sphRenderingProgram);

	mvLoc = glGetUniformLocation(sphRenderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(sphRenderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(sphRenderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(sphRenderingProgram, "norm_matrix");

	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 5.0));
	sMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));//����
	upwards = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.5f, 0.0));//����0.5
	mvMat = upwards * vMat * sMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);// 0 = vertShader�� in ���±꣬3 = �������ԵĴ�С
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphereTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	//glBindVertexArray(vao[0]); // Ϊʲô���ܶ�ΰ󶨣�����
	glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

	// ˮ��
	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	viewLoc = glGetUniformLocation(renderingProgram, "view_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	waterScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 2.0f));//����
	waterVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	mvMat = upwards * vMat * waterVMat * waterScaleMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// �رձ����޳���������Ա��棨���������ڣ���������
	glFrontFace(GL_CCW);

	glDrawArrays(GL_TRIANGLES, 0, myCylinder.getNumIndices());

	// ����������
	mMat = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));// ��x����ת90��
	bodyScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(1.25f, 0.6f, 1.25f));//����
	bodyVMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.25f, 0.0f));
	mvMat = bodyVMat * vMat * bodyScaleMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cylinderTexture);

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);// �رձ����޳���������Ա��棨���������ڣ���������
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