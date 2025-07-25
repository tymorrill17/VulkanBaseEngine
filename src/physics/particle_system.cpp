#include "physics/particle_system.h"
#include <random>

static const glm::vec2 down{ 0.0f, -0.1f };
static const double pi = 3.14159265358979323846;
static bool usePredictedPositions = false;

static long double norm(glm::vec2 v) {
	return glm::sqrt(v.x * v.x + v.y * v.y);
}

ParticleSystem2D::ParticleSystem2D(
	GlobalParticleInfo& particleInfo, 
	GlobalPhysicsInfo& physicsInfo,
	BoundingBox& box,
	InputManager& inputManager,
	Hand* hand
	) :
	_globalParticleInfo(particleInfo),
	_globalPhysics(physicsInfo),
	_bbox(box),
	_inputManager(inputManager),
	_interactionHand(hand),
	_simulationPaused(false),
	_doOneFrame(false) {

	_particles = new RenderedParticle2D[MAX_PARTICLES];
	_densities = new float[MAX_PARTICLES];

	_acceleration = new glm::vec2[MAX_PARTICLES];
	_particleIndices = new uint32_t[MAX_PARTICLES];
	_spatialLookup = new uint32_t[MAX_PARTICLES];
	_startIndices = new uint32_t[MAX_PARTICLES];

	_particles2 = new Particle2D[MAX_PARTICLES];
	// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
	//_particles3 = new Particle2D[MAX_PARTICLES];
	//_particles4 = new Particle2D[MAX_PARTICLES];

	// Initialize all entries to 0 in case we add more
	for (int i = 0; i < MAX_PARTICLES; i++) {
		_densities[i] = 0.0f;
		_particles2[i].position = { 0.f, 0.f }; _particles2[i].velocity = { 0.f, 0.f };
		// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
		//_particles3[i].position = { 0.f, 0.f }; _particles3[i].velocity = { 0.f, 0.f };
		//_particles4[i].position = { 0.f, 0.f }; _particles4[i].velocity = { 0.f, 0.f };

		_particleIndices[i] = i;
		_spatialLookup[i] = 0;
		_startIndices[i] = INT_MAX;
	}
	arrangeParticles();
	assignInputEvents();
}

ParticleSystem2D::~ParticleSystem2D() {
	delete[] _particles;
	delete[] _densities;

	delete[] _acceleration;
	delete[] _particleIndices;
	delete[] _spatialLookup;
	delete[] _startIndices;

	delete[] _particles2;
	// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
	//delete[] _particles3;
	//delete[] _particles4;
}

// @brief Returns an integer vector containing the indices of the grid cell the position corresponds to
static glm::ivec2 getGridCell(glm::vec2 position, int cellSize) {
	int cellX = static_cast<int>(position.x / cellSize);
	int cellY = static_cast<int>(position.y / cellSize);
	//std::cout << "Grid Cell Coordinates: (" << cellX << ", " << cellY << ")" << std::endl;
	return glm::vec2{ cellX, cellY };
}

// @brief Returns the hash code of the given grid cell (modulo hashSize)
static uint32_t hashGridCell(glm::ivec2 gridCell, uint32_t hashSize) {
	const uint32_t p1 = 73856093;
	const uint32_t p2 = 19349663;
	// const int p3 = 83492791;

	return (static_cast<uint32_t>(gridCell.x) * p1 + static_cast<uint32_t>(gridCell.y) * p2) % hashSize;
}

void ParticleSystem2D::arrangeParticles() {
	float spacing = _globalParticleInfo.radius + _globalParticleInfo.spacing;
	glm::vec2 offset = glm::vec2();

	// Random number generator for randomizing velocity (or position)
	//std::default_random_engine generator;
	//std::uniform_real_distribution<double> distributiony(_bbox.bottom, _bbox.top);
	//std::uniform_real_distribution<double> distributionx(_bbox.left, _bbox.right);

	// Calculate the size of the grid based on how many particles we have
	int gridSize = static_cast<int>(glm::ceil(glm::sqrt(static_cast<float>(_globalParticleInfo.numParticles))));
	offset = glm::vec2(-(gridSize-1)*spacing); // Center the grid around the origin

	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		// Arrange the positions of the particles into grids
		_particles[i].position.x = static_cast<float>((i) % gridSize) * 2.0f * spacing + offset.x;
		_particles[i].position.y = static_cast<float>((i) / gridSize) * 2.0f * spacing + offset.y;

		// Initialize the color of the particle to the default
		_particles[i].color = glm::vec4{ _globalParticleInfo.defaultColor[0], _globalParticleInfo.defaultColor[1], _globalParticleInfo.defaultColor[2], _globalParticleInfo.defaultColor[3] };

		// Set a random starting velocity
		// _particles[i].velocity = glm::vec2{ distribution(generator), distribution(generator) };
		_particles[i].velocity = glm::vec2{ 0.f, 0.f };
	}
}

static const int numThreads = 16;

void ParticleSystem2D::update() {
	if (_simulationPaused && !_doOneFrame) {
		return;
	}

	static Timer& timer = Timer::getTimer();
	float subDeltaTime = timer.frameTime() / _globalPhysics.nSubsteps;
	//float predictionStep = 1.f / 120.f; // Used to gain some stability with the position-prediction code. I should refine this later on.

	// Parallel batching setup
	std::vector<int> batchSizes;
	batchSizes.reserve(numThreads);
	int batchSize = _globalParticleInfo.numParticles / numThreads; // Divides the work to be done into equal batches
	// In case numThreads doesn't divide numParticles evenly. If numThreads divides numParticles, then this should be equal to batchSize
	int oddBatchOut = _globalParticleInfo.numParticles - (numThreads - 1) * batchSize;

	// Fill batchSizes 
	for (int i = 0; i < numThreads-1; i++) {
		batchSizes.push_back(batchSize);
	}
	batchSizes.push_back(oddBatchOut);

	glm::vec2* l2 = new glm::vec2[_globalParticleInfo.numParticles];
	// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
	//glm::vec2* l3 = new glm::vec2[_globalParticleInfo.numParticles];
	//glm::vec2* l4 = new glm::vec2[_globalParticleInfo.numParticles];

	for (int i = 0; i < _globalPhysics.nSubsteps; i++) {

		float halfDeltaTime = 0.5f * subDeltaTime;

		// For RK4, the position and velocity of _particles[i] acts as the INITIAL values until the end

		// Update the spatial lookup arrays for use in calculating densities and forces
		updateSpatialLookup<RenderedParticle2D>(_particles);

		// Finds density at current r_i
		calculateParticleDensitiesParallel<RenderedParticle2D>(batchSizes, _particles);
		// Wait for futures to get results
		for (int i = 0; i < numThreads; i++) {
			_futures[i].get();
		}
		_futures.clear();
		// Does an euler step of dv/dt to get velocity at the (i+1)th step
		getAccelerationParallel<RenderedParticle2D>(batchSizes, _acceleration, _particles);
		for (int i = 0; i < numThreads; i++) {
			_futures[i].get();
		}
		_futures.clear();
		// Now we have l1=_acceleration and k1=_particles[i].velocity

		// Find k2 and l2
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
			//_particles2[i].position = _particles[i].position + _particles[i].velocity * halfDeltaTime;
			//_particles2[i].velocity = _particles[i].velocity + halfDeltaTime * _acceleration[i]; // This is k2
			_particles2[i].velocity = _particles[i].velocity + subDeltaTime * _acceleration[i]; // This is k2
			_particles2[i].position = _particles[i].position + subDeltaTime * _particles[i].velocity;
		}
		updateSpatialLookup<Particle2D>(_particles2);
		calculateParticleDensitiesParallel<Particle2D>(batchSizes, _particles2);
		for (int i = 0; i < numThreads; i++) {
			_futures[i].get();
		}
		_futures.clear();
		getAccelerationParallel<Particle2D>(batchSizes, l2, _particles2); // This finds l2
		for (int i = 0; i < numThreads; i++) {
			_futures[i].get();
		}
		_futures.clear();

		// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
		//// Find k3 and l3
		//for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		//	_particles3[i].velocity = _particles[i].velocity + halfDeltaTime * l2[i]; // This is k3
		//	_particles3[i].position = _particles[i].position + _particles2[i].velocity * halfDeltaTime;
		//}
		//updateSpatialLookup<Particle2D>(_particles3);
		//calculateParticleDensitiesParallel<Particle2D>(batchSizes, _particles3);
		//for (int i = 0; i < numThreads; i++) {
		//	_futures[i].get();
		//}
		//_futures.clear();
		//getAccelerationParallel<Particle2D>(batchSizes, l3, _particles3);
		//for (int i = 0; i < numThreads; i++) {
		//	_futures[i].get();
		//}
		//_futures.clear();

		//// Find k4 and l4
		//for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		//	_particles4[i].velocity = _particles[i].velocity + 2.f * halfDeltaTime * l3[i]; // This is k4
		//	_particles4[i].position = _particles[i].position + _particles3[i].velocity * 2.f * halfDeltaTime;
		//}
		// updateSpatialLookup<Particle2D>(_particles4);
		//calculateParticleDensitiesParallel<Particle2D>(batchSizes, _particles4);
		//for (int i = 0; i < numThreads; i++) {
		//	_futures[i].get();
		//}
		//_futures.clear();
		//getAccelerationParallel<Particle2D>(batchSizes, l4, _particles4);
		//for (int i = 0; i < numThreads; i++) {
		//	_futures[i].get();
		//}
		//_futures.clear();

		// Then combine it all to get the next position
		for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
			// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
			//_particles[i].velocity += subDeltaTime / 6.f * (_acceleration[i] + 2.f * l2[i] + 2.f * l3[i] + l4[i]);
			//_particles[i].position += subDeltaTime / 6.f * (_particles[i].velocity + 2.f * _particles2[i].velocity + 2.f * _particles3[i].velocity + _particles4[i].velocity);
			_particles[i].velocity += halfDeltaTime * (_acceleration[i] + l2[i]);
			_particles[i].position += halfDeltaTime * (_particles[i].velocity + _particles2[i].velocity);
		}

		// Resolve collisions between particles
		// resolveParticleCollisions();

		// Resolve collisions with the walls of the bounding box
		resolveBoundaryCollisions();
	}
	frameDone();

	delete[] l2;
	// Uncomment for RK4 (slower and didn't have much improvement in terms of stability...)
	//delete[] l3;
	//delete[] l4;
}

void ParticleSystem2D::resolveBoundaryCollisions() {
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		if (_particles[i].position.y < (_bbox.bottom + _globalParticleInfo.radius)) {
			_particles[i].position.y = _bbox.bottom + _globalParticleInfo.radius;
			_particles[i].velocity.y = -_particles[i].velocity.y * _globalPhysics.boundaryDampingFactor;
		}
		else if (_particles[i].position.y > (_bbox.top - _globalParticleInfo.radius)) {
			_particles[i].position.y = _bbox.top - _globalParticleInfo.radius;
			_particles[i].velocity.y = -_particles[i].velocity.y * _globalPhysics.boundaryDampingFactor;
		}
		if (_particles[i].position.x > (_bbox.right - _globalParticleInfo.radius)) {
			_particles[i].position.x = _bbox.right - _globalParticleInfo.radius;
			_particles[i].velocity.x = -_particles[i].velocity.x * _globalPhysics.boundaryDampingFactor;
		}
		else if (_particles[i].position.x < (_bbox.left + _globalParticleInfo.radius)) {
			_particles[i].position.x = _bbox.left + _globalParticleInfo.radius;
			_particles[i].velocity.x = -_particles[i].velocity.x * _globalPhysics.boundaryDampingFactor;
		}
	}
}

template<typename ParticleType>
float ParticleSystem2D::calculateDensity(uint32_t particleIndex, ParticleType* particles) {
	float density = 0.0f;
	// Use the locations of each particle to calculate the density at position, with the smoothing function lessening the impact of particles further away
	loopThroughNearbyPoints(particles[particleIndex].position, particles, [&](glm::vec2 dist, int particleIndex) {
		float squareDst = glm::dot(dist, dist);
		density += SmoothingKernels2D::smooth(squareDst, _globalPhysics.densitySmoothingRadius);
	});
	if (density == 0.0f) {
		std::cout << "ERROR: density is 0 for particleIndex: " << particleIndex << std::endl;
	}
	return density;
}

template<typename ParticleType>
void ParticleSystem2D::calculateParticleDensitiesParallel(std::vector<int> batchSizes, ParticleType* particles) {
	// We want to calculate the density at each particle location all at once.
	int start = 0;
	int end = 0;
	for (int i = 0; i < numThreads; i++) {
		start = end;
		end = start + batchSizes[i];
		_futures.push_back(std::async(std::launch::async, [this, particles](int startIndex, int endIndex) {
			for (int i = startIndex; i < endIndex; i++) {
				_densities[i] = calculateDensity(i, particles); // for k1
			}
		}, start, end));
	}
}

template<typename ParticleType>
glm::vec2 ParticleSystem2D::getAcceleration(uint32_t particleIndex, ParticleType* particles) {
	static Timer& timer = Timer::getTimer();

	// initialize each acceleration type
	glm::vec2 handAcceleration{ 0.f, 0.f };
	glm::vec2 pressureAcceleration{ 0.f, 0.f };

	// Input actions modify gravity
	if (_interactionHand->isInteracting()) {
		float interactionStrength = _interactionHand->action() == HandAction::pulling ? _interactionHand->strengthFactor : -_interactionHand->strengthFactor;
		// Hand is interacting, so find the vector from the hand to the particle and find its squared distance
		glm::vec2 particleToHand = _interactionHand->position() - particles[particleIndex].position;
		float sqrDst = glm::dot(particleToHand, particleToHand);

		// If particle is in hand radius, change acceleration on particle
		if (sqrDst < _interactionHand->radius * _interactionHand->radius) {
			float dst = glm::sqrt(sqrDst);
			// Adding acceleration based on how far away the hand is... Could potentially use one of our smoothing functions for this
			float centerFactor = 1 - dst / _interactionHand->radius;
			particleToHand = particleToHand / dst; // Normalize the direction vector
			handAcceleration += (particleToHand * interactionStrength - particles[particleIndex].velocity) * centerFactor;
		}
	}

	// Get force due to pressure and convert it to acceleration by dividing by density
	pressureAcceleration = calculatePressureForce(particleIndex, particles, _densities) / _densities[particleIndex];
	glm::vec2 gravityAcceleration = _globalPhysics.gravity * down;
	return handAcceleration + pressureAcceleration + gravityAcceleration;
}

template<typename ParticleType>
void ParticleSystem2D::getAccelerationParallel(std::vector<int> batchSizes, glm::vec2* outputAccel, ParticleType* particles) {
	int start = 0;
	int end = 0;
	for (int i = 0; i < numThreads; i++) {
		start = end;
		end = start + batchSizes[i];
		_futures.push_back(std::async(std::launch::async, [this, particles, outputAccel](int startIndex, int endIndex) {
			for (int i = startIndex; i < endIndex; i++) {
				// getAcceleration applies gravity, interaction force, and pressure force at once
				outputAccel[i] = getAcceleration(i, particles); // This is dv/dt (and k1)
			}
			}, start, end));
	}
}

static glm::vec2 getRandomDirection() {
	// Random number generator for randomizing velocity (or position)
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(-1, 1);

	return glm::normalize(glm::vec2{ distribution(generator), distribution(generator) });
}

float ParticleSystem2D::getPressure(float density) {
	return (density - _globalPhysics.restDensity) * _globalPhysics.pressureConstant;
}

float ParticleSystem2D::getSharedPressure(float density, float otherDensity) {
	float pressure = getPressure(density);
	float otherPressure = getPressure(otherDensity);
	return (pressure + otherPressure) * 0.5f;
}

template<typename ParticleType>
glm::vec2 ParticleSystem2D::calculatePressureForce(int particleIndex, ParticleType* particles, float* densities) {
	glm::vec2 force{ 0.0f, 0.0f };
	// We are finding a field quantity like density, so we use the SPH equation. This involves looping over each particle that contributes to the quantity
	loopThroughNearbyPoints(particles[particleIndex].position, particles, [this, densities, particleIndex, &force](glm::vec2 dist, int index) {
		if (index == particleIndex) return; // The particle itself doesn't contribute to the pressure force it feels

		float squareDst = glm::dot(dist, dist);
		// If particles are on top of each other, pick a random normal direction
		glm::vec2 direction = (squareDst == 0.0f) ? getRandomDirection() : dist / glm::sqrt(squareDst);

		// The pressure force needs to follow newton's third law, so instead of using the particles full pressure, take the average between particle index and particle j
		// Then multiply with the opposite direction to
		force += getSharedPressure(densities[particleIndex], densities[index]) * direction * SmoothingKernels2D::spikeyDerivative(squareDst, _globalPhysics.densitySmoothingRadius) / densities[index];
	});
	return force;
}

static const std::vector<glm::ivec2> gridCellOffsets {
	{1, 1}, {1, 0}, {1, -1},
	{0, 1}, {0, -1}, {0, 0},
	{-1, 0}, {-1, 1}, {-1, -1}
};

template<typename ParticleType>
void ParticleSystem2D::loopThroughNearbyPoints(glm::vec2 particlePosition, ParticleType* particles, std::function<void(glm::vec2, uint32_t)> callback) {
	// Get the center grid cell
	glm::ivec2 center = getGridCell(particlePosition, _globalPhysics.densitySmoothingRadius);
	float squareSmoothingRadius = _globalPhysics.densitySmoothingRadius * _globalPhysics.densitySmoothingRadius;

	for (auto& offset : gridCellOffsets) {
		uint32_t gridKey = hashGridCell(center + offset, _globalParticleInfo.numParticles);
		uint32_t cellStartIndex = _startIndices[gridKey];

		// Loop through the rest of the particles in the grid cell
		for (int i = cellStartIndex; i < _globalParticleInfo.numParticles; i++) {
			if (_spatialLookup[i] != gridKey) break;

			uint32_t particleIndex = _particleIndices[i];
			glm::vec2 dist{ 0.f, 0.f };
			dist = particles[particleIndex].position - particlePosition;
			float squareDst = glm::dot(dist, dist);
			if (squareDst <= squareSmoothingRadius) {
				callback(dist, particleIndex);
			}
		}
	}
}

// TODO: Make this more efficient using the same grid system as the fluid simulation calculations
void ParticleSystem2D::resolveParticleCollisions() {
	// Brute-force collision detection
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		for (int j = i + 1; j < _globalParticleInfo.numParticles; j++) {

			// Calculate the normalized vector pointing from particle i to j
			glm::vec2 itojDirection = _particles[j].position - _particles[i].position;
			float distance = norm(itojDirection); // Distance between the two particles

			// Check to see if the particles collide
			if (distance < 2.0f * _globalParticleInfo.radius) {
				itojDirection = itojDirection / distance; // normalize the vector

				// Update the colliding particles' positions
				float posCorrection = 0.5f * (2.0f * _globalParticleInfo.radius - distance); // how much to move the particles after colliding
				_particles[i].position += -posCorrection * itojDirection;
				_particles[j].position += posCorrection * itojDirection;

				// Update the colliding particles' velocities
				// Compute the velocities in the direction of the collision
				float v1 = glm::dot(_particles[i].velocity, itojDirection);
				float v2 = glm::dot(_particles[j].velocity, itojDirection);
				// Update the particles' velocities
				_particles[i].velocity += ((0.5f * (v1 + v2 - (v1 - v2) * _globalPhysics.collisionDampingFactor)) - v1) * itojDirection;
				_particles[j].velocity += ((0.5f * (v1 + v2 - (v2 - v1) * _globalPhysics.collisionDampingFactor)) - v2) * itojDirection;
			}
		}
	}
}

void ParticleSystem2D::sortSpatialArrays() {
	
	int numParticles = _globalParticleInfo.numParticles;
	uint32_t* indices = new uint32_t[numParticles];

	// Fill indices array
	for (int i = 0; i < numParticles; i++) {
		indices[i] = i;
	}

	// Sort the indices array based on the given lambda function comparing the spatial lookup array
	std::sort(indices, indices + numParticles, [&](uint32_t i, uint32_t j) {
			return _spatialLookup[i] < _spatialLookup[j];
		});

	// Create temporary arrays to store sorted results
	uint32_t* sortedParticleIndices = new uint32_t[numParticles];
	uint32_t* sortedSpatialLookup = new uint32_t[numParticles];

	for (int i = 0; i < numParticles; i++) {
		sortedParticleIndices[i] = _particleIndices[indices[i]];
		sortedSpatialLookup[i] = _spatialLookup[indices[i]];
	}

	for (int i = 0; i < numParticles; i++) {
		_particleIndices[i] = sortedParticleIndices[i];
		_spatialLookup[i] = sortedSpatialLookup[i];
	}
	
	delete[] indices;
	delete[] sortedParticleIndices;
	delete[] sortedSpatialLookup;
}

template<typename ParticleType>
void ParticleSystem2D::updateSpatialLookup(ParticleType* particles) {

	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		// First, get the spatial grid cell index and its hash value
		glm::ivec2 gridCellIndex{ 0, 0 };
		gridCellIndex = getGridCell(particles[i].position, _globalPhysics.densitySmoothingRadius);
		uint32_t gridCellHashValue = hashGridCell(gridCellIndex, _globalParticleInfo.numParticles);

		_particleIndices[i] = i;
		_spatialLookup[i] = gridCellHashValue;
		_startIndices[i] = INT_MAX; // reset the start indices
	}

	// Sort _particleIndices and _spatialLookup based on _spatialLookup
	sortSpatialArrays();

	// Calculate the start indices for each non-empty grid cell
	for (int i = 0; i < _globalParticleInfo.numParticles; i++) {
		uint32_t gridKey = _spatialLookup[i];
		uint32_t prevGridKey = i == 0 ? INT_MAX : _spatialLookup[i - 1];
		if (gridKey != prevGridKey) {
			_startIndices[gridKey] = i;
		}
	}
}

void ParticleSystem2D::assignInputEvents() {
	if (!_interactionHand) return;
	_inputManager.addListener(InputEvent::leftMouseDown, [&]() {
		_interactionHand->setAction(HandAction::pushing);
	});
	_inputManager.addListener(InputEvent::leftMouseUp, [&]() {
		_interactionHand->setAction(HandAction::idle);
	});
	_inputManager.addListener(InputEvent::rightMouseUp, [&]() {
		_interactionHand->setAction(HandAction::idle);
	});
	_inputManager.addListener(InputEvent::rightMouseDown, [&]() {
		_interactionHand->setAction(HandAction::pulling);
	});
	_inputManager.addListener(InputEvent::spacebarDown, [&]() {
		_simulationPaused = _simulationPaused ? false : true;
	});
	_inputManager.addListener(InputEvent::rightArrowDown, [&]() {
		if (_simulationPaused) {
			proceedFrame();
		}
	});
}

void ParticleSystem2D::proceedFrame() {
	_doOneFrame = true;
}

void ParticleSystem2D::frameDone() {
	_doOneFrame = false;
}

// ----------------------------------------------- SMOOTHING KERNELS --------------------------------------------- //

float SmoothingKernels2D::smooth(float squareDst, float smoothingRadius) {
	if (squareDst > smoothingRadius*smoothingRadius)
		return 0;

	return 4.f / (pi * pow(smoothingRadius, 8)) * pow(smoothingRadius*smoothingRadius - squareDst, 3);
}

float SmoothingKernels2D::smoothDerivative(float squareDst, float smoothingRadius) {
	float rmag = sqrt(squareDst);
	if (rmag > smoothingRadius)
		return 0;

	return -24.f / (pi * pow(smoothingRadius, 8)) * rmag * pow(smoothingRadius * smoothingRadius - squareDst, 2);
}

float SmoothingKernels2D::spikey(float squareDst, float smoothingRadius) {
	float rmag = glm::sqrt(squareDst);
	if (rmag > smoothingRadius)
		return 0;

	return 10.f / (pi * pow(smoothingRadius, 5)) * pow(smoothingRadius - rmag, 3);
}

float SmoothingKernels2D::spikeyDerivative(float squareDst, float smoothingRadius) {
	float rmag = glm::sqrt(squareDst);
	if (rmag > smoothingRadius)
		return 0;
	
	return -30.f / (pi * pow(smoothingRadius, 5)) * pow(smoothingRadius - rmag, 2);
}


