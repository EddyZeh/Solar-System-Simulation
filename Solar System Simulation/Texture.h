#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/scene.h>

class Texture {
public:

	Texture();
	Texture(std::string dir, std::string path);

	void generate();
	void load(bool flip = true, bool srgb = true);

	unsigned int id;
	aiTextureType type;
	std::string dir;
	std::string path;

	void bind();
};

#endif // !TEXTURE_H

