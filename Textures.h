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

public:

	Textures(std::string diffuse = "default",std::string normal = "default", std::string specular = "default");
	~Textures();

};

