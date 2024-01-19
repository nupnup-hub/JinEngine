#include"LightShapeDrawCommon.hlsl"

struct VertexIn
{
#ifdef USE_VERTEX_PACK
	float4 posL : POSITION;
#else
	float3 posL : POSITION;
#endif
	uint id : SV_INSTANCEID;
};
struct VertexOut
{
	float3 posV : VIEW_POSITION;
	uint id : LIGHT_ID;
};

#ifdef POINT_LIGHT
VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	uint index = vin.id + litOffset;
	float range = light[index].frustumFar * LIGHT_RANGE_OFFSET;
	 
	float4 posW = float4(vin.posL.xyz * range + light[index].midPosition, 1);
	vout.posV = mul(posW, camView);
	vout.id = vin.id;   
	return vout;
}
#elif SPOT_LIGHT
VertexOut VS(VertexIn vin)
{ 
	uint index = vin.id + litOffset;
	float range = light[index].frustumFar * LIGHT_RANGE_OFFSET;
 
	float angle = light[index].outerConeAngle;
	float XZscale = tan(angle) * range * 2.0f; //cone bottom radius is 0.5f
	float3 posL = vin.posL.xyz;
	posL.xz *= XZscale; 
	posL.y *= range;

	//if (length(posL) > range)
	//	posL = normalize(posL) * range;

	 //rotate
	float3 dir = light[index].direction;
	float3 initDir = float3(0, -1, 0);
	float rotAngle = acos(dot(initDir, dir));
	float rotCosAngle = cos(rotAngle); 
	if(abs(rotCosAngle) == 1.0f)		
		initDir.x = 0.000001f;
 
	float3 k = normalize(cross(initDir, dir)); 
	posL = posL * rotCosAngle +  cross(k, posL) * sin(rotAngle) + k * dot(k, posL) * (1.0f - cos(rotAngle));

	VertexOut vout;
	float4 posW = float4(posL + light[index].position, 1);
	vout.posV = mul(posW, camView).xyz;
	vout.id = vin.id;

	return vout;
}
#elif RECT_LIGHT
VertexOut VS(VertexIn vin)
{
#ifdef HEMISPHERE
	uint index = vin.id + litOffset;
	float range = light[index].frustumFar * LIGHT_RANGE_OFFSET;
	
	float xScale = light[index].extents.x + range;
	float yScale = light[index].extents.y + range;
	
	float3 posL = vin.posL.xyz;
	//rotate x90
	posL = float3(posL.x, -posL.z, posL.y);
	posL.x *= xScale;
	posL.y *= yScale;
	posL.z *= range;
	posL = mul(float3x3(light[index].axis[0], light[index].axis[1], light[index].axis[2]), posL);
 
	VertexOut vout;
	float4 posW = float4(posL + light[index].origin, 1);
	vout.posV = mul(posW, camView).xyz;
	vout.id = vin.id;

	return vout;
#else
	VertexOut vout;
	uint index = vin.id + litOffset;
	float range = (light[index].frustumFar) * LIGHT_RANGE_OFFSET;
	//float range = light[index].frustumFar  * LIGHT_RANGE_OFFSET;

	float4 posW = float4(vin.posL.xyz * range + light[index].origin, 1);
	vout.posV = mul(posW, camView);
	vout.id = vin.id;   
	return vout;
#endif
}
#endif