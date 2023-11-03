#include"Math.hlsl"

#if !defined(CSM_MAX_COUNT)
#define CSM_MAX_COUNT 8
#endif
 
#if !defined(CUBE_MAP_FACE_COUNT)
#define CUBE_MAP_FACE_COUNT 6
#endif

#define DIRECTONAL_LIGHT_NON_SHADOW_MAP -1
#define DIRECTONAL_LIGHT_HAS_NORMAL_SHADOW_MAP 0
#define DIRECTONAL_LIGHT_HAS_CSM 1

#if defined(USE_BRDF_GGX_NDF) && defined(USE_BRDF_SMITH_MASKING)&& defined(USE_BRDF_ISOTROPY_NDF) 	 
#define USE_GGX_SMITH_APPROXIMATION
#endif
//272byte
struct DirectionalLightData
{ 
	float4x4 view;
	float4x4 viewProj;
    float4x4 shadowMapTransform; 
    float3 color; 
    int shadowMapIndex;
    float3 direction;  
    int shadowMapType;  //-1 not, 0 normal, 1 csm, 2 cube
	float2 frustumSize;
	float frustumNear;
	float frustumFar;
	float penumbraScale;
	float penumbraBlockerScale; 
    int csmDataIndex;  //data start index
	float shadowMapSize;
	float shadowMapInvSize;
	float tanAngle; 
	float bias;
	uint dLightPad00;
};

//336byte
struct CsmData
{ 
	float4 scale[CSM_MAX_COUNT]; // (projM * textureM) scale
	float4 posOffset[CSM_MAX_COUNT]; // (projM * textureM) translate
	//float2 frustumSize[CSM_MAX_COUNT];
	float frustumNear[CSM_MAX_COUNT];
	float frustumFar[CSM_MAX_COUNT];
    float mapMinBorder;     //default value is 0
    float mapMaxBorder;     //default value is 1
	float levelBlendRate;
    uint count; 
};
//448byte
struct PointLightData
{
	float4x4 shadowMapTransform[CUBE_MAP_FACE_COUNT];
    float3 color;
    float power; 
    float3 position;
	float frustumNear;
	float frustumFar;
    float radius;       
	float penumbraScale;
	float penumbraBlockerScale;
	float shadowMapIndex;
	int hasShadowMap; 
	float shadowMapSize;
	float shadowMapInvSize; 
};
//144byte
struct SpotLightData
{
    float4x4 shadowMapTransform;
    float3 color;
	float power;
    float3 position;
	float frustumNear;
	float3 direction;
	float frustumFar;
	float innerConeAngle;
	float outerConeAngle;  
	float penumbraScale;
	float penumbraBlockerScale;
    int shadowMapIndex;
    uint hasShadowMap; 
	float shadowMapSize;
	float shadowMapInvSize;
};
struct Material
{
    float4 albedoColor;
    float metalic;
    float roughness;
};

float CalLinearAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}
float CalInverseAttenuation(float d, float range)
{
	return 1.0f - saturate(pow(d / range, 2.0f));
}
float CalFactorAttenuation(float d, float constantsF, float linearF, float quadraticF)
{
	return saturate(1.0f / (constantsF + linearF * d + quadraticF * (d * d)));
} 
//BRDF--------------------------------------------------------------------
//distribution and masking
//real time renderin 4 468p
float SmithMasking(const float dotHV, const float dotHL, const float aV, const float aL)
{
	//(saturate(dot(halfVec, viewVec)) * saturate(dot(halfVec, lightVec))) / (1 + aV + aL);
	// 1.0f / (1.0f + aV + aL);
	return (saturate(dotHV) * saturate(dotHL)) / (1.0f + aV + aL);
}
//https://inst.eecs.berkeley.edu//~cs283/sp13/lectures/cookpaper.pdf
float TorranceMasking(const float3 normal, const float3 lightVec, const float3 viewVec, const float3 halfVec)
{
	const float dotNH = dot(normal, halfVec);
	const float dotNV = dot(normal, viewVec);
	const float dotNL = dot(normal, lightVec);
	const float dotVH = dot(viewVec, halfVec); 
	return min(1.0f, min((2.0f * dotNH * dotNV) / dotVH, (2.0f * dotNH * dotNL) / dotVH));
} 
float BeckmannINDF(const float3 normal, const float3 halfVec, const float roughness)
{
	//used by cook-torrance brdf
	const float dotNH = dot(normal, halfVec);
	const float dotNH2 = dotNH * dotNH;
	
	const float theta = acos(dotNH);
	const float tanNH = tan(theta);
	const float r2 = roughness * roughness; 
	
	return (saturate(dotNH) * exp(-(tanNH * tanNH) / r2)) / (PI * r2 * dotNH2 * dotNH2);
} 
float BeckmannANDF(const float3 normal, const float3 tangent, const float3 binormal, const float3 halfVec, const float roughness, const float kAniso)
{
	const float dotNH = dot(normal, halfVec);
	const float dotTH = dot(tangent, halfVec);
	const float dotBH = dot(binormal, halfVec);
	const float dotNH2 = dotNH * dotNH; 
	const float r2 = roughness * roughness;
	
	const float rx = r2 * (1.0f + kAniso);
	const float ry = r2 * (1.0f - kAniso);

	const float xFactor = (dotTH * dotTH) / (rx * rx);
	const float yFactor = (dotBH * dotBH) / (ry * ry);

	return (saturate(dotNH) * exp(-(xFactor + yFactor) / dotNH2)) / (PI * rx * ry * (dotNH2 * dotNH2));
}
float BeckmannLambda(const float3 normal, const float3 dir, const float roughness)
{
	const float dotND = dot(normal, dir);
	const float a = dotND / (roughness * sqrt((1 - dotND * dotND)));
	
	//use approximation lambda
	if (a >= 1.6f)
		return 0.0f;
	else
		return (1.0f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}
float BlinnPhongINDF(const float3 normal, const float3 halfVec, const float ap)
{
	const float dotNH = dot(normal, halfVec);
	return saturate(dotNH) * ((ap + 2) / (2 * PI)) * pow(dotNH, ap);
}
float BlinnPhongLambda(const float3 normal, const float3 dir, const float ap)
{
	const float dotND = dot(normal, dir);
	const float a = sqrt(0.5f * ap + 1.0f) / (sqrt(1.0f - dotND * dotND) / dotND);

	//use approximation lambda
	if (a >= 1.6f)
		return 0.0f;
	else
		return (1.0f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}
float BlinnPhongRoughness(const float roughness)
{
	return pow(8192.0f, roughness);
}
float GGXINDF(const float3 normal, const float3 halfVec, const float roughness)
{
	const float dotNH = dot(normal, halfVec);
	const float ag = roughness * roughness * roughness * roughness;
	const float b = 1.0f + (dotNH * dotNH) * (ag - 1.0f);
	return (saturate(dotNH) * ag) / (PI * b * b);
}
float GGXANDF(const float3 normal, const float3 tangent, const float3 binormal, const float3 halfVec, const float roughness, const float kAniso)
{
	const float dotNH = dot(normal, halfVec);
	const float dotTH = dot(tangent, halfVec);
	const float dotBH = dot(binormal, halfVec); 
	const float r2 = roughness * roughness;
	
	const float rx = r2 * (1.0f + kAniso);
	const float ry = r2 * (1.0f - kAniso);
	
	const float xFactor = (dotTH * dotTH) / (rx * rx);
	const float yFactor = (dotBH * dotBH) / (ry * ry);
	
	return saturate(dotNH) / (PI * rx * ry * (xFactor + yFactor + dotNH * dotNH));
}
float GGXLambda(const float3 normal, const float3 dir, const float roughness)
{
	const float dotND = dot(normal, dir);
	const float a = dotND / (roughness * roughness * sqrt((1.0f - dotND * dotND)));
	return (-1.0f + sqrt(1.0f + 1.0f / (a * a))) / 2.0f;
}
float GGXSmith(const float3 normal, const float3 lightVec, const float3 viewVec, const float roughness)
{
	//Approximate
	const float aDotNL = abs(dot(normal, lightVec));
	const float aDotNV = abs(dot(normal, viewVec));
	
	return 0.5f / lerp(2.0f * aDotNL * aDotNV, aDotNL + aDotNV, roughness * roughness);
} 
//Fresnel
float3 SchlickFresnel(const float3 albedoColor, const float3 normal, const float3 lightVec, const float metalic)
{
	const float3 f0 = albedoColor * metalic;
	const float rFactor = 1.0f - saturate(dot(normal, lightVec));
	return f0 + (float3(1.0f, 1.0f, 1.0f) - f0) * (rFactor * rFactor * rFactor * rFactor * rFactor);
}
float3 SchlickFresnel(const float3 albedoColor, const float3 normal, const float3 lightVec, const float metalic, out float3 f0)
{
	f0 = albedoColor * metalic;
	const float rFactor = 1.0f - saturate(dot(normal, lightVec));
	return f0 + (float3(1.0f, 1.0f, 1.0f) - f0) * (rFactor * rFactor * rFactor * rFactor * rFactor);
}
float3 SchlickFresnel(const float3 albedoColor, const float3 normal, const float3 lightVec, const float metalic, out float3 f0, out float3 diffuseColor)
{
	f0 = albedoColor * metalic;
	diffuseColor = albedoColor * (1.0f - metalic);
	
	const float rFactor = 1.0f - saturate(dot(normal, lightVec));	
	return f0 + (float3(1.0f, 1.0f, 1.0f) - f0) * (rFactor * rFactor * rFactor * rFactor * rFactor);
}
//Soft diffuse
float3 LambertianIDiffuse(const float3 albedoColor, const float3 frenel)
{
	return (float3(1.0f, 1.0f, 1.0f) - frenel) * (albedoColor / PI);
}
//Schlick근사가 사용된다고 가정
//정반사율이 완벽한 프레넬 거울같은 반사율 표면에마 적용
float3 ShirelyDiffuse(const float3 albedoColor, const float3 f0, const float3 normal, const float3 lightVec, const float3 viewVec)
{
	const float nl = (1.0f - saturate(dot(normal, lightVec)));
	const float nV = (1.0f - saturate(dot(normal, viewVec)));
	return (21.0f / (20.0f * PI)) * (float3(1.0f, 1.0f, 1.0f) - f0) * albedoColor * (1.0f - (nl * nl * nl * nl * nl)) * (1.0f - (nV * nV * nV * nV * nV));
} 
//Hard diffuse---------
float3 DisneyDiffuse(const float3 diffuseColor,
const float3 normal,
const float3 lightVec,
const float3 viewVec,
const float3 halfVec,
const float roughness,
const float kss)
{
	const float dotNL = dot(normal, lightVec);
	const float dotNV = dot(normal, viewVec);
	const float dotNH = dot(normal, halfVec);
	const float dotHL = dot(halfVec, lightVec);
	const float dotHL2 = dotHL * dotHL;
	
	const float nlFactor = 1.0f - dotNL;
	const float nvFactor = 1.0f - dotNV;
	
	const float nlFactor5 = nlFactor * nlFactor * nlFactor * nlFactor * nlFactor;
	const float nvFactor5 = nvFactor * nvFactor * nvFactor * nvFactor * nvFactor;
	
	const float r2 = roughness * roughness;
	
	const float Fd90 = 0.5f + 2 * sqrt(r2) * dotHL2;
	const float fd = (1.0f + (Fd90 - 1.0f) * nlFactor5) * (1.0f + (Fd90 - 1.0f) * nvFactor5);
	const float Fss90 = sqrt(r2) * dotHL2;
	const float Fss = (1.0f + (Fss90 - 1.0f) * nlFactor5) * (1.0f + (Fss90 - 1.0f) * nvFactor5);
	const float fss = (1.0f / (dotNL * dotNV) - 0.5f) * Fss + 0.5f;
	return saturate(dotNL) * saturate(dotNV) * (diffuseColor / PI) * ((1.0f - kss) * fd + 1.25f * kss * fss);
}
float3 HammonDiffuse(const float3 diffuseColor, 
const float3 f0, 
const float3 normal, 
const float3 lightVec, 
const float3 viewVec, 
const float3 halfVec,
const float roughness)
{
	const float dotNL = dot(normal, lightVec);
	const float dotNV = dot(normal, viewVec);
	const float dotNH = dot(normal, halfVec);
	
	const float nlFactor = 1.0f - dotNL;
	const float nvFactor = 1.0f - dotNV;
	
	const float nlFactor5 = nlFactor * nlFactor * nlFactor * nlFactor * nlFactor;
	const float nvFactor5 = nvFactor * nvFactor * nvFactor * nvFactor * nvFactor;
	const float r2 = roughness * roughness;
	
	const float3 fSmooth = 1.05f * (float3(1.0f, 1.0f, 1.0f) - f0) * (1.0f - nlFactor5) * (1.0f - nvFactor5);
	const float fMulti = 0.3641f * r2;
	const float facing = 0.5f + 0.5f * dot(lightVec, viewVec);
	const float fRough = facing * (0.9f - 0.4f * facing) * ((0.5f + dotNH) / dotNH);
	return saturate(dotNL) * saturate(dotNV) * (diffuseColor / PI) * ((1 - r2) * fSmooth + r2 * fRough + diffuseColor * fMulti);
}  

float3 CalBRDF(const float3 normal, float3 tangent, const float3 lightVec, const float3 viewVec, const Material mat)
{
	const float3 halfVec = normalize(viewVec + lightVec);
	
	float3 f0;
	const float3 f = SchlickFresnel(mat.albedoColor.xyz, halfVec, lightVec, mat.metalic, f0);
#ifdef USE_BRDF_DISNEY_DIFFUSE
	const float3 diffColor = DisneyDiffuse(mat.albedoColor.xyz, normal, lightVec, viewVec, halfVec, mat.roughness, 0.75f);
#elif USE_BRDF_HAMMON_DIFFUSE
	const float3 diffColor = HammonDiffuse(mat.albedoColor.xyz, f0, normal, lightVec, viewVec, halfVec, mat.roughness);
#elif USE_BRDF_SHIRELY_DIFFUSE
	const float3 diffColor = ShirelyDiffuse(mat.albedoColor.xyz, f0, normal, lightVec, viewVec);
#elif USE_BRDF_LAMBERTIAN_DIFFUSE
	const float3 diffColor = LambertianIDiffuse(mat.albedoColor.xyz, f0);
#else 
	const float3 diffColor = float3(0, 0, 0);
#endif
	
#ifdef USE_GGX_SMITH_APPROXIMATION
	const float3 specColor = SchlickFresnel(mat.albedoColor.xyz, halfVec, lightVec, mat.metalic) * GGXINDF(normal, halfVec, mat.roughness) * GGXSmith(normal, lightVec, viewVec, mat.roughness);
	return diffColor + specColor;
#else
	const float dotNL = dot(normal, lightVec);
	const float dotNV = dot(normal, viewVec);
	const float dotHL = dot(halfVec, lightVec);
	const float dotHV = dot(halfVec, viewVec);
	
#ifdef USE_BRDF_ISOTROPY_NDF
#ifdef USE_BRDF_GGX_NDF
	const float ndf = GGXINDF(normal, halfVec, mat.roughness);
#elif USE_BRDF_BECKMANN_NDF
	const float ndf = BeckmannINDF(normal, halfVec, mat.roughness);
#elif USE_BRDF_BLINN_PHONG_NDF
	const float ndf = BlinnPhongINDF(normal, halfVec, mat.roughness);
#else
	const float ndf = 1.0f;
#endif
#else
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	float3 binormal = cross(normal, tangent);
#ifdef USE_BRDF_GGX_NDF
	const float ndf = GGXANDF(normal, tangent, binormal, halfVec, mat.roughness, 0.5f);
#elif USE_BRDF_BECKMANN_NDF 
	const float ndf = BeckmannANDF(normal, tangent, binormal, halfVec, mat.roughness, 0.5f); 
#else
	const float ndf = 1.0f;
#endif
#endif
	
#ifdef USE_BRDF_GGX_NDF
	const float aV = GGXLambda(normal, viewVec, mat.roughness);
	const float aL = GGXLambda(normal, lightVec, mat.roughness);
#elif USE_BRDF_BECKMANN_NDF
	const float aV = BeckmannLambda(normal, viewVec, mat.roughness);
	const float aL = BeckmannLambda(normal, lightVec, mat.roughness);
#elif USE_BRDF_BLINN_PHONG_NDF
	const float aV = BlinnPhongLambda(normal, viewVec, mat.roughness);
	const float aL = BlinnPhongLambda(normal, lightVec, mat.roughness);
#else
	const float aV = 0;
	const float aL = 0;
#endif

#ifdef USE_BRDF_SMITH_MASKING
	const float masking = SmithMasking(dotHV, dotHL, aV, aL);
#elif USE_BRDF_TORRANCE_MASKING
	const float masking = TorranceMasking(normal, lightVec, viewVec, halfVec);
#else
	const float masking = 1.0f;
#endif
		
	const float3 specColor = f * ndf * masking;
	return diffColor + (specColor / (4.0f * abs(dotNL) * abs(dotNV)));
#endif
}
//------------------------------------------------------------------------

float3 BlinnPhong(float3 lightColor, float3 lightVec, float3 normal, float3 viewVec, Material mat)
{
    const float m = mat.roughness * 256.0f;
	float3 halfVec = normalize(viewVec + lightVec);
        
    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
	float cosIncidentAngle = saturate(dot(halfVec, lightVec));

	float f0 = 1.0f - cosIncidentAngle;
	float3 fresnelFactor = lightColor + (1.0f - lightColor) * (f0 * f0 * f0 * f0 * f0);
	
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.albedoColor.rgb + specAlbedo) * lightColor;
} 
float3 ComputeDirectionalLight(DirectionalLightData light, Material mat, float3 normal, float3 tangent, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -light.direction;

    // Scale light down by Lambert's cosine law.
	float nDotL = max(dot(lightVec, normal), 0.0f);
	return light.color * nDotL * CalBRDF(normal, tangent, lightVec, toEye, mat);
}  
float3 ComputePointLight(PointLightData light, Material mat, float3 pos, float3 normal, float3 tangent, float3 toEye)
{ 
    float3 lightVec = light.position - pos;   
    float d = length(lightVec);     
    if (d > light.frustumFar)
        return 0.0f;
    
    lightVec /= d;    
    float nDotL = max(dot(lightVec, normal), 0.0f);
	float attFactor = CalInverseAttenuation(d, light.frustumFar);
    
	float3 lightColor = light.color * nDotL * light.power * attFactor;
	return lightColor * CalBRDF(normal, tangent, lightVec, toEye, mat);
} 
float3 ComputeSpotLight(SpotLightData light, Material mat, float3 pos, float3 normal, float3 tangent, float3 toEye)
{ 
    float3 lightVec = light.position - pos;   
    float d = length(lightVec);
	if (d > light.frustumFar)
        return 0.0f;
    
    lightVec /= d;
    
    float nDotL = max(dot(lightVec, normal), 0.0f);
	float dDotL = dot(-lightVec, light.direction);    
    
	float attFactor = CalInverseAttenuation(d, light.frustumFar);
	float theta = dDotL;
	float epsilon = light.innerConeAngle - light.outerConeAngle;
	float smoothFactor = clamp((theta - light.outerConeAngle) / epsilon, 0.0, 1.0);
    
	float3 lightColor = light.color * nDotL * light.power * attFactor * smoothFactor * smoothFactor; 
	return lightColor * CalBRDF(normal, tangent, lightVec, toEye, mat);
}


