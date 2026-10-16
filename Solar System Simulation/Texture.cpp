#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

Texture::Texture() {}

Texture::Texture(std::string dir, std::string path)
	: dir(dir), path(path) {
	generate();
}

void Texture::generate() {
	glGenTextures(1, &id);
}

void Texture::load(bool flip, bool srgb) {
	stbi_set_flip_vertically_on_load(flip);

	int width, height, nChannels;
	unsigned char* data = stbi_load((dir + "/" + path).c_str(), &width, &height, &nChannels, 0);

	GLenum colorMode = GL_RGB;
	GLenum internalMode = srgb ? GL_SRGB8 : GL_RGB;
	switch (nChannels) {
	case 1:
		colorMode = GL_RED;
		internalMode = GL_RED;
		break;
	case 4:
		colorMode = GL_RGBA;
		internalMode = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA;
		break;
	}

	if (data) {
		glBindTexture(GL_TEXTURE_2D, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // rows of 3-channel images are not 4-byte aligned in general
		glTexImage2D(GL_TEXTURE_2D, 0, internalMode, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// (these two were swapped: mipmaps are only valid for MIN, and MAG_FILTER rejects MIPMAP modes)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else {
		std::cerr << "IMAGE NOT LOADED AT: " << path << std::endl;
	}

	stbi_image_free(data);
}


void Texture::bind() {
	glBindTexture(GL_TEXTURE_2D, id);
}

