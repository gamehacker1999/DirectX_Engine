#pragma once
//class to manage a specific group of particles
#include"Particles.h"
#include<vector>

#define MAX_PARTICLES 250
class Emitter
{
	std::vector<Particle> particles;
	std::vector<ParticleVertex> particleVertices; //four times as many vertices
	unsigned int lifetime; //life of each particle
	XMFLOAT3 emitterPosition;

public:
	Emitter();
	~Emitter();

	XMFLOAT3 GetPosition();
	void SetPosition(XMFLOAT3 pos);
	void BeginEmission();

	void UpdateParticles(float deltaTime);
};

