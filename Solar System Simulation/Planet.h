#pragma once

#ifndef PLANET_H
#define PLANET_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>
#include <vector>

#include "Shader.h"
#include "Mesh.h"
#include "Physics.h"

class Planet {
public:
	Planet(const std::string& name,
		float radius,
		double mass,
		float orbitRadius,          
		float spinDegPerSec,        
		float axialTiltDeg,         
		GLuint textureID,
		std::shared_ptr<Planet> parent = nullptr,
		float emission = 0.0f);     
	
	void update(double dt);
	void draw(Shader& shader);

	glm::vec3 positionWorld() const { return glm::vec3(position); }

	void addChild(const std::shared_ptr<Planet>& c) { children.push_back(c); }

	std::string name;
	float radius;
	double mass;
	float emission;

	// --- physical state (double precision) ---
	glm::dvec3 position;          
	glm::dvec3 relPosition;       
	glm::dvec3 relVelocity;       
	glm::dquat orientation;       
	glm::dvec3 angularMomentum;   
	double momentOfInertia;

	GLuint textureID;
	static Mesh* sharedSphere;
	std::shared_ptr<Planet> parent;
	std::vector<std::shared_ptr<Planet>> children;
};


#endif // !PLANET_H
