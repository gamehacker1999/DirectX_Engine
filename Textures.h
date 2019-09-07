#pragma once
#include<d3d11.h>
#include<WICTextureLoader.h>
#include<DirectXMath.h>
#include<string>
#include<vector>

using namespace DirectX;

//class to hold the specular, diffuse, and normal maps
class Textures
{
	ID3D11ShaderResourceView* diffuseTexture;
	ID3D11ShaderResourceView* specularTexture;
	ID3D11ShaderResourceView* normalTexture;
	ID3D11ShaderResourceView* metalTexture;
	ID3D11ShaderResourceView* roughnessTexture;
	ID3D11SamplerState* basicSampler;

public:

	Textures(std::string diffuse = "default",std::string normal = "default", std::string specular = "default",
		std::string metal = "default",std::string roughness = "default");
	~Textures();

};

