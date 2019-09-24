#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;

//struct to define a particle
struct Particle
{
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT3 StartVelocity;
	float size;
	float age;
};

//single vertex of a particel
//each particle needs four vertcies to make a quad
struct ParticleVertex 
{
	XMFLOAT3 position;
	XMFLOAT2 texCoord;
	XMFLOAT4 color;
	float size;
};