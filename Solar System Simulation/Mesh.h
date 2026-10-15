#pragma once

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>

#include "glMemory.hpp"

class Mesh {
public:
	Mesh(std::vector<float>& vertices,
		 std::vector<unsigned int>& indices);

	void draw();

	static Mesh* CreateSphere(unsigned int X_SEGMENTS, unsigned int Y_SEGMENTS);

private:
	//GLuint VAO, VBO, EBO;
	ArrayObject VAO;
	size_t indexCount;
};


#endif // !MESH_H
