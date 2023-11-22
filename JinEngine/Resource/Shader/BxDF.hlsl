#pragma once
#include"Math.hlsl" 

#if defined(USE_BRDF_GGX_NDF) && defined(USE_BRDF_SMITH_MASKING) && defined(USE_BRDF_ISOTROPY_NDF)
#define USE_GGX_SMITH_APPROXIMATION
#endif

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
#if 1
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
#else
	const float dotVH = dot(viewVec, halfVec);
	const float nlFactor = 1.0f - dotNL;
	const float nvFactor = 1.0f - dotNV;
	
	float fd90 = 0.5 + 2 * dotVH * dotVH * roughness;
	float FdL = 1 + (fd90 - 1) * (nlFactor * nlFactor * nlFactor * nlFactor * nlFactor);
	float FdV = 1 + (fd90 - 1) * (nvFactor * nvFactor * nvFactor * nvFactor * nvFactor);
	return diffuseColor * ((1 / PI) * FdV * FdL);
#endif
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
