#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;

//struct to define a particle
struct Particle
{
	float spawnTime;
	XMFLOAT3 startPosition;

	float rotationStart;
	XMFLOAT3 startVelocity;

	float rotationEnd;
	XMFLOAT3 padding;
};

//single vertex of a particle
//each particle needs four vertcies to make a quad
struct ParticleVertex 
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT4 color;
};