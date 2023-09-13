
float ToLinearZValue(const float v, const float near, const float far)
{
	return (2.0f * near) / (near + far - v * (far - near));
}
float ToNoLinearZValue(const float v, const float near, const float far)
{
	return  -((near + far) * v - (2 * near)) / ((near - far) * v);
}