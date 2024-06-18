#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include "Utils.h"
using namespace std;

#define numVAOs 11
#define numVBOs 22
#define PI 3.141592653589793

//ȫ�ֱ���
GLuint renderingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];
int windowWidth = 800;
int windowHeight = 800;
const char* WINDOW_TITLE = "My first OpenGL program";
const int NUM_SEGMENTS = 127;

/*
	@brief ���� Բ
	@param cx				Բ�ĵ� ������
	@param cy				Բ�ĵ� ������
	@param r				Բ�� �뾶
	@param numSegments		�зֵ������θ���
	@return float*			��������
*/
float* genCircle(float cx, float cy, float r, int numSegments) {
	float* pos = new float[2*numSegments];// n����ĺ�������������
	for (int i = 0; i < numSegments; i++) {
		float theta = 2.0f * PI * float(i) / float(numSegments);
		float x = r * cosf(theta);
		float y = r * sinf(theta);
		pos[2 * i] = cx + x;
		pos[2 * i + 1] = cy + y;
	}
	
	float* res = new float[6 * numSegments];// n�������εĶ�������
	int index = 0;
	for (int i = 0; i < 6 * numSegments; i += 6) {
		// ��һ������
		res[i] = cx;
		res[i + 1] = cy;
		// �ڶ�������
		res[i + 2] = pos[index];
		res[i + 3] = pos[index+1];
		// ����������
		res[i + 4] = pos[(index+2) % (2 * numSegments)];
		res[i + 5] = pos[(index+3) % (2 * numSegments)];
		index += 2;
	}
	return res;
}

/*
	@brief ���� ����
	@param p_x			����� ������
	@param p_y			����� ������
	@param height		���ε� ��
	@param width		���ε� ��
	@param theta		��������̱��� x�������� �ļн�
*/
float* genRectangle(float p_x, float p_y, float height, float width, float theta) {
	float* res = new float[12];
	res[0] = p_x;
	res[1] = p_y;
	res[2] = p_x - height * sinf(theta);
	res[3] = p_y + height * cosf(theta);
	res[4] = p_x + width * cosf(theta);
	res[5] = p_y + width * sinf(theta);
	res[6] = p_x - height * sinf(theta);
	res[7] = p_y + height * cosf(theta);
	res[8] = p_x + width * cosf(theta);
	res[9] = p_y + width * sinf(theta);
	res[10] = res[2] + res[4] - p_x;
	res[11] = res[3] + res[5] - p_y;
	return res;
}

/* @brief ����ͷ����Ȧ */
void initHeadOuter() {
	float* vertices = genCircle(0, 0.5, 0.15, NUM_SEGMENTS);

	float* vertexColors = new float[12 * NUM_SEGMENTS];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 12 * NUM_SEGMENTS; i += 4) {
		vertexColors[i] = 246.0f/255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[0]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 6 * NUM_SEGMENTS * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 12 * NUM_SEGMENTS * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief ����ͷ����Ȧ */
void initHeadInner() {
	float* vertices = genCircle(0, 0.5, 0.1, NUM_SEGMENTS);

	float* vertexColors = new float[12 * NUM_SEGMENTS];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 12 * NUM_SEGMENTS; i += 4) {
		vertexColors[i] = 0.0f;// R
		vertexColors[i+1] = 0.0f;// G
		vertexColors[i+2] = 0.0f;// B
		vertexColors[i+3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[1]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, 6 * NUM_SEGMENTS * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 12 * NUM_SEGMENTS * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief �������� */
void initLeftHand() {
	float* vertices = genCircle(0.0125-0.3*cosf(PI/6), 0.25-0.025*cosf(PI/6), 0.025, NUM_SEGMENTS);

	float* vertexColors = new float[12 * NUM_SEGMENTS];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 12 * NUM_SEGMENTS; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[2]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, 6 * NUM_SEGMENTS * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, 12 * NUM_SEGMENTS * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief �������� */
void initRightHand() {
	float* vertices = genCircle(-0.0125 + 0.3 * cosf(PI / 6), 0.25 - 0.025 * cosf(PI / 6), 0.025, NUM_SEGMENTS);

	float* vertexColors = new float[12 * NUM_SEGMENTS];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 12 * NUM_SEGMENTS; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[3]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, 6 * NUM_SEGMENTS * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, 12 * NUM_SEGMENTS * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief ������� */
void initLeftFoot() {
	float* vertices = genCircle(0.025*cosf(PI/6)-0.25, 0.0475-0.5*cosf(PI/6), 0.025, NUM_SEGMENTS);

	float* vertexColors = new float[12 * NUM_SEGMENTS];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 12 * NUM_SEGMENTS; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[4]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, 6 * NUM_SEGMENTS * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, 12 * NUM_SEGMENTS * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief �����ҽ� */
void initRightFoot() {
	float* vertices = genCircle(-0.025 * cosf(PI / 6) + 0.25, 0.0475 - 0.5 * cosf(PI / 6), 0.025, NUM_SEGMENTS);

	float* vertexColors = new float[12 * NUM_SEGMENTS];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 12 * NUM_SEGMENTS; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[5]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
	glBufferData(GL_ARRAY_BUFFER, 6 * NUM_SEGMENTS * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
	glBufferData(GL_ARRAY_BUFFER, 12 * NUM_SEGMENTS * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief �������� */
void initBody() {
	float* vertices = genRectangle(-0.0425,-0.01,0.4,0.085,0);

	float* vertexColors = new float[24];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 24; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[6]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief ������� */
void initLeftArm() {
	float* vertices = genRectangle(0.025, 0.4-0.05*cosf(PI/6), 0.3, 0.05, 2 * PI / 3);

	float* vertexColors = new float[24];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 24; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[7]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[15]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief �����ұ� */
void initRightArm() {
	float* vertices = genRectangle(0, 0.4, 0.3, 0.05, -2 * PI / 3);

	float* vertexColors = new float[24];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 24; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[8]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[16]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[17]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief �������� */
void initLeftLeg() {
	float* vertices = genRectangle(0.05*cosf(PI/6), 0.035, 0.5, 0.05, 5 * PI / 6);

	float* vertexColors = new float[24];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 24; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[9]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[18]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[19]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

/* @brief �������� */
void initRightLeg() {
	float* vertices = genRectangle(0, 0.06, 0.5, 0.05, -5 * PI / 6);

	float* vertexColors = new float[24];
	// Ϊÿ������������ɫ
	for (int i = 0; i < 24; i += 4) {
		vertexColors[i] = 246.0f / 255.0f;// R
		vertexColors[i + 1] = 130.0f / 255.0f;// G
		vertexColors[i + 2] = 5.0f / 255.0f;// B
		vertexColors[i + 3] = 1.0f;// A
	}

	//�󶨵�ǰVAO
	glBindVertexArray(vao[10]);
	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[20]);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertices, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vPositionLoc = glGetAttribLocation(renderingProgram, "vPosition");
	glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPositionLoc);

	// Load the data into the GPU  
	glBindBuffer(GL_ARRAY_BUFFER, vbo[21]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertexColors, GL_STATIC_DRAW);
	//��VBO������������ɫ������Ӧ�Ķ�������
	GLuint vColorLoc = glGetAttribLocation(renderingProgram, "vColor");
	glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColorLoc);
}

//������ɫ������ʼ������
void init(GLFWwindow* window) 
{
	renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");

	// ����vao��vbo
	glGenVertexArrays(numVAOs, vao);// Vertex Array Object �����е�Ԫ�ش���������
	glGenBuffers(numVBOs, vbo);// Vertex Buffer Object ���������

	// ����ͼ��
	initHeadOuter();
	initHeadInner();
	initLeftHand();
	initRightHand();
	initLeftFoot();
	initRightFoot();
	initBody();
	initLeftArm();
	initRightArm();
	initLeftLeg();
	initRightLeg();
}
//��������
void display(GLFWwindow* window, double currentTime) 
{
	glUseProgram(renderingProgram); 

	glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//����ģ��
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * NUM_SEGMENTS);
	glBindVertexArray(vao[1]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * NUM_SEGMENTS);
	glBindVertexArray(vao[2]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * NUM_SEGMENTS);
	glBindVertexArray(vao[3]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * NUM_SEGMENTS);
	glBindVertexArray(vao[4]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * NUM_SEGMENTS);
	glBindVertexArray(vao[5]);
	glDrawArrays(GL_TRIANGLES, 0, 3 * NUM_SEGMENTS);
	glBindVertexArray(vao[6]);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(vao[7]);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(vao[8]);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(vao[9]);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(vao[10]);
	glDrawArrays(GL_TRIANGLES, 0, 12);
}

int main(void) 
{
	//glfw��ʼ��
	if (!glfwInit()) 
	{ 
		exit(EXIT_FAILURE); 
	}
	//���ڰ汾
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//��������
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, WINDOW_TITLE, NULL, NULL);
	// ���ô���window�Ĺ���OpenGL����Ϊ��ǰ������ֱ������ɾ������������������Ϊ��ǰ����
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)//glew��ʼ��
	{ 
		exit(EXIT_FAILURE);
	}
	glfwSwapInterval(1);// ����ˢ��ʱ��

	//������ɫ���������Ͱ�VAO��VBO
	init(window);

	//�¼�ѭ��
	while (!glfwWindowShouldClose(window)) // ����û�׼���رմ���window��᷵��GL_TRUE
	{
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}