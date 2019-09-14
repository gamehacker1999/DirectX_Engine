
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;	// XYZW position (System Value Position)
	float4 lightPos		: TEXCOORD1;
	float3 normal		: NORMAL;		//normal of the vertex
	float3 worldPosition: POSITION; //position of vertex in world space
	float3 tangent		: TANGENT;	//tangent of the vertex
	float2 uv			: TEXCOORD;
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


//function for the fresnel term(Schlick approximation)
float3 Fresnel(float3 h, float3 v, float3 f0)
{
	//calculating v.h
	float VdotH = saturate(dot(v, h));
	//raising it to fifth power
	float VdotH5 = pow(1 - VdotH, 5);

	float3 finalValue = f0 + (1 - f0) * VdotH5;

	return finalValue;
}

//function for the Geometric shadowing
// k is remapped to a / 2 (a is roughness^2)
// roughness remapped to (r+1)/2
float GeometricShadowing(
	float3 n, float3 v, float3 h, float roughness)
{
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}


//function for the GGX normal distribution of microfacets
float SpecularDistribution(float roughness, float3 h, float3 n)
{
	//remapping the roughness
	float a = pow(roughness, 2);
	float a2 = a * a;

	float PI = 3.14159265f;

	float NdotHSquared = saturate(dot(n, h));
	NdotHSquared *= NdotHSquared;

	float denom = NdotHSquared * (a2 - 1) + 1;
	denom *= denom;
	denom *= PI;

	return a2 / denom;

}

//function that calculates the cook torrence brdf
float3 CookTorrence(float3 n, float3 h, float roughness, float3 v, float3 f0, float3 l)
{
	float D = SpecularDistribution(roughness,h,n);
	float3 F = Fresnel(h, v, f0);
	float G = GeometricShadowing(n,v,h,roughness) * GeometricShadowing(n,l,h,roughness);

	return (D*F*G) / (4 * dot(n,v)*dot(n,l));
}

float4 CalculateDiffuse(float3 n, float3 l, float4 diffuseColor, DirectionalLight light)
{
	float3 L = l;
	L = normalize(L); //normalizing the negated direction
	float3 N = n;
	N = normalize(N); //normalizing the normal

	float NdotL = dot(N, L);
	NdotL = saturate(NdotL); //this is the light amount, we need to clamp it to 0 and 1.0

	//adding diffuse, ambient color
	float4 finalLight = light.diffuse * NdotL;
	//finalLight += light.ambientColor;
	return finalLight;
}

//function to calculate diffuse color based on energy conservation	
float3 DiffuseEnergyConserve(
	float3 diffuse, float3 specular, float3 metalness)
{
	return diffuse *((1 - saturate(specular)) * (1 - metalness));
}

//textures and basic sampler
Texture2D diffuseTexture: register(t0);
Texture2D normalMap: register(t1);
Texture2D roughnessMap: register(t2);
Texture2D metalnessMap: register(t3);
TextureCube cubeMap: register(t4);
SamplerState basicSampler: register(s0);

float4 CalculateEnvironmentReflection(float3 normal, float3 position)
{
	float3 I = position - cameraPosition;
	I = normalize(I); //incident ray
	float3 reflected = reflect(I, normal);

	float4 reflectedColor = cubeMap.Sample(basicSampler, reflected);

	return reflectedColor;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	//sampling the diffuse texture
	float4 surfaceColor = diffuseTexture.Sample(basicSampler,input.uv);
	
	//getting the normal texture
	float3 normalColor = normalMap.Sample(basicSampler, input.uv).xyz;
	float3 unpackedNormal = normalColor * 2.0f - 1.0f;

	//orthonormalizing T, B and N using the gram-schimdt process
	float3 N = normalize(input.normal);
	float3 T = input.tangent - dot(input.tangent, N) * N;
	T = normalize(T);
	float3 B = normalize(cross(N, T));

	float3x3 TBN = float3x3(T, B, N); //getting the tbn matrix

	//transforming normal from map to world space
	float3 finalNormal = mul(unpackedNormal, TBN);

	//getting the metalness of the pixel
	float3 metalColor = metalnessMap.Sample(basicSampler, input.uv).xyz;

	float3 f0 = float3(0.04f, 0.04f, 0.04f);
	f0 = lerp(f0, surfaceColor.xyz, metalColor);

	//getting the roughness of pixel
	float roughness = roughnessMap.Sample(basicSampler, input.uv).x;

	float3 L = -light.direction;
	L = normalize(L); //normalizing the negated direction
	N = finalNormal;
	N = normalize(N); //normalizing the normal
	float3 R = reflect(-L, N); //reflect R over N
	float3 V = normalize(cameraPosition - input.worldPosition); //view vector
	float3 H = normalize(L+V/2);

	float4 diffuse = CalculateDiffuse(N, L, light.diffuse,light);
	float3 specular = CookTorrence( N, H, roughness, V, f0, L);

	float3 finalDiffuse=DiffuseEnergyConserve(diffuse.xyz, specular, metalColor);

	float4 skyboxReflection = CalculateEnvironmentReflection(finalNormal, input.worldPosition);

	float4 finalColor = float4(finalDiffuse + specular+light.ambientColor.xyz,1.0f)*surfaceColor;

	return finalColor;
}