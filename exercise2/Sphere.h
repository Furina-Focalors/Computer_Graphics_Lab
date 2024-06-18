#include <cmath>
#include <vector>
#include <glm\glm.hpp>
class Sphere
{
private:
	int numVertices;// 顶点数
	int numIndices;// 索引数
	std::vector<int> indices;// 索引 用于重用顶点，由多个图元共享同一个顶点
	std::vector<glm::vec3> vertices;// 顶点坐标
	std::vector<glm::vec2> texCoords;// 纹理坐标
	std::vector<glm::vec3> normals;// 法线向量
	std::vector<glm::vec3> tangents;// 切线向量
	void init(int);
	float toRadians(float degrees);

public:
	//Sphere();
	Sphere(int prec=48);
	int getNumVertices();
	int getNumIndices();
	std::vector<int> getIndices();
	std::vector<glm::vec3> getVertices();
	std::vector<glm::vec2> getTexCoords();
	std::vector<glm::vec3> getNormals();
	std::vector<glm::vec3> getTangents();
};