// Input control point
struct HullShaderInput
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

cbuffer hsExternData: register(b0)
{
	matrix world;
	float3 cameraPos;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

float CalcLOD(float3 a, float3 b)
{
	float dist = distance(a, b);
	float3 center = (a + b) / 2;
	float camDist = distance(cameraPos, center);
	return dist * 5 / camDist;
}

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<HullShaderInput, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	float3 center = ip[0].position + ip[1].position + ip[2].position;
	center /= 3;
	
	float3 centerWorld = mul(float4(center, 1.0f), world).xyz;
	
	float d = distance(centerWorld, cameraPos);
	
	float d0 = 30;
	float d1 = 1000;
	
	float tess = 64.0f * saturate((d1 - d) / (d1 - d0))+1;
	
	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor = tess; 

	//float3 v0 = mul(float4(ip[0].position, 1.0f), world).xyz;
	//float3 v1 = mul(float4(ip[1].position, 1.0f), world).xyz;
	//float3 v2 = mul(float4(ip[2].position, 1.0f), world).xyz;
	//
	//Output.EdgeTessFactor[0] = CalcLOD(v2, v0);
	//Output.EdgeTessFactor[1] = CalcLOD(v0, v1);
	//Output.EdgeTessFactor[2] = CalcLOD(v1, v2);
	//Output.InsideTessFactor = (Output.EdgeTessFactor[0] + Output.EdgeTessFactor[1] + Output.EdgeTessFactor[2]) / 3;
	//
	//
	//
	//float dist;
	//float3 midPoint;
	//
	//midPoint = (ip[2].position + ip[0].position) / 2.0f;
	//dist = distance(midPoint, cameraPos) * 0.6;

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]

HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<HullShaderInput, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.position =      ip[i].position;
	Output.lightPos = ip[i].lightPos;
	Output.normal = ip[i].normal;
	Output.worldPosition = ip[i].worldPosition;
	Output.tangent = ip[i].tangent;
	Output.uv = ip[i].uv;
	Output.motion = ip[i].motion;
	Output.heightUV = ip[i].heightUV;
	Output.screenUV =      ip[i].screenUV;


	return Output;
}
