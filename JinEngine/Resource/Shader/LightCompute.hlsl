#pragma once
#include"BxDF.hlsl" 
#include"PixelCommon.hlsl" 
 
struct DirectionalLight
{
	float3 direction;
	float3 color;
	float power;
};
struct PointLight
{
	float3 midPosition;
	float3 sidePosition[2];
	float3 color;
	float power;
	float range;
};
struct SpotLight
{
	float3 position;
	float3 direction;
	float3 color;
	float power;
	float range;
	float innerConeCosAngle;
	float outerConeCosAngle;
};
struct RectLight
{
	float3 origin; 
	float2 extents;
	float2 offset;
	float3 axis[3];
	float3 color;
	float power; 
	float range; 
	float distance;
	int sourceTextureIndex; 
};
#define LCT_SIZE 32.0f 

float3 CalBxDF(const float3 normal, float3 tangent, const float3 lightVec, const float3 viewVec, const Material mat)
{
	const float3 halfVec = normalize(viewVec + lightVec);
	
	float3 f0;
	const float3 f = SchlickFresnel(mat.albedoColor.xyz, halfVec, lightVec, mat.metalic, f0);
#ifdef USE_BRDF_DISNEY_DIFFUSE
	const float3 diffColor = DisneyDiffuse(mat.albedoColor.xyz, normal, lightVec, viewVec, halfVec, mat.roughness);
#elif USE_BRDF_HAMMON_DIFFUSE
	const float3 diffColor = HammonDiffuse(mat.albedoColor.xyz, f0, normal, lightVec, viewVec, halfVec, mat.roughness);
#elif USE_OREN_NAYAR_DIFFUSE
	const float3 diffColor = OrenNayarDiffuse(mat.albedoColor.xyz, normal, lightVec, viewVec, halfVec, mat.roughness);
#elif USE_BRDF_SHIRELY_DIFFUSE
	const float3 diffColor = ShirelyDiffuse(mat.albedoColor.xyz, f0, normal, lightVec, viewVec);
#elif USE_BRDF_LAMBERTIAN_DIFFUSE
	const float3 diffColor = LambertianIDiffuse(mat.albedoColor.xyz, f0);
#else 
	const float3 diffColor = float3(0, 0, 0);
#endif
	
#ifdef USE_GGX_SMITH_APPROXIMATION
	const float3 specColor = f * GGXINDF(normal, halfVec, mat.roughness) * GGXSmith(normal, lightVec, viewVec, mat.roughness);
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

//https://web.archive.org/web/20051026043415id_/http://www.aliaswavefront.com:80/eng/about/research/papers/file/extlight92.pdf
float3 CalPointLightNearPoint(float3 pLitSidePos00, float3 pLitSidePos01, float3 posW, float3 normal, float3 toEye)
{
	float3 litVec00 = pLitSidePos00 - posW;
	float3 litVec01 = pLitSidePos01 - posW;
	float3 r = reflect(toEye, normal);

	float3 ld = litVec01 - litVec00;
	float rDotLd = dot(r, ld);
	float ldL = length(ld);
	
	float t = ((dot(r, litVec00) * dot(r, ld)) - dot(litVec00, ld)) / (ldL * ldL - rDotLd * rDotLd);
	return litVec00 + saturate(t) * ld;
}
float3 IntegrateEdgeVec(float3 L0, float3 L1)
{
	//float w01 = ( 1.5708 - 0.175 * c01 ) * rsqrt( c01 + 1 );	// 1 mad, 1 mul, 1 add, 1 rsqrt
	//float w01 = 1.5708 + (-0.879406 + 0.308609 * abs(c01) ) * abs(c01);

	//return acos( c01 ) * rsqrt( 1 - c01 * c01 );

#if 0
	// [ Hill et al. 2016, "Real-Time Area Lighting: a Journey from Research to Production" ]
	float c01 = dot(L0, L1);
	float w01 = ( 5.42031 + (3.12829 + 0.0902326 * abs(c01)) * abs(c01) ) /
				( 3.45068 + (4.18814 + abs(c01)) * abs(c01) );

	w01 = c01 > 0 ? w01 : PI * rsqrt( 1 - c01 * c01 ) - w01;
	return w01;
#elif 0
	float c01 = dot(L0, L1);
	float w01 = (0.8543985 + (0.4965155 + 0.0145206 * abs(c01)) * abs(c01)) /
				(3.4175940 + (4.1616724 + abs(c01)) * abs(c01));
	
	w01 = c01 > 0 ? w01 : 0.5 * rsqrt(1 - c01 * c01) - w01;
	return w01;
#elif 1
	float x = dot(L0, L1);
	float y = abs(x);

	float a = 0.8543985f + (0.4965155f + 0.0145206f * y) * y;
	float b = 3.4175940f + (4.1616724f + y) * y;
	float v = a / b;

	float theta_sintheta = (x > 0.0f) ? v : 0.5f * rsqrt(max(1.0f - x * x, 1e-7f)) - v;
	return theta_sintheta * cross(L0, L1);
#else
	float cosTheta = dot(L0, L1);
	cosTheta = clamp(cosTheta, -0.9999f, 0.9999f);
    
	float theta = acos(cosTheta);
	const float res = cross(L0, L1).z * ((theta > 0.001f) ? theta / sin(theta) : 1.0);
	return res;
	
	//float res = cross(L0, L1).z * theta / sin(theta); 
	//return res;
#endif
}
float IntegrateEdge(float3 v1, float3 v2)
{
	return IntegrateEdgeVec(v1, v2).z;
}
float3 FetchDiffuseFilteredTexture(float3 p[4], Texture2D sourceTexture)
{
    // area light plane basis
	float3 V1 = p[1] - p[0];
	float3 V2 = p[3] - p[0];
	float3 planeOrtho = (cross(V1, V2));
	float planeAreaSquared = dot(planeOrtho, planeOrtho);
	float planeDistxPlaneArea = dot(planeOrtho, p[0]);
    // orthonormal projection of (0,0,0) in area light space
	float3 P = planeDistxPlaneArea * planeOrtho / planeAreaSquared - p[0];

    // find tex coords of P
	float dot_V1_V2 = dot(V1, V2);
	float inv_dot_V1_V1 = 1.0 / dot(V1, V1);
	float3 V2_ = V2 - V1 * dot_V1_V2 * inv_dot_V1_V1;
	float2 Puv;
	Puv.y = dot(V2_, P) / dot(V2_, V2_);
	Puv.x = dot(V1, P) * inv_dot_V1_V1 - dot_V1_V2 * inv_dot_V1_V1 * Puv.y;

    // LOD
	float d = abs(planeDistxPlaneArea) / pow(planeAreaSquared, 0.75f);
	float2 textureSize;
	sourceTexture.GetDimensions(textureSize.x, textureSize.y); // TODO optimize

	return sourceTexture.SampleLevel(samLTCSample, float2(0.125f, 0.125f) + 0.75 * Puv, log(textureSize.x * d) / log(3.0)).rgb;
}
void ClipQuadToHorizon(inout float3 L[5], out int n)
{
    // detect clipping config
	int config = 0;
	if (L[0].z > 0.0)
		config += 1;
	if (L[1].z > 0.0)
		config += 2;
	if (L[2].z > 0.0)
		config += 4;
	if (L[3].z > 0.0)
		config += 8;
    
    // clip
	n = 0;
    
	if (config == 0)
	{
        // clip all
	}
	else if (config == 1) // V1 clip V2 V3 V4
	{
		n = 3;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[3].z * L[0] + L[0].z * L[3];
	}
	else if (config == 2) // V2 clip V1 V3 V4
	{
		n = 3;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 3) // V1 V2 clip V3 V4
	{
		n = 4;
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
		L[3] = -L[3].z * L[0] + L[0].z * L[3];
	}
	else if (config == 4) // V3 clip V1 V2 V4
	{
		n = 3;
		L[0] = -L[3].z * L[2] + L[2].z * L[3];
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
	}
	else if (config == 5) // V1 V3 clip V2 V4) impossible
	{
		n = 0;
	}
	else if (config == 6) // V2 V3 clip V1 V4
	{
		n = 4;
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
		L[3] = -L[3].z * L[2] + L[2].z * L[3];
	}
	else if (config == 7) // V1 V2 V3 clip V4
	{
		n = 5;
		L[4] = -L[3].z * L[0] + L[0].z * L[3];
		L[3] = -L[3].z * L[2] + L[2].z * L[3];
	}
	else if (config == 8) // V4 clip V1 V2 V3
	{
		n = 3;
		L[0] = -L[0].z * L[3] + L[3].z * L[0];
		L[1] = -L[2].z * L[3] + L[3].z * L[2];
		L[2] = L[3];
	}
	else if (config == 9) // V1 V4 clip V2 V3
	{
		n = 4;
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
		L[2] = -L[2].z * L[3] + L[3].z * L[2];
	}
	else if (config == 10) // V2 V4 clip V1 V3) impossible
	{
		n = 0;
	}
	else if (config == 11) // V1 V2 V4 clip V3
	{
		n = 5;
		L[4] = L[3];
		L[3] = -L[2].z * L[3] + L[3].z * L[2];
		L[2] = -L[2].z * L[1] + L[1].z * L[2];
	}
	else if (config == 12) // V3 V4 clip V1 V2
	{
		n = 4;
		L[1] = -L[1].z * L[2] + L[2].z * L[1];
		L[0] = -L[0].z * L[3] + L[3].z * L[0];
	}
	else if (config == 13) // V1 V3 V4 clip V2
	{
		n = 5;
		L[4] = L[3];
		L[3] = L[2];
		L[2] = -L[1].z * L[2] + L[2].z * L[1];
		L[1] = -L[1].z * L[0] + L[0].z * L[1];
	}
	else if (config == 14) // V2 V3 V4 clip V1
	{
		n = 5;
		L[4] = -L[0].z * L[3] + L[3].z * L[0];
		L[0] = -L[0].z * L[1] + L[1].z * L[0];
	}
	else if (config == 15) // V1 V2 V3 V4
	{
		n = 4;
	}
    
	if (n == 3)
		L[3] = L[0];
	if (n == 4)
		L[4] = L[0];
}
float3 PolygonIrradiance(float3 pos, float3 points[4], uint twoSided)
{
#if 1
	float3 nPoint[5];
	nPoint[0] = points[0];
	nPoint[1] = points[1];
	nPoint[2] = points[2];
	nPoint[3] = points[3];
	nPoint[4] = points[3];
 	
	int n = 0;
	ClipQuadToHorizon(nPoint, n);
	if (n == 0)
		return 0;
	
	[unroll]
	for (int i = 0; i < 5; ++i)
		nPoint[i] = normalize(nPoint[i]);
	 
    // integrate 
	float sum = 0;
	sum += IntegrateEdge(nPoint[0], nPoint[1]);
	sum += IntegrateEdge(nPoint[1], nPoint[2]);
	sum += IntegrateEdge(nPoint[2], nPoint[3]);
	if (n >= 4)
		sum += IntegrateEdge(nPoint[3], nPoint[4]);
	if (n == 5)
		sum += IntegrateEdge(nPoint[4], nPoint[0]);
      
	sum = twoSided ? abs(sum) : max(0.0, -sum);
	//sum /= (2.0f * PI); 
    
    // scale by filtered light color
	//Lo_i *= textureLight;
    
	return float3(sum, sum, sum);
#else 
	 // use tabulated horizon-clipped sphere
    // check if the shading point is behind the light

	float3 dir = points[0] - pos; // ltcM space
	float3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
	bool behind = (dot(dir, lightNormal) < 0.0);
 
	[unroll]
	for (int i = 0; i < 4; ++i)
		points[i] = normalize(points[i]);
	
    // integrate
	float3 vsum = 0;
	vsum += IntegrateEdgeVec(points[0], points[1]);
	vsum += IntegrateEdgeVec(points[1], points[2]);
	vsum += IntegrateEdgeVec(points[2], points[3]);
	vsum += IntegrateEdgeVec(points[3], points[0]);
	 
    // form factor of the polygon in direction vsum
	float len = length(vsum);

	float z = vsum.z / len;
	if (behind)
		z = -z;

	float2 uv = float2(z * 0.5f + 0.5f, len) * ((LCT_SIZE - 1) / LCT_SIZE) + (0.5f / LCT_SIZE);
	
    // Fetch the form factor for horizon clipping
	float scale = textureMaps[ltcAmpTextureIndex].SampleLevel(samLTC, uv, 0).w; 
 
	float sum = len;
	if (!behind && !twoSided)
		sum = 0.0;
	
	return float3(sum, sum, sum);
#endif
}
float3 PolygonFormFactor(float3 points[4])
{
	[unroll]
	for (uint i = 0; i < 4; i++)
		points[i] = normalize(points[i]);

	float3 f = IntegrateEdgeVec(points[0], points[1]);
	f += IntegrateEdgeVec(points[1], points[2]);
	f += IntegrateEdgeVec(points[2], points[3]);
	f += IntegrateEdgeVec(points[3], points[0]);

	return f;
}

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
float3 ComputeDirectionalLight(DirectionalLight light, Material mat, float3 normalW, float3 tangent, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -light.direction;

    // Scale light down by Lambert's cosine law.
	float nDotL = max(dot(lightVec, normalW), 0.0f);
	return light.color * nDotL * light.power * CalBxDF(normalW, tangent, lightVec, toEye, mat);
}
float3 ComputePointLight(PointLight light, Material mat, float3 posW, float3 normalW, float3 tangent, float3 toEye)
{
	float3 lightVec = CalPointLightNearPoint(light.sidePosition[0], light.sidePosition[1], posW, normalW, toEye);
	float d = length(lightVec);
	if (d > light.range)
		return 0.0f;
    
	lightVec /= d;
	float nDotL = max(dot(lightVec, normalW), 0.0f);
	float attFactor = CalInverseAttenuation(d, light.range);

	return light.color * nDotL * light.power * attFactor * CalBxDF(normalW, tangent, lightVec, toEye, mat);
}
float3 ComputeSpotLight(SpotLight light, Material mat, float3 posW, float3 normalW, float3 tangent, float3 toEye)
{
	float3 lightVec = light.position - posW;
	float d = length(lightVec);
	if (d > light.range)
		return 0.0f;
    
	lightVec /= d;
	float nDotL = max(dot(lightVec, normalW), 0.0f);
	float dDotL = dot(-lightVec, light.direction);
    
	float attFactor = CalInverseAttenuation(d, light.range);
	float epsilon = light.innerConeCosAngle - light.outerConeCosAngle;
	float smoothFactor = clamp((dDotL - light.outerConeCosAngle) / epsilon, 0.0f, 1.0f);
     
	return light.color * nDotL * light.power * attFactor * smoothFactor * smoothFactor * CalBxDF(normalW, tangent, lightVec, toEye, mat);
}
float FastAcos(float inX)
{
	float x = abs(inX);
	float res = -0.156583f * x + 1.570796f;
	res *= sqrt(1.0f - x);
	return (inX >= 0) ? res : PI - res;
}
float2 LtcCoord(float cosTheta, float roughness)
{
#if 1
	return float2(roughness, sqrt(1.0 - cosTheta)) * ((LCT_SIZE - 1.0f) / LCT_SIZE) + (0.5f / LCT_SIZE);
#else
	return float2(roughness, FastAcos(cosTheta) / (0.5 * PI)) * ((LCT_SIZE - 1) / LCT_SIZE) + (0.5f / LCT_SIZE);
#endif
}
float3 SampleSourceTexture(float3 lightDir, float3 origin, float3 axis[3], float2 extents, Texture2D sourceTexture)
{
 	// Force to point at plane
	lightDir += axis[2] * saturate(0.001f - dot(axis[2], lightDir));

	// Intersect ray with plane
	float DistToPlane = dot(axis[2], origin) / dot(axis[2], lightDir);
	float3 PointOnPlane = lightDir * DistToPlane;

	float2 PointInRect;
	PointInRect.x = dot(axis[0], PointOnPlane - origin);
	PointInRect.y = dot(axis[1], PointOnPlane - origin);
	//PointInRect.x = clamp(dot(axis[0], PointOnPlane - origin), -extents.x, extents.x);
	//PointInRect.y = clamp(dot(axis[1], PointOnPlane - origin), -extents.y, extents.y);

	float2 RectUV = (PointInRect / (extents * 2)) * float2(0.5, -0.5) + 0.5;
	float2 TextureSize;
	sourceTexture.GetDimensions(TextureSize.x, TextureSize.y); // TODO optimize

	float Level = log2(DistToPlane * rsqrt(extents.x * extents.y));
	Level += log2(max(TextureSize.x, TextureSize.y)) - 2;
	  
	return sourceTexture.SampleLevel(samLTCSample, RectUV, Level).rgb;
}
float3 SampleSourceTextureU(float3 points[4], float3 f, Texture2D sourceTexture)
{
	//points[0] = bottom-left
	//points[1] = bottom-right
	//points[2] = top-right
	//points[3] = top-left	
	float3 origin = points[0];
	float3 right = points[1] - origin;
	float3 up = points[3] - origin;
	
	float3 normal = cross(right, up);
	float sqArea = dot(normal, normal);
	normal *= rsqrt(sqArea);
	
	float hitDistance = dot(origin, normal) / dot(f, normal);
	float3 hitPosition = hitDistance * normal;
	
	float recSqLengthRight = 1.0 / dot(right, right);
	float upRightMixing = dot(up, right);
	float3 ortho = up - upRightMixing * right * recSqLengthRight;
	
	float v = dot(hitPosition, ortho) / dot(ortho, ortho);
	float u = (dot(hitPosition, right) - upRightMixing * v) * recSqLengthRight;
	float2 hitUV = float2(1.0 - u, v);
	
	const float COOKIE_MIPS_COUNT = 10;
	float mipLevel = 0.5 * log2(1e-8 + PI * hitDistance * hitDistance * rsqrt(sqArea)) + COOKIE_MIPS_COUNT;

	return sourceTexture.SampleLevel(samLTCSample, hitUV, 0).rgb;
}
float3 ComputeRectLight(RectLight light, Material mat, float3 posW, float3 normalW, float3 tangent, float3 toEye)
{
	//float NoV = saturate( abs( dot(N, V) ) + 1e-5 );
	if (length(light.extents) == 0)
		return float3(0, 0, 0);
	
	float dotNV = saturate(dot(normalW, toEye));
	float2 uv = LtcCoord(dotNV, mat.roughness);
 
	//float2 uv = LtcCoord(dotNV, mat.roughness);
	float4 ltcMat = textureMaps[ltcMatTextureIndex].SampleLevel(samLTC, uv, 0);
	float4 ltcAmp = textureMaps[ltcAmpTextureIndex].SampleLevel(samLTC, uv, 0);
	
	float3x3 ltcM = float3x3
	(
		float3(1, 0, ltcMat.y),
		float3(0, ltcMat.z, 0),
		float3(ltcMat.w, 0, ltcMat.x)
    );
	// construct orthonormal basis around N
	float3 t1 = normalize(toEye - normalW * dot(normalW, toEye));
	float3 t2 = cross(normalW, t1);
	float3x3 tM = float3x3(t1, t2, normalW);
	
    //rotate area light in (T1, T2, N) basis
	ltcM = mul(transpose(tM), ltcM);

	float3 ex = light.extents.x * light.axis[0];
	float3 ey = light.extents.y * light.axis[1];
	
	float3 rPoints[4];
	rPoints[0] = light.origin - ex - ey;
	rPoints[1] = light.origin + ex - ey;
	rPoints[2] = light.origin + ex + ey;
	rPoints[3] = light.origin - ex + ey;
			
	float3 v1 = rPoints[1] - rPoints[0];
	float3 v2 = rPoints[3] - rPoints[0]; 
	float att = CalInverseAttenuation(light.distance, light.range);
 
	float3 diffuse = PolygonIrradiance(posW, rPoints, false);
	float3 diffuseLightColor = float3(1, 1, 1);
	if (light.sourceTextureIndex != -1)
	{
		diffuseLightColor = FetchDiffuseFilteredTexture(rPoints, textureMaps[light.sourceTextureIndex]);
		diffuseLightColor = pow(diffuseLightColor, float3(2.2f, 2.2f, 2.2f));
	}
	diffuseLightColor *= mat.albedoColor.xyz;
	
	[unroll]
	for (int i = 0; i < 4; ++i)
		rPoints[i] = mul(rPoints[i] - posW, ltcM);
	
	float3 irradiance = PolygonIrradiance(posW, rPoints, false);
	float3 lightColor = float3(1, 1, 1);
	if (light.sourceTextureIndex != -1)
	{
		lightColor = FetchDiffuseFilteredTexture(rPoints, textureMaps[light.sourceTextureIndex]);
		lightColor = pow(lightColor, float3(2.2f, 2.2f, 2.2f));
	}
	lightColor *= light.color;
 
	//DisneyDiffuse(mat.albedoColor.xyz, normalW, lightVec, toEye, halfVec, mat.roughness, 0.75f);
	// GGX BRDF shadowing and Fresnel
    // t2.x: shadowedF90 (F90 normally it should be 1.0)
    // t2.y: Smith function for Geometric Attenuation Term, it is dot(V or L, H).
	 
	// specular *= mSpecular*t2.x + (1.0f - mSpecular) * t2.y; 	
	float3 specular = mat.albedoColor.xyz * ltcAmp.x + (1.0f - mat.albedoColor.xyz) * ltcAmp.y;
	return lightColor * light.power * att * irradiance * (specular + (diffuse * diffuseLightColor));
}
RectLight ComputeRect(RectLightData data, float3 posW)
{
	RectLight rect;  
	rect.offset = 0;
	rect.axis = data.axis;
	rect.color = data.color;
	rect.power = data.power;
	rect.range = data.frustumFar;
	rect.sourceTextureIndex = data.sourceTextureIndex;
	//rect.isTwoSide = data.isTwoSide;
	
	float3 origin = data.origin;
	float2 extents = data.extents;
	float barndoorCosAngle = data.barndoorCosAngle;
	float barndoorSinAngle = sqrt(1 - barndoorCosAngle * barndoorCosAngle);
	float barndoorDepth = data.barndoorLength * barndoorSinAngle;
	const float3 xAxis = rect.axis[0];
	const float3 yAxis = rect.axis[1];
	
	float3 posL = mul(float3x3(data.axis[0], data.axis[1], data.axis[2]), posW - origin);
	float2 sgn = sign(posL.xy);
	
	//rect 범위 밖에 있는 point들만 유효한 값을 가짐
	float2 posToCorner = extents - abs(posL.xy);
	posToCorner = sgn * clamp(posToCorner, posToCorner, 0);
	 
	float2 cornerToClosePoint = posToCorner * (data.barndoorLength / (posL.z - data.barndoorLength + 0.0001f));
	float2 corner = posL.xy + posToCorner;
	
	float cornerAngle = dot(normalize(float3(corner.xy, posL.z)), normalize(rect.axis[2]));	// 0~1 max = cos(90degree) = 0
	cornerAngle = abs(cornerAngle);
	float rate = saturate((cornerAngle - min(barndoorCosAngle, cornerAngle)) * (1 / cornerAngle)); //cornerAngle보다 낮은 angle clamp
	
	float2 conreToClosePointF = abs(cornerToClosePoint * rate);
	const float2 minXY = clamp(-extents + conreToClosePointF * max(0, -sgn), -extents, extents);
	const float2 maxXY = clamp(extents - conreToClosePointF * max(0, sgn), -extents, extents);
	const float2 offset = 0.5f * (minXY + maxXY);
	
	rect.origin = origin + xAxis * offset.x + yAxis * offset.y;
	rect.extents = (0.5f * (maxXY - minXY));
	if (length(posToCorner) != 0)
		rect.extents *= saturate(posL.z - barndoorDepth);
	rect.distance = length(float3(abs(posToCorner) + conreToClosePointF, posL.z));
	rect.offset = offset;
	
	return rect;
}

