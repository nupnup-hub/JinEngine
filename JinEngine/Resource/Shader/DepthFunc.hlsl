#pragma once
/*
* Convert Ndc depth to linear depth (Perpective)
*/
float LinearDepth(const float v, const float near, const float far)
{
#if 1
	return ((far * near) / (far - v * (far - near)) - near) / (far - near);
#else 	
	float c1 = far / near;
	float c0 = 1.0 - c1; 
	return 1.0f / (c0 * v + c1); 
#endif
}
/*
* Convert linear depth to Ndc depth (Perpective)
*/
float NonLinearDepth(const float v, const float near, const float far)
{
	return (far / (far - near)) - ((near * far) / ((far - near) * (v * (far - near) + near)));
}

/*
Orhto depth value is linear
*/

float NdcToViewPZ(const float v, const float near, const float far)
{ 
	return (far * near) / (far - v * (far - near));
}
float NdcToViewPZ(const float v, const float camNearMulFar, const float2 camNearFar)
{
    return camNearMulFar / (camNearFar.y - v * (camNearFar.y - camNearFar.x));
}
float ViewToNdcPZ(const float v, const float near, const float far)
{
	return (far / (far - near)) - ((near * far) / ((far - near) * v));
}
float NdcToViewOZ(const float v, const float near, const float far)
{
	return v * (far - near) + near;
}
float ViewToNdcOZ(const float v, const float near, const float far)
{
	return (v - near) / (far - near);
}
 