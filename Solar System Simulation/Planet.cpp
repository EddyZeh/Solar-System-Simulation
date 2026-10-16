#include "Planet.h"

Mesh* Planet::sharedSphere = nullptr;

Planet::Planet(const std::string& name, float radius, double mass, float orbitRadius,
	float spinDegPerSec, float axialTiltDeg, GLuint textureID, std::shared_ptr<Planet> parent, float emission)
	: name(name), radius(radius), mass(mass), emission(emission),
	position(0.0), relPosition(0.0), relVelocity(0.0),
	orientation(1.0, 0.0, 0.0, 0.0), angularMomentum(0.0),
	momentOfInertia(physics::solidSphereInertia(mass, radius)),
	textureID(textureID), parent(parent)
{
	if (!sharedSphere) {
		sharedSphere = Mesh::CreateSphere(64, 64);
	}

	if (parent) {
		const double mu = physics::G * (parent->mass + mass);
		relPosition = glm::dvec3(orbitRadius, 0.0, 0.0);
		relVelocity = glm::dvec3(0.0, 0.0, -physics::circularSpeed(mu, orbitRadius));
		position = parent->position + relPosition;
	}

	orientation = glm::angleAxis(-glm::radians((double)axialTiltDeg), glm::dvec3(0.0, 0.0, 1.0));
	const glm::dvec3 spinAxis = orientation * glm::dvec3(0.0, 1.0, 0.0);
	angularMomentum = momentOfInertia * glm::radians((double)spinDegPerSec) * spinAxis;
}

void Planet::update(double dt) {
	
	if (parent) {
		const double mu = physics::G * (parent->mass + mass);
		physics::stepTwoBody(relPosition, relVelocity, mu, dt);
		position = parent->position + relPosition;
	}

	const glm::dvec3 omega = angularMomentum / momentOfInertia;
	physics::stepOrientation(orientation, omega, dt);

	for (auto& c : children) c->update(dt);
}

void Planet::draw(Shader& shader) {
	glm::mat4 model = glm::mat4_cast(glm::quat(orientation));
	model[0] *= radius;
	model[1] *= radius;
	model[2] *= radius;
	model[3] = glm::vec4(glm::vec3(position), 1.0f);

	shader.activate();
	shader.setMat4("model", model);
	shader.setFloat("emission", emission);

	shader.setInt("diffuse0", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	sharedSphere->draw();

	glBindTexture(GL_TEXTURE_2D, 0);

	// draw children
	for (auto& c : children)
		c->draw(shader);
}
