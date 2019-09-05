#include "Textures.h"

Textures::Textures(std::string diffuse, std::string normal, std::string specular)
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
}
