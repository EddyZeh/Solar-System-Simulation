#pragma once

#ifndef PLANET_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>

#include "Shader.h"
#include "Mesh.h"

class Planet {
public:
	Planet(const std::string& name,
		float radius,
		float distanceFromPlanet,
		float orbitSpeedDegPerSec,
		float rotationSpeedDegPerSec,
		GLuint textureID,
		std::shared_ptr<Planet> parent = nullptr);

	void update(float deltaTime, float timeScale = 1.0f);
	void draw(Shader& shader, const glm::mat4& parentModel);

	glm::vec3 positionWorld() const;

	void addChild(const std::shared_ptr<Planet>& c) { children.push_back(c); }

	std::string name;
	float radius;
	float distanceFromPlanet;
	float orbitSpeed;
	float rotationSpeed;
	float orbitAngleDeg;
	float rotationAngleDeg;
	glm::vec3 orbitAxis;
	glm::vec3 rotationAxis;

	GLuint textureID;
	static Mesh* sharedSphere;
	std::shared_ptr<Planet> parent;
	std::vector<std::shared_ptr<Planet>> children;


private:
	glm::mat4 computeLocalModel() const;
	glm::mat4 computeWorldModel() const;
};


#endif // !PLANET_H

