#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The position of the vertex
	//DirectX::XMFLOAT4 Color;        // The color of the vertex
	DirectX::XMFLOAT3 normal; //normal of the current vertex
	DirectX::XMFLOAT3 tangent;
	DirectX::XMFLOAT2 uv; //texture coordinates of the vertex in u v space
};