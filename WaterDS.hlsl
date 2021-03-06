struct DS_OUTPUT
{
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
	float2 motion		: TEXCOORD2;
	float2 heightUV		: TEXCOORD3;
	noperspective float2 screenUV		: VPOS;
};

cbuffer dsExternData: register (b0)
{
	matrix view;
	matrix projection;
	matrix world;
	float3 cameraPos;
	float2 windDir;
	float motion;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 position		: POSITION;
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION1; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
	float2 motion		: TEXCOORD2;
	float2 heightUV		: TEXCOORD3;
	noperspective float2 screenUV		: VPOS;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

Texture2D heightMap: register(t0);
Texture2D heightMapX: register(t1);
Texture2D heightMapZ: register(t2);

SamplerState sampleOptions: register(s0);

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;


	float3 pos = 
		patch[0].position*domain.x+patch[1].position*domain.y+patch[2].position*domain.z;


	Output.normal = float3(
		patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z);
	Output.uv = float2(
		patch[0].uv * domain.x + patch[1].uv * domain.y + patch[2].uv * domain.z);

	Output.worldPosition = float3(
		patch[0].worldPosition * domain.x + patch[1].worldPosition * domain.y + patch[2].worldPosition * domain.z);

	float height =  heightMap.SampleLevel (sampleOptions, Output.uv+ (40 * float2(1, 1)), 0).r* 0.02    ;//max(0,  -distance(Output.worldPosition, cameraPos)/  10000.5);
	float heightX = heightMapX.SampleLevel(sampleOptions, Output.uv+ (40 * float2(1, 1)), 0).r*1.8* 0.02;//max(0,  -distance(Output.worldPosition, cameraPos) / 10000.5);
	float heightZ = heightMapZ.SampleLevel(sampleOptions, Output.uv+ (40 * float2(1, 1)), 0).r*1.8* 0.02;//max(0,  -distance(Output.worldPosition, cameraPos) / 10000.5);

	pos.y  = height;
	pos.x -= heightX;
	pos.z -= heightZ;

	//wPos *= -Output.normal;
	float4 wPos = mul(float4(pos, 1.0f), world);
	matrix viewProj = mul(view, projection);

	float4 csPos = mul(wPos, viewProj);

	Output.position = csPos;

	Output.lightPos = float4(
		patch[0].lightPos * domain.x + patch[1].lightPos * domain.y + patch[2].lightPos * domain.z);
	Output.tangent = float3(
		patch[0].tangent * domain.x + patch[1].tangent * domain.y + patch[2].tangent * domain.z);
	Output.motion = float2(
		patch[0].motion * domain.x + patch[1].motion * domain.y + patch[2].motion * domain.z);
	Output.heightUV = float2(
		patch[0].heightUV * domain.x + patch[1].heightUV * domain.y + patch[2].heightUV * domain.z);

	Output.screenUV =	Output.position.xy / Output.position.w;
	Output.screenUV.x = Output.screenUV.x * 0.5 + 0.5;
	Output.screenUV.y = -Output.screenUV.y * 0.5 + 0.5;

	//Output.screenUV = float2(
		//patch[0].screenUV * domain.x + patch[1].screenUV * domain.y + patch[2].screenUV * domain.z);

	return Output;
}
