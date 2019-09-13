#pragma once
#include<d3d11.h>
#include"SimpleShader.h"
#include<DirectXMath.h>
#include<memory>
#include"Textures.h"
#include<WICTextureLoader.h>
using namespace DirectX;
class Material
{
	//pointers for pixel and vertex shader	
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

	//variables for the textures
	ID3D11SamplerState* samplerState;
	ID3D11ShaderResourceView* textureSRV;
	ID3D11ShaderResourceView* normalTextureSRV;
	ID3D11ShaderResourceView* roughnessTextureSRV;
	ID3D11ShaderResourceView* metalnessTextureSRV;

	//textures in this material
	std::shared_ptr<Textures> textures;

public:
	//constructor
	Material(SimpleVertexShader* vertexShader,SimplePixelShader* pixelShader);

	~Material();

	//material for loading textures
	Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader,
		ID3D11SamplerState* samplerState, ID3D11ShaderResourceView* textureSRV,
		ID3D11ShaderResourceView* normalTextureSRV=nullptr, ID3D11ShaderResourceView* roughnessTextureSRV = nullptr,
		ID3D11ShaderResourceView* metalnessTextureSRV = nullptr);

	Material(SimpleVertexShader* vertexShader, SimplePixelShader* pixelShader, std::shared_ptr<Textures> textures);

	//getters for shaders and textures
	SimplePixelShader* GetPixelShader();
	SimpleVertexShader* GetVertexShader();

	ID3D11SamplerState* GetSamplerState();
	ID3D11ShaderResourceView* GetTextureSRV();
	ID3D11ShaderResourceView* GetNormalTextureSRV();
	ID3D11ShaderResourceView* GetRoughnessSRV();
	ID3D11ShaderResourceView* GetMetalnessSRV();

	//function to send texture to pixelshader
	void SetPixelShaderData();
};

