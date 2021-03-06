
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 worldPos		: TEXCOORD;
};

cbuffer externalData: register(b0)
{
	float roughness;
}

static const float PI = 3.141592653f;

TextureCube skybox : register(t0);
SamplerState basicSampler: register(s0);
//function to generate the van der corpus sequence
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

//function to generate a hammersly low discrepency sequence for importance sampling
float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

//importance sampling
float3 ImportanceSamplingGGX(float2 xi, float3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0f * PI * xi.x;
	float cosTheta = sqrt((1.0f - xi.y) / (1.0f + (a * a - 1.f) * xi.y));
	float sinTheta = 1 - (cosTheta * cosTheta); //using the trigonometric rule

	//from spherica coordinats to cartsian space
	float3 H;
	H.x = sinTheta * cos(phi);
	H.y = sinTheta * sin(phi);
	H.z = cosTheta;

	//from tangent to world space using TBN matrix
	float3 up = abs(N.z) < 0.999 ? float3(0.0f, 0.0f, 1.0f) : float3(0.0f, 1.0f, 0.0f); //up vector is based on the value of N
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + H.z * N;

	return normalize(sampleVec);
}

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 N = normalize(input.worldPos);
	float3 R = N;
	float3 V = R;

	uint sampleCount = 4096;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0.0, 0.0, 0.0);

	for (uint i = 0; i < sampleCount; i++)
	{
		float2 xi = Hammersley(i, sampleCount);
		float3 H = ImportanceSamplingGGX(xi, N, roughness);
		float3 L = normalize(2 * dot(V, H) * H - V);

		float NdotL = saturate(dot(N, L));

		prefilteredColor += skybox.Sample(basicSampler, L).rgb * NdotL;
		totalWeight += NdotL;
	}

	prefilteredColor /= totalWeight;

	return float4(prefilteredColor, 1.0f);

}