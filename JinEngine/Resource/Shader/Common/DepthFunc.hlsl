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
 