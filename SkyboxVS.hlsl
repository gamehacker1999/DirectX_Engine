struct VertexShaderInput
{
	float3 position		: POSITION;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldPos		: POSITION;
};

//cbuffer for matrix position
cbuffer externalData: register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float3 cameraPos;
}

//entry point of the skybox vertex shader
VertexToPixel main(VertexShaderInput input) 
{
	VertexToPixel output; //this specifies the data that gets passed along the render pipeline

	//view projection matrix
	matrix viewProj = mul(view, projection);

	//calculating the vertex position
	output.position = mul(float4(cameraPos, 1.0), viewProj);

	//sending the world position to pixelshader
	output.worldPos = cameraPos;

	return output;
}