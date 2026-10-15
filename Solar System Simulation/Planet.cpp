#include "Planet.h"

Mesh* Planet::sharedSphere = nullptr;

Planet::Planet(const std::string& name, float radius, float distanceFromPlanet, 
	float orbitSpeedDegPerSec, float rotationSpeedDegPerSec, GLuint textureID, std::shared_ptr<Planet> parent)
	: name(name), radius(radius), distanceFromPlanet(distanceFromPlanet), 
	orbitSpeed(orbitSpeedDegPerSec), rotationSpeed(rotationSpeedDegPerSec), 
	orbitAngleDeg(0.0f), rotationAngleDeg(0.0f), orbitAxis(0.0f, 1.0f, 0.0f), 
	rotationAxis(0.0f, 1.0f, 0.0f), textureID(textureID), parent(parent)
	
	{
	if (!sharedSphere) {
		sharedSphere = Mesh::CreateSphere(64, 64);
	}
}

void Planet::update(float deltaTime, float timeScale){
	float scaledDt = deltaTime * timeScale;
	orbitAngleDeg += orbitSpeed * scaledDt;
	rotationAngleDeg += rotationSpeed * scaledDt;

	// keep angles in same ranges
	if (orbitAngleDeg >= 360.0f) orbitAngleDeg -= 360.0f;
	if (rotationAngleDeg >= 360.0f) rotationAngleDeg -= 360.0f;

	for (auto& c : children) c->update(deltaTime, timeScale);
}

void Planet::draw(Shader& shader, const glm::mat4& parentModel){
	glm::mat4 model = parentModel * computeLocalModel();

	shader.activate();
	shader.setMat4("model", model);

	shader.setInt("diffuse0", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	

	sharedSphere->draw();

	glBindTexture(GL_TEXTURE_2D, 0);

	// draw children
	for (auto& c : children)
		c->draw(shader, model);
}

glm::vec3 Planet::positionWorld() const{
	glm::mat4 w = computeWorldModel();
	return glm::vec3(w * glm::vec4(0.0, 0.0, 0.0, 1.0));
}

glm::mat4 Planet::computeLocalModel() const{
	glm::mat4 model(1.0f);

	// orbit rotation around partents origin
	model = glm::rotate(model, glm::radians(orbitAngleDeg), orbitAxis);

	// translate out to orbital radius
	model = glm::translate(model, glm::vec3(distanceFromPlanet, 0.0f, 0.0f));

	// apply axial tilt and rotation
	model = glm::rotate(model, glm::radians(rotationAngleDeg), rotationAxis);

	// scale to planet radius
	model = glm::scale(model, glm::vec3(radius));

	return model;
}

glm::mat4 Planet::computeWorldModel() const{
	if (parent) return parent->computeWorldModel() * computeLocalModel();
	return computeLocalModel();
}
