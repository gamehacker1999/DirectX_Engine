#include "Material.h"

Material::Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader)
{
	//setting the shaders
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;
}

Material::~Material()
{
	if (textureSRV)
		textureSRV->Release();
	if (samplerState)
		samplerState->Release();
	if (normalTextureSRV)
		normalTextureSRV->Release();
}

Material::Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader, 
	ID3D11SamplerState* samplerState, ID3D11ShaderResourceView* textureSRV,ID3D11ShaderResourceView* normalTextureSRV)
{
	//setting the shaders
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;

	//setting samplerstate and resource view
	this->textureSRV = textureSRV;
	this->samplerState = samplerState;

	this->normalTextureSRV = normalTextureSRV;
}

SimplePixelShader* Material::GetPixelShader()
{
	return pixelShader;
}

SimpleVertexShader* Material::GetVertexShader()
{
	return vertexShader;
}

ID3D11SamplerState* Material::GetSamplerState()
{
	return samplerState;
}

ID3D11ShaderResourceView* Material::GetTextureSRV()
{
	return textureSRV;
}

ID3D11ShaderResourceView* Material::GetNormalTextureSRV()
{
	return normalTextureSRV;
}
