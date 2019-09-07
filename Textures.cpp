#include "Textures.h"

Textures::Textures(std::string diffuse, std::string normal, std::string specular, std::string metal, std::string roughness)
{
}

Textures::~Textures()
{
	if (diffuseTexture)
		diffuseTexture->Release();
	if (specularTexture)
		specularTexture->Release();
	if (normalTexture)
		normalTexture->Release();
	if (metalTexture)
		metalTexture->Release();
	if (roughnessTexture)
		roughnessTexture->Release();
	if (basicSampler)
		basicSampler->Release();
}
