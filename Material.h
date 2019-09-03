#pragma once
#include<d3d11.h>
#include"SimpleShader.h"
#include<DirectXMath.h>
#include<memory>
class Material
{
	//pointers for pixel and vertex shader	
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

public:
	//constructor
	Material(SimpleVertexShader* vertexShader,SimplePixelShader* pixelShader);

	//getters for shaders
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();
};

