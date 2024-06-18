#pragma once
#include <vector>
#include <glm\glm.hpp>
#include <cmath>

#define PI 3.14159265359

class Cylinder {
private:
	int numVertices;// ������
	int numIndices;// ������
	std::vector<int> indices;// ���� �������ö��㣬�ɶ��ͼԪ����ͬһ������
	std::vector<glm::vec3> vertices;// ��������
	std::vector<glm::vec2> texCoords;// ��������
	std::vector<glm::vec3> normals;// ��������
	std::vector<glm::vec3> tangents;// ��������
	void init(int);
public:
	Cylinder(int prec=48);
	int getNumVertices();
	int getNumIndices();
	std::vector<int> getIndices();
	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
	std::vector<glm::vec3> getNormals();
	std::vector<glm::vec3> getTangents();
};