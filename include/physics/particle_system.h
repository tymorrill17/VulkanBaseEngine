#pragma once
#include "glm/glm.hpp"
#include "NonCopyable.h"
#include <vector>

struct Particle2D {
	glm::vec2 position{ 0.0f, 0.0f };
	glm::vec2 velocity{ 0.0f, 0.0f };
};

class ParticleSystem2D : public NonCopyable {
public:
	ParticleSystem2D(const int numParticles, const float radius, Particle2D* particles);

	// @brief initialize the particles in a grid
	void arrangeParticles();
	// @brief Runs every frame and updates the positions of the particles
	void update();

protected:
	const int _numParticles;
	float _particleRadius;
	Particle2D* _particles; // Array of 2D particles
};