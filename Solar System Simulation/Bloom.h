#pragma once

#ifndef BLOOM_H
#define BLOOM_H

#include <glad/glad.h>
#include "Shader.h"

class Bloom {
public:
	static constexpr int LEVELS = 5;   // blur levels at 1/2, 1/4, 1/8, 1/16, 1/32 resolution


	float threshold = 1.0f; 
	float strength = 0.9f;  
	float exposure = 1.0f;   

	void init(int width, int height);   
	void resize(int width, int height);
	void bindScene();                   // bind HDR framebuffer, set viewport, clear
	void render();                      // run the post-process and draw to the default framebuffer

private:
	struct Target {
		GLuint fbo = 0;
		GLuint tex = 0;
		int w = 0, h = 0;
	};

	void allocate(Target& t, int w, int h);
	void drawQuad();

	int width = 0, height = 0;

	Target scene;            
	GLuint sceneDepth = 0;   
	Target ping[LEVELS];    
	Target pong[LEVELS];

	Shader brightShader, blurShader, downsampleShader, finalShader;

	GLuint quadVAO = 0, quadVBO = 0;
};

#endif // !BLOOM_H
