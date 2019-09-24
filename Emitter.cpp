#include "Emitter.h"

Emitter::Emitter()
{
	particles.reserve(1000);
	particleVertices.reserve(4000);
	lifetime = 100;
	emitterPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

Emitter::~Emitter()
{
	particles.clear();
	particles.shrink_to_fit();

	particleVertices.clear();
	particleVertices.shrink_to_fit();
}

XMFLOAT3 Emitter::GetPosition()
{
	return emitterPosition;
}

void Emitter::SetPosition(XMFLOAT3 pos)
{
	emitterPosition = pos;
}

void Emitter::BeginEmission()
{
	//function to begin the emission of new particles
	for (int i = 0; i < 100; i++)
	{
		Particle p;

		p.age = 0;
		p.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); //red
		p.position = emitterPosition;
		p.size = 1.0f;
		p.age = 0;
		p.StartVelocity = XMFLOAT3(0.0f, 1.0f, 0.0f);
		particles.emplace_back(p);
	}
}

void Emitter::UpdateParticles(float deltaTime)
{
	static int start = 0;
	//looping through the particel
	for (size_t i = 0; i < particles.size(); i++)
	{
		particles[i].age++;

	}
}
