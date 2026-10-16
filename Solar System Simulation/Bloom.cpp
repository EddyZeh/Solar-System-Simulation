#include "Bloom.h"

#include <algorithm>
#include <iostream>
#include <string>

void Bloom::init(int w, int h) {
	brightShader.generate("screen.vert", "bright.frag");
	blurShader.generate("screen.vert", "blur.frag");
	downsampleShader.generate("screen.vert", "downsample.frag");
	finalShader.generate("screen.vert", "final.frag");

	// full-screen quad (pos.xy, uv.xy)
	float quadVertices[] = {
		-1.f, -1.f,   0.f, 0.f,
		 1.f, -1.f,   1.f, 0.f,
		 1.f,  1.f,   1.f, 1.f,

		-1.f, -1.f,   0.f, 0.f,
		 1.f,  1.f,   1.f, 1.f,
		-1.f,  1.f,   0.f, 1.f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	
	glGenFramebuffers(1, &scene.fbo);
	glGenTextures(1, &scene.tex);
	glGenRenderbuffers(1, &sceneDepth);
	for (int i = 0; i < LEVELS; i++) {
		glGenFramebuffers(1, &ping[i].fbo); glGenTextures(1, &ping[i].tex);
		glGenFramebuffers(1, &pong[i].fbo); glGenTextures(1, &pong[i].tex);
	}

	resize(w, h);
}


void Bloom::allocate(Target& t, int w, int h) {
	t.w = w;
	t.h = h;

	glBindTexture(GL_TEXTURE_2D, t.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, t.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t.tex, 0);
}

void Bloom::resize(int w, int h) {
	if (w <= 0 || h <= 0) return;   // minimised window
	width = w;
	height = h;

	allocate(scene, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, sceneDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sceneDepth);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "HDR framebuffer not complete" << std::endl;

	for (int i = 0; i < LEVELS; i++) {
		const int lw = std::max(1, w >> (i + 1));
		const int lh = std::max(1, h >> (i + 1));
		allocate(ping[i], lw, lh);
		allocate(pong[i], lw, lh);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Bloom::bindScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, scene.fbo);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Bloom::drawQuad() {
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Bloom::render() {
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);

	// 1) bright pass: scene -> level 0 (half resolution)
	glBindFramebuffer(GL_FRAMEBUFFER, ping[0].fbo);
	glViewport(0, 0, ping[0].w, ping[0].h);
	brightShader.activate();
	brightShader.setInt("scene", 0);
	brightShader.setFloat("threshold", threshold);
	glBindTexture(GL_TEXTURE_2D, scene.tex);
	drawQuad();

	// 2) for every level: separable Gaussian blur, then downsample into the next level
	for (int i = 0; i < LEVELS; i++) {
		blurShader.activate();
		blurShader.setInt("image", 0);

		glViewport(0, 0, ping[i].w, ping[i].h);

		glBindFramebuffer(GL_FRAMEBUFFER, pong[i].fbo);   // horizontal: ping -> pong
		blurShader.setBool("horizontal", true);
		glBindTexture(GL_TEXTURE_2D, ping[i].tex);
		drawQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, ping[i].fbo);   // vertical:   pong -> ping
		blurShader.setBool("horizontal", false);
		glBindTexture(GL_TEXTURE_2D, pong[i].tex);
		drawQuad();

		if (i + 1 < LEVELS) {
			glBindFramebuffer(GL_FRAMEBUFFER, ping[i + 1].fbo);
			glViewport(0, 0, ping[i + 1].w, ping[i + 1].h);
			downsampleShader.activate();
			downsampleShader.setInt("image", 0);
			glBindTexture(GL_TEXTURE_2D, ping[i].tex);
			drawQuad();
		}
	}

	// 3) composite: scene + all blur levels -> tone map -> gamma -> screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	finalShader.activate();
	finalShader.setInt("scene", 0);
	for (int i = 0; i < LEVELS; i++)
		finalShader.setInt("bloom" + std::to_string(i), 1 + i);
	finalShader.setFloat("exposure", exposure);
	finalShader.setFloat("bloomStrength", strength);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scene.tex);
	for (int i = 0; i < LEVELS; i++) {
		glActiveTexture(GL_TEXTURE1 + i);
		glBindTexture(GL_TEXTURE_2D, ping[i].tex);
	}
	drawQuad();

	// leave the texture units and GL state the way the main loop expects them
	for (int i = LEVELS; i >= 0; i--) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}
