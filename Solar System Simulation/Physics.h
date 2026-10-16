#pragma once

#ifndef PHYSICS_H
#define PHYSICS_H

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace physics {

	
	constexpr double G = 1.0;

	constexpr double kFixedDt = 1.0 / 240.0;

	
	inline glm::dvec3 gravityAcceleration(const glm::dvec3& relPos, double mu) {
		const double r2 = glm::dot(relPos, relPos);
		const double invR3 = 1.0 / (r2 * std::sqrt(r2));
		return -mu * invR3 * relPos;
	}

	
	inline double circularSpeed(double mu, double r) {
		return std::sqrt(mu / r);
	}

	inline double circularAngularRate(double mu, double r) {
		return std::sqrt(mu / (r * r * r));
	}

	inline double solidSphereInertia(double mass, double radius) {
		return 0.4 * mass * radius * radius;
	}

	inline void stepTwoBody(glm::dvec3& relPos, glm::dvec3& relVel, double mu, double dt) {
		const glm::dvec3 a0 = gravityAcceleration(relPos, mu);
		relPos += relVel * dt + 0.5 * a0 * dt * dt;
		const glm::dvec3 a1 = gravityAcceleration(relPos, mu);
		relVel += 0.5 * (a0 + a1) * dt;
	}
	
	inline void stepOrientation(glm::dquat& orientation, const glm::dvec3& omega, double dt) {
		const double w = glm::length(omega);
		if (w < 1e-12) return;
		const glm::dquat dq = glm::angleAxis(w * dt, omega / w);
		orientation = glm::normalize(dq * orientation);
	}

} 

#endif // !PHYSICS_H
