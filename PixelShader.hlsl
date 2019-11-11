
// Struct representing the data we expect to receive from earlier pipeline stages
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD; //uv coordinates
	//float4 color		: COLOR;
};

//struct to desctibe the directional light
struct DirectionalLight
{
	float4 ambientColor;
	float4 diffuse;
	float4 specularity;
	float3 direction;
};


//constant buffer to hold light data
cbuffer LightData: register(b0)
{
	DirectionalLight light;
	DirectionalLight light2;
	float3 cameraPosition;
};

//texture variables
Texture2D diffuseTexture: register(t0);
SamplerState basicSampler: register(s0);
SamplerComparisonState shadowSampler: register(s1);
Texture2D normalMap : register(t1);
TextureCube cubeMap: register(t2);
Texture2D shadowMap	: register(t3);
Texture1D celShading: register(t4);

//function that accepts light and normal and then calculates the final color
float4 CalculateLight(DirectionalLight light, float3 normal, VertexToPixel input)
{
	//standard N dot L calculation for the light
	float3 L = -light.direction;
	L = normalize(L); //normalizing the negated direction
	float3 N = normal;
	N = normalize(N); //normalizing the normal
	float3 R = reflect(-L,N); //reflect R over N
	float3 V = normalize(cameraPosition - input.worldPosition); //view vector
	float4 NdotV = saturate(dot(N, V));
	float4 rimColor = float4(0.0f, 0.0f, 1.0f, 1.0f);

	//calculate the cosine of the angle to calculate specularity
	//I am calculating the light based on the phong reflection model
	float cosine = dot(R, V);
	cosine = saturate(cosine);
	float shininess = 64.f;
	float specularAmount = pow(cosine, shininess); //increase the cosine curve fall off

	float NdotL = dot(N, L);
	NdotL = saturate(NdotL); //this is the light amount, we need to clamp it to 0 and 1.0

	float4 diffuse = celShading.Sample(basicSampler, NdotL);
	//return diffuse;

	//adding diffuse, ambient, and specular color
	float4 finalLight = light.diffuse * NdotL;
	finalLight += specularAmount;

	return finalLight;
}

//calculate shadows
float ShadowCalculation(DirectionalLight light,float2 shadowUV,float lightDepth)
{

	//getting the width and height of shadow map
	float w;
	float h;
	float shadowDepth = 0.0f;

	shadowMap.GetDimensions(w, h);

	
	for (float j = -1.0f; j <= 1.0f; j++)
	{
		for (float i = -1.0f; i <= 1.0f; i++)
		{
			float2 offset = float2(i / (float)w, j / (float)h);
			shadowDepth += shadowMap.SampleCmpLevelZero(shadowSampler, offset+shadowUV, lightDepth);
		}
	}

	return (shadowDepth / 9.0f);
}

// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{

	float3 normalColor = normalMap.Sample(basicSampler,input.uv).xyz;
	float3 unpackedNormal = normalColor * 2.0f - 1.0f;

	//orthonormalizing T, B and N using the gram-schimdt process
	float3 N = normalize(input.normal);
	float3 T = input.tangent - dot(input.tangent, N) * N;
	T = normalize(T);
	float3 B = normalize(cross(T,N));

	float3x3 TBN = float3x3(T, B, N); //getting the tbn matrix

	//transforming normal from map to world space
	float3 finalNormal = mul(unpackedNormal, TBN);

	//calculating cubemap reflections
	float3 I = input.worldPosition - cameraPosition;
	I = normalize(I); //incident ray
	float3 reflected = reflect(I, finalNormal);

	float4 reflectedColor = cubeMap.Sample(basicSampler, reflected);

	float4 surfaceColor = diffuseTexture.Sample(basicSampler,input.uv);

	//doing calculations from shadow map to see if it is a shadow or not
	float lightDepth = input.lightPos.z / input.lightPos.w;

	//adjust the -1 to 1 range to 0 to 1
	float2 shadowUV = input.lightPos.xy / input.lightPos.w * 0.5f + 0.5f;

	//flip the y coordinates
	shadowUV.y = 1.0f - shadowUV.y;

	//read shadow map for closest surface
	float shadowDepth = ShadowCalculation(light, shadowUV, lightDepth);

	float3 V = normalize(cameraPosition - input.worldPosition); //view vector
	float NdotV = saturate(dot(finalNormal, V));
	float4 rimColor = float4(0.0f, 0.0f, 1.0f, 1.0f);

	float4 finalColor = ((CalculateLight(light, finalNormal, input) * shadowDepth) + light.ambientColor) * surfaceColor;

	//return shadowDepth;
	return ((CalculateLight(light,input.normal,input)*shadowDepth)+light.ambientColor)*surfaceColor;
}