#include "Material.h"

Material::Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader)
{
	//setting the shaders
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;
}

Material::~Material()
{
}

Material::Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader, 
	ID3D11SamplerState* samplerState, ID3D11ShaderResourceView* textureSRV,ID3D11ShaderResourceView* normalTextureSRV,
	ID3D11ShaderResourceView* roughnessTextureSRV, ID3D11ShaderResourceView* metalnessTextureSRV)
{
	//setting the shaders
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;

	//setting samplerstate and resource view
	this->textureSRV = textureSRV;
	this->samplerState = samplerState;

	this->normalTextureSRV = normalTextureSRV;
	this->roughnessTextureSRV = roughnessTextureSRV;
	this->metalnessTextureSRV = metalnessTextureSRV;
}

Material::Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader, std::shared_ptr<Textures> textures)
{
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->textures = textures;
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

ID3D11ShaderResourceView* Material::GetRoughnessSRV()
{
	return roughnessTextureSRV;
}

ID3D11ShaderResourceView* Material::GetMetalnessSRV()
{
	return metalnessTextureSRV;
}

void Material::SetPixelShaderData()
{
	//sending the sampler state and textures to pixel shader
	if (samplerState)
		pixelShader->SetSamplerState("basicSampler", samplerState);

	if(textureSRV)
		pixelShader->SetShaderResourceView("diffuseTexture", textureSRV);

	if(normalTextureSRV)
		pixelShader->SetShaderResourceView("normalMap", normalTextureSRV);

	if(roughnessTextureSRV)
		pixelShader->SetShaderResourceView("roughnessMap", roughnessTextureSRV);

	if(metalnessTextureSRV)
		pixelShader->SetShaderResourceView("metalnessMap", metalnessTextureSRV);

	pixelShader->CopyAllBufferData();

}
