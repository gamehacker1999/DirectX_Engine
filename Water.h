#pragma once
#include"Mesh.h"
#include<d3d11.h>
#include<DirectXMath.h>
#include"SimpleShader.h"
#include"Camera.h"
#include "Lights.h"
#include<limits.h>
#include<memory>

struct BitIndices
{
	int index;
	XMFLOAT3 padding;
};

using namespace DirectX;
class Water
{
	//water variables
	std::shared_ptr<Mesh> waterMesh;
	ID3D11ShaderResourceView* waterTex;
	ID3D11ShaderResourceView* waterNormal1;
	ID3D11ShaderResourceView* waterNormal2;

	SimplePixelShader* waterPS;
	SimpleVertexShader* waterVS;
	ID3D11SamplerState* samplerState;

	XMFLOAT4X4 worldMat;

	SimpleComputeShader* h0CS;
	SimpleComputeShader* htCS;
	SimpleComputeShader* twiddleFactorsCS;
	SimpleComputeShader* butterflyCS;
	SimpleComputeShader* inversionCS;
	SimpleComputeShader* sobelFilter;
	SimpleComputeShader* jacobianCS;

	ID3D11ShaderResourceView* noiseR1;
	ID3D11ShaderResourceView* noiseI1;
	ID3D11ShaderResourceView* noiseR2;
	ID3D11ShaderResourceView* noiseI2;

	ID3D11ShaderResourceView* h0SRV;
	ID3D11ShaderResourceView* h0MinusSRV;
	ID3D11UnorderedAccessView* h0URV;
	ID3D11UnorderedAccessView* h0MinusURV;

	ID3D11ShaderResourceView*  htxSRV;
	ID3D11UnorderedAccessView* htxUAV;

	ID3D11ShaderResourceView*  htySRV;
	ID3D11UnorderedAccessView* htyUAV;

	ID3D11ShaderResourceView*  htzSRV;
	ID3D11UnorderedAccessView* htzUAV;

	ID3D11ShaderResourceView* twiddleSRV;
	ID3D11UnorderedAccessView* twiddleUAV;

	ID3D11ShaderResourceView* pingpong0SRV;
	ID3D11UnorderedAccessView* pingpong0UAV;

	ID3D11ShaderResourceView* dySRV;
	ID3D11UnorderedAccessView* dyUAV;

	ID3D11ShaderResourceView* dxSRV;
	ID3D11UnorderedAccessView* dxUAV;

	ID3D11ShaderResourceView* dzSRV;
	ID3D11UnorderedAccessView* dzUAV;

	ID3D11ShaderResourceView* normalMapSRV;
	ID3D11UnorderedAccessView* normalMapUAV;

	ID3D11ShaderResourceView* foldingMapSRV;
	ID3D11UnorderedAccessView* foldingMapUAV;

	int texSize;



public:
	Water(std::shared_ptr<Mesh> waterMesh, ID3D11ShaderResourceView* waterTex,
		ID3D11ShaderResourceView* waterNormal1, ID3D11ShaderResourceView* waterNormal2,
		SimplePixelShader* waterPS, SimpleVertexShader* waterVS, 
		SimpleComputeShader* h0CS, SimpleComputeShader* htCS, SimpleComputeShader* twiddleFactorsCS,
		SimpleComputeShader* butterflyCS, SimpleComputeShader* inversionCS, SimpleComputeShader* sobelFilter,
		SimpleComputeShader* jacobianCS,ID3D11SamplerState* samplerState,
		ID3D11Device* device, ID3D11ShaderResourceView* noiseR1, ID3D11ShaderResourceView* noiseI1,
		ID3D11ShaderResourceView* noiseR2, ID3D11ShaderResourceView* noiseI2);
	~Water();

	void Update(float deltaTime, XMFLOAT3 shipPos);

	void Draw(Light lights, ID3D11ShaderResourceView* cubeMap, std::shared_ptr<Camera> camera,
		ID3D11DeviceContext* context, float deltaTime, float totalTime,ID3D11SamplerState* waterSampler);

	void CreateH0Texture();
	void CreateHtTexture(float totalTime);
	int CreateBitReversedIndices(int num, int d);
	void CreateTwiddleIndices();
	void RenderFFT(float totalTime);

};

