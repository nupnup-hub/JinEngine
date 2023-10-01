
/*
* Convert Ndc depth to linear depth (Perpective)
*/
float LinearDepth(const float v, const float near, const float far)
{ 
	return ((far * near) / (far - v * (far - near)) - near) / (far - near);
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