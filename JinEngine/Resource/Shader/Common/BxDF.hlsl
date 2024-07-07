/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"Math.hlsl" 
#include"Sampling.hlsl" 

#if defined(USE_BRDF_GGX_NDF) && defined(USE_BRDF_SMITH_MASKING) && defined(USE_BRDF_ISOTROPY_NDF)
#define USE_GGX_SMITH_APPROXIMATION
#endif
 
class BxDF
{
    float3 albedoColor;
    float3 f0;
    
    float3 normal;
    float3 lightVec;
    float3 viewVec;
    float3 halfVec;
        
    float dotNL;
    float dotNV;
    float dotNH;
    float dotHL;
    float dotHV;
    float dotVL;
    //dotND is dot(T or B or N, Direction)
    
    float roughness;
    float roughness2;
    float metalic;
       
#ifdef USE_BRDF_ANISOTROPY_NDF  
    float roughnessX;
    float roughnessY;
    float dotTH;
    float dotBH;
#endif
    
    float3 ComputeF0(float3 specularColor, float3 albedoColor)
    {
        return lerp(specularColor, albedoColor, metalic);
    } 
    void Initialize(const float3 _albedoColor, 
        const float _specularFactor, 
        const float3 _normal,
        const float3 _lightVec, 
        const float3 _viewVec, 
        const float _roughness, 
        const float _metalic,
        const float epsilon = EPSILON)
    {
        albedoColor = _albedoColor;
        normal = _normal;
        lightVec = _lightVec;
        viewVec = _viewVec;
        halfVec = normalize(viewVec + lightVec);
        dotNL = max(dot(normal, lightVec), epsilon);
        dotNV = max(dot(normal, viewVec), epsilon);
        dotNH = dot(normal, halfVec);
        dotHL = dot(halfVec, lightVec);
        
#if defined(USE_BRDF_DISNEY_DIFFUSE) || defined(USE_OREN_NAYAR_DIFFUSE)
        dotHV = dot(halfVec, viewVec);
#else
        dotHV = 0;
#endif
        
#ifdef USE_BRDF_HAMMON_DIFFUSE
        dotVL = dot(viewVec, lightVec);
#else
        dotVL = 0;
#endif
        roughness = max(_roughness, epsilon);
        roughness2 = roughness * roughness;
        metalic = _metalic;    
        
        f0 = ComputeF0(_specularFactor.xxx, albedoColor);
    }    
    void Initialize(const float3 _albedoColor,
        const float _specularFactor,
        const float3 _normal,
        const float3 _tangent,
        const float3 _lightVec,
        const float3 _viewVec,
        const float _roughness,
        const float _metalic,
        const float ansio,
        const float epsilon = EPSILON)
    { 
        Initialize(_albedoColor, _specularFactor, _normal, _lightVec, _viewVec, _roughness, _metalic, epsilon);    
#ifdef USE_BRDF_ANISOTROPY_NDF  
        roughnessX = roughness2 * (1.0f + ansio);
        roughnessY = roughness2 * (1.0f - ansio);
        dotTH = dot(_tangent, halfVec);
        dotBH = dot(CalBinormal(normal, _tangent), halfVec);
#endif
    }
    void DiffuseSamplingInitialize(const float3 _albedoColor,
        const float _specularFactor,
        const float3 _normal, 
        const float3 _tanget,
        const float3 _viewVec,
        const float _roughness,
        const float _metalic,
        const float2 uniformRate,
        const float epsilon = EPSILON)
    {
        float3 _lightVec = normalize(mul(SampleDirectionHemisphere(uniformRate.x, uniformRate.y), CalTBN(_normal, _tanget)));
        Initialize(_albedoColor, _specularFactor, _normal, _lightVec, _viewVec, _roughness, _metalic, epsilon);
    }
    void SpecularSamplingInitialize(const float3 _albedoColor,
        const float _specularFactor,
        const float3 _normal,
        const float3 _viewVec,
        const float _roughness,
        const float _metalic,
        const float2 uniformRate,
        const float epsilon = EPSILON)
    {
        float3 microfaceNormal = SampleGGXVisibleNormal(_viewVec, _roughness, _roughness, uniformRate.x, uniformRate.y);
        float3 _lightVec = reflect(-_viewVec, microfaceNormal);
        
        Initialize(_albedoColor, _specularFactor, _normal, _lightVec, _viewVec, _roughness, _metalic, epsilon);
    }
//Microfact-------------------------------------------------------------------- 
    float SmithG1General(float lambda)
    {
        return 1.0f / (1.0f + lambda);
    }
    float BeckmannINDF()
    {
        const float dotNH2 = dotNH * dotNH;
        const float numerator = exp((dotNH2 - 1.0f) / (roughness2 * dotNH2));
        const float denominator = PI * roughness2 * dotNH2 * dotNH2;
        return numerator / denominator;
    }
    float BeckmannANDF()
    {
#ifdef USE_BRDF_ANISOTROPY_NDF
        const float dotNH2 = dotNH * dotNH;
        const float dotTH2 = dotTH * dotTH;
        const float dotBH2 = dotBH * dotBH;
        const float en = (dotTH2 / (rougnessX * rougnessX)) + (dotBH2 / (rougnessY * rougnessY));
        const float numerator = exp(-en / dotNH2);
        const float denominator = PI * rougnessX * rougnessY * dotNH2 * dotNH2;
        return numerator / denominator;
#else
        return BeckmannINDF();
#endif
    }
    float BeckmannA(const float dotND, const float _roughness)
    {
        return dotND / (max(_roughness, 0.00001f) * sqrt((1.0f - min(dotND * dotND, 0.99999f))));
    }
    float BeckmannLambda(const float a)
    { 
	    //use approximation lambda
        if (a >= 1.6f)
            return 0.0f;
        else
            return (1.0f - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
    }  
    float BeckmannSmithG1Walter(float a)
    {
        if (a < 1.6f)
            return ((3.535f + 2.181f * a) * a) / (1.0f + (2.276f + 2.577f * a) * a);
        else
            return 1.0f;
    }
    float BeckmannSmithG2Separable(const float aL, const float aV)
    {
        return BeckmannSmithG1Walter(aL) * BeckmannSmithG1Walter(aV);
    }
    float BeckmannSmithG2HeightCorrelated(const float aL, const float aV)
    {
        return 1.0f / (1.0f + BeckmannLambda(aL) + BeckmannLambda(aV));
    }
    float BlinnPhongINDF()
    {
        return ((roughness + 2) / PI2) * pow(dotNH, roughness);
    }
    float BlinnPhongLambda(const float dotND)
    { 
        return BeckmannLambda(BeckmannA(dotND, sqrt(2.0f / (roughness + 2))));
    }
    float BlinnPhongRoughness()
    {
        return pow(8192.0f, roughness);
    }
    
    float GGXINDF()
    {
        const float b = ((roughness2 - 1.0f) * dotNH * dotNH + 1.0f);
        return roughness2 / (PI * b * b);
    }
    float GGXANDF()
    {
#ifdef USE_BRDF_ANISOTROPY_NDF
        const float denominator = ((dotTH * dotTH) / (roughnessX * roughnessX)) + ((dotBH * dotBH) / (roughnessY * roughnessY)) + dotNH * dotNH;
        return 1.0f / PI * roughnessX * roughnessY * denominator;
#else
        return GGXINDF();
#endif
    }
    float GGXA(const float dotND)
    {
        return dotND / (max(roughness * roughness, 0.00001f) * sqrt((1.0f - min(dotND * dotND, 0.99999f))));
    }
    float GGXLambda(const float a)
    { 
        return (-1.0f + sqrt(1.0f + 1.0f / (a * a))) / 2.0f;
    } 
    float GGXSmithG1(float a)
    {
        float a2 = a * a;
        return 2.0f / (sqrt((a2 + 1.0f) / a2) + 1.0f);
    }
    float GGXSmithG1A(const float dotND)
    {
	    //Approximate
        const float dotND2 = dotND * dotND;
        return 2.0f / (sqrt(((roughness2 * (1.0f - dotND2)) + dotND2) / dotND2) + 1.0f);
    }
    // Separable version assuming independent (uncorrelated) masking and shadowing, uses G1 functions for selected NDF
    float GGXSmithG2Separable(const float aL, const float aV)
    {
        return GGXSmithG1(aL) * GGXSmithG1(aV);
    }
    // Height correlated version - non-optimized, uses G_Lambda functions for selected NDF
    float GGXSmithG2HeightCorrelated(const float aL, const float aV)
    {
        return 1.0f / (1.0f + GGXLambda(aL) + GGXLambda(aV));
    }
    // Source: "PBR Diffuse Lighting for GGX + Smith Microsurfaces", slide 84 by Hammon
    // Note that returned value is G2 / (4 * NdotL * NdotV) and therefore includes division by specular BRDF denominator
    float GGXSmithG2HeightCorrelatedA()
    {
        float _dotNL = abs(dotNL);
        float _dotNV = abs(dotNV);
        return 0.5f / (lerp(2.0f * _dotNL * _dotNV, _dotNL + _dotNV, roughness));
    }
    // Source: "Moving Frostbite to Physically Based Rendering" by Lagarde & de Rousiers
    // Note that returned value is G2 / (4 * NdotL * NdotV) and therefore includes division by specular BRDF denominator
    float GGXSmithG2HeightCorrelatedA2()
    {
        float a = dotNV * sqrt(roughness2 + dotNL * (dotNL - roughness2 * dotNL));
        float b = dotNL * sqrt(roughness2 + dotNV * (dotNV - roughness2 * dotNV));
        return 0.5f / (a + b);
    }
    
//Fresnel--------------------------------------------------------------------    
    float3 SchlickFresnel(const float dotND)
    {
        const float rFactor = 1.0f - saturate(dotND);
        return f0 + (float3(1.0f, 1.0f, 1.0f) - f0) * (rFactor * rFactor * rFactor * rFactor * rFactor);
    }
    float3 SchlickFresnel()
    {
        return SchlickFresnel(dotHL); 
    }
//Diffuse--------------------------------------------------------------------    
    float3 LambertianIDiffuse()
    {
        return 1.0f;
    }
//Schlick근사가 사용된다고 가정
//정반사율이 완벽한 프레넬 거울같은 반사율 표면에마 적용
    float3 ShirelyDiffuse()
    {
        const float nl = (1.0f - saturate(dotNL));
        const float nV = (1.0f - saturate(dotNV));
        return (21.0f / 20.0f) * (float3(1.0f, 1.0f, 1.0f) - f0) * (1.0f - (nl * nl * nl * nl * nl)) * (1.0f - (nV * nV * nV * nV * nV));
    }
    float3 OrenNayarDiffuse()
    {
        float a = roughness * roughness;
        float s = a; // / ( 1.29 + 0.5 * a );
        float s2 = s * s;
        float VoL = 2 * dotHV * dotHV - 1; // double angle identity
        float Cosri = VoL - dotNV * dotNL;
        float c1 = 1 - 0.5 * s2 / (s2 + 0.33f);
        float c2 = 0.45 * s2 / (s2 + 0.09f) * Cosri * (Cosri >= 0 ? rcp(max(dotNL, dotNV)) : 1);
        return (c1 + c2) * (1 + roughness * 0.5f);
    }
//Hard diffuse---------
    float3 DisneyDiffuse()
    {
#if 0
	    const float kss = 0.75f;
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
	    return ((1.0f - kss) * fd + 1.25f * kss * fss);
#else 
        const float nlFactor = 1.0f - dotNL;
        const float nvFactor = 1.0f - dotNV;
	
        float fd90 = 0.5 + 2 * dotHV * dotHV * roughness;
        float FdL = 1 + (fd90 - 1) * (nlFactor * nlFactor * nlFactor * nlFactor * nlFactor);
        float FdV = 1 + (fd90 - 1) * (nvFactor * nvFactor * nvFactor * nvFactor * nvFactor);
        return FdV * FdL;
#endif
    }
    float3 FrostbiteDisneyDiffuse()
    {
        float energyBias = 0.5f * roughness;
        float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);

        float FD90MinusOne = energyBias + 2.0 * dotHL * dotHL * roughness - 1.0f;

        float FDL = 1.0f + (FD90MinusOne * pow(1.0f - dotNL, 5.0f));
        float FDV = 1.0f + (FD90MinusOne * pow(1.0f - dotNV, 5.0f));
         
        return  FDL * FDV * energyFactor;
    }
    float3 HammonDiffuse()
    {
        const float nlFactor = 1.0f - dotNL;
        const float nvFactor = 1.0f - dotNV;
	
        const float nlFactor5 = nlFactor * nlFactor * nlFactor * nlFactor * nlFactor;
        const float nvFactor5 = nvFactor * nvFactor * nvFactor * nvFactor * nvFactor;
        const float r2 = roughness * roughness;
	
        const float3 fSmooth = 1.05f * (float3(1.0f, 1.0f, 1.0f) - f0) * (1.0f - nlFactor5) * (1.0f - nvFactor5);
        const float fMulti = 0.3641f * r2;
        const float facing = 0.5f + 0.5f * dotVL;
        const float fRough = facing * (0.9f - 0.4f * facing) * ((0.5f + dotNH) / dotNH);
        return ((1 - r2) * fSmooth + r2 * fRough + albedoColor * fMulti);
    }
    
 //Combine--------------------------------------------------------------------       
    float3 Evaluate()
    { 
        const float3 fresnel = SchlickFresnel(); 
#ifdef USE_BRDF_DISNEY_DIFFUSE 
	    const float3 diffuse = DisneyDiffuse() * dotNL * ONE_OVER_PI;
#elif USE_FROST_BITE_DISNEY_DIFFUSE
        const float3 diffuse = FrostbiteDisneyDiffuse() * dotNL * ONE_OVER_PI;
#elif USE_BRDF_HAMMON_DIFFUSE 
	    const float3 diffuse = HammonDiffuse() * dotNL * ONE_OVER_PI;
#elif USE_OREN_NAYAR_DIFFUSE 
	    const float3 diffuse = OrenNayarDiffuse() * dotNL * ONE_OVER_PI;
#elif USE_BRDF_SHIRELY_DIFFUSE
	    const float3 diffuse = ShirelyDiffuse() * dotNL * ONE_OVER_PI;
#elif USE_BRDF_LAMBERTIAN_DIFFUSE
        const float3 diffuse = LambertianIDiffuse() * dotNL * ONE_OVER_PI;
#else
        const float3 diffuse = float3(0, 0, 0);
#endif
        
#if USE_BRDF_GGX_MICROFACET
    
#ifdef USE_BRDF_ISOTROPY_NDF
        const float ndf = GGXINDF();
#else
        const float ndf = GGXANDF();
#endif   
        return diffuse + fresnel * ndf * GGXSmithG2HeightCorrelatedA();  
    
#elif USE_BRDF_BECKMANN_MICROFACET   
#ifdef USE_BRDF_ISOTROPY_NDF
        const float ndf = BeckmannINDF();
#else
        const float ndf = BeckmannANDF();
#endif   
        float aL = BeckmannLambda(BeckmannA(dotNL, mat.roughness));
        float aV = BeckmannLambda(BeckmannA(dotNV, mat.roughness)); 
        return diffuse + ((fresnel * ndf * BeckmannSmithG2HeightCorrelated(aL, aV)) / (4.0f * dotNL * dotNV));
    
#elif USE_BRDF_BLINN_PHONG_MICROFACET
        
#ifdef USE_BRDF_ISOTROPY_NDF
        const float ndf = BlinnPhongINDF(dotNH, r2);
#else
        const float ndf = BlinnPhongINDF(dotNH, r2);
#endif   
        
        float aL = BeckmannLambda(BeckmannA(dotNL, mat.roughness));
        float aV = BeckmannLambda(BeckmannA(dotNV, mat.roughness)); 
        return diffuse + ((fresnel * ndf * BeckmannSmithG2HeightCorrelated(aL, aV)) / (4.0f * dotNL * dotNV));
#else
        return diffuse + fresnel / (4.0f * dotNL * dotNV);
#endif

    }
    void SampleDiffuse(out float3 eval, out float pdf)
    {
#ifdef USE_BRDF_DISNEY_DIFFUSE 
	    eval = DisneyDiffuse();
#elif USE_FROST_BITE_DISNEY_DIFFUSE
        eval = FrostbiteDisneyDiffuse();
#elif USE_BRDF_HAMMON_DIFFUSE 
	    eval = HammonDiffuse();
#elif USE_OREN_NAYAR_DIFFUSE 
	    eval = OrenNayarDiffuse();
#elif USE_BRDF_SHIRELY_DIFFUSE
	    eval = ShirelyDiffuse();
#elif USE_BRDF_LAMBERTIAN_DIFFUSE
        eval = LambertianIDiffuse()I;
#else
        eval = float3(0, 0, 0);
#endif
        pdf = dotNL * ONE_OVER_PI; 
    }
    void SampleSpecular(out float3 eval, out float pdf)
    {   
        float3 f = SchlickFresnel();
        float d = GGXINDF();
        float g1 = GGXSmithG1A(dotNV);
        float g2 = GGXSmithG2HeightCorrelatedA();
        
        eval = f * (d * g2);
        pdf = d * g1;
    }
};

 
/*

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
*/