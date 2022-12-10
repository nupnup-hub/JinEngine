
float ToLinearZValue(const float v, const float camNear, const float camFar)
{
	return (2.0f * camNear) / (camNear + camFar - v * (camFar - camNear));
}
float ToNoLinearZValue(const float v, const float camNear, const float camFar)
{
	return  -((camNear + camFar) * v - (2 * camNear)) / ((camNear - camFar) * v);
}