#include "Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

Mesh::Mesh(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
	indexCount = indices.size();

	VAO.generate();

	VAO["VBO"] = BufferObject(GL_ARRAY_BUFFER);
	VAO["VBO"].generate();

	VAO.bind();

	VAO["VBO"].bind();
	VAO["VBO"].setData<float>(static_cast<GLuint>(vertices.size()), &vertices[0], GL_STATIC_DRAW);


	VAO["EBO"] = BufferObject(GL_ELEMENT_ARRAY_BUFFER);
	VAO["EBO"].generate();

	VAO["EBO"].bind();
	VAO["EBO"].setData<GLuint>(static_cast<GLuint>(indices.size()), &indices[0], GL_STATIC_DRAW);

	// Position

	VAO["VBO"].setAttPointer<float>(0, 3, GL_FLOAT, 8, 0);

	// Normal
	VAO["VBO"].setAttPointer<float>(1, 3, GL_FLOAT, 8, 3);

	//UV
	VAO["VBO"].setAttPointer<float>(2, 2, GL_FLOAT, 8, 6);
}

void Mesh::draw() {
	VAO.bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
	ArrayObject::clear();

}

Mesh* Mesh::CreateSphere(unsigned int X_SEGMENTS, unsigned int Y_SEGMENTS) {
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {

			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;

	
			float xPos = std::cos(xSegment * glm::two_pi<float>()) *
				std::sin(ySegment * glm::pi<float>());
			float yPos = std::cos(ySegment * glm::pi<float>());
			float zPos = std::sin(xSegment * glm::two_pi<float>()) *
				std::sin(ySegment * glm::pi<float>());

			// Position (3) + Normal (3) + UV (2)
			vertices.push_back(xPos);
			vertices.push_back(yPos);
			vertices.push_back(zPos);

			vertices.push_back(xPos);
			vertices.push_back(yPos);
			vertices.push_back(zPos);

			// U runs opposite to the longitude angle above, otherwise maps appear mirrored east-west
			vertices.push_back(1.0f - xSegment);
			vertices.push_back(1.0f - ySegment);
		}
	}

	bool oddRow = false;
	for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
		for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
			unsigned int a = (y) * (X_SEGMENTS + 1) + x;
			unsigned int b = (y + 1) * (X_SEGMENTS + 1) + x;

			// counter-clockwise seen from outside, so GL_BACK culling keeps the outer surface
			indices.push_back(a);
			indices.push_back(a + 1);
			indices.push_back(b);

			indices.push_back(b);
			indices.push_back(a + 1);
			indices.push_back(b + 1);
		}
	}
	return new Mesh(vertices, indices);
}
