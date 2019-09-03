#include "Material.h"

Material::Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader)
{
	//setting the shaders
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;
}

SimplePixelShader* Material::GetPixelShader()
{
	return pixelShader;
}

SimpleVertexShader* Material::GetVertexShader()
{
	return vertexShader;
}
