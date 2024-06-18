#include"Cylinder.h"

using namespace std;

int Cylinder::getNumVertices() { return numVertices; }
int Cylinder::getNumIndices() { return numIndices; }
std::vector<int> Cylinder::getIndices() { return indices; }
std::vector<glm::vec3> Cylinder::getVertices() { return vertices; }
std::vector<glm::vec2> Cylinder::getTexCoords() { return texCoords; }
std::vector<glm::vec3> Cylinder::getNormals() { return normals; }
std::vector<glm::vec3> Cylinder::getTangents() { return tangents; }

Cylinder::Cylinder(int prec) {
	init(prec);
}

void Cylinder::init(int prec) {
	// initialize
	numVertices = 2 * (prec + 1);
	numIndices = 12 * prec - 12;
	for (int i = 0; i < numVertices; i++) { vertices.push_back(glm::vec3()); }
	for (int i = 0; i < numVertices; i++) { texCoords.push_back(glm::vec2()); }
	for (int i = 0; i < numVertices; i++) { normals.push_back(glm::vec3()); }
	for (int i = 0; i < numVertices; i++) { tangents.push_back(glm::vec3()); }
	for (int i = 0; i < numIndices; i++) { indices.push_back(0); }

	// calculate triangle vertices
	// 上底面
	for (int i = 0; i <= prec; ++i) {
		float x = (float)cos(2 * PI * i / prec);
		float y = (float)sin(2 * PI * i / prec);
		float z = 0.5;
		vertices[i] = glm::vec3(x, y, z);
		texCoords[i] = glm::vec2((float)i / prec, 0.0f);
		normals[i] = glm::vec3(x, y, 0.0f);
		tangents[i] = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(x, y, z));
	}
	// 下底面
	for (int i = prec + 1; i < numVertices; ++i) {
		float x = (float)cos(2 * PI * (i - prec - 1) / prec);
		float y = (float)sin(2 * PI * (i - prec - 1) / prec);
		float z = -0.5;
		vertices[i] = glm::vec3(x, y, z);
		texCoords[i] = glm::vec2((float)(i - prec - 1) / prec, 1.0f);
		normals[i] = glm::vec3(x, y, 0.0f);
		tangents[i] = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(x, y, z));
	}
	
	// calculate triangle indices
	for (int i = 0; i < prec; i++) {
		indices[6 * i] = i;
		indices[6 * i + 1] = i + prec + 1;
		indices[6 * i + 2] = i + prec + 2;
		indices[6 * i + 3] = i;
		indices[6 * i + 4] = i + 1;
		indices[6 * i + 5] = i + prec + 2;
	}

	// 计算上底面和下底面 这是一大坨
	for (int i = 6 * prec; i <= 9 * prec - 9; i += 3) {
		indices[i] = 0;
		indices[i + 1] = i / 3 - 2 * prec + 1;
		indices[i + 2] = i / 3 - 2 * prec + 2;
	}
	for (int i = 9 * prec - 6; i <= 12 * prec - 15; i += 3) {
		indices[i] = prec + 1;
		indices[i + 1] = i / 3 - 2 * prec + 4;
		indices[i + 2] = i / 3 - 2 * prec + 5;
	}
}