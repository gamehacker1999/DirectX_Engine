// Input control point
struct HullShaderInput
{
	float4 position		: WORLDPOS;	
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
	float2 motion		: TEXCOORD2;
	float2 heightUV		: TEXCOORD3;
	noperspective float2 screenUV		: VPOS;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 position		: WORLDPOS;
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
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

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<HullShaderInput, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor = 30000; 

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
