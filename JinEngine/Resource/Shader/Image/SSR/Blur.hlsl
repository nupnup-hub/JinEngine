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
 
Texture2D<float4> srcMap : register(t0);
RWTexture2D<float4> destMap : register(u0); 
SamplerState samLinearClmap : register(s0);

cbuffer cb0 : register(b0)
{
    float4 dim;     //xy = rt, zw = inverse 
    float stepScale;
};

#ifndef DIMX
#define DIMX 16
#endif
#ifndef DIMY
#define DIMY 16
#endif 
 
#define TAB_COUNT 7 
 
float4 Blur(const float2 centerUV)
{
    float weight[TAB_COUNT] =
    {
        0.001f, 0.028f, 0.233f, 0.474f, 0.233f, 0.028f, 0.001f
    };
    
    float4 totalColor = float4(0, 0, 0, 0);
    float totalWeight = 0;
      
#ifdef VERTICAL
    float2 processDir = float2(1, 0);
#else
    float2 processDir = float2(0, 1);
#endif
    float2 uvStepScale = processDir * dim.zw * stepScale;
    float2 uv = centerUV + uvStepScale * int(-(TAB_COUNT / 2));
    
    [unroll]
    for (uint i = 0; i < TAB_COUNT; ++i)
    {
        totalColor += srcMap.SampleLevel(samLinearClmap, uv, 0) * weight[i];
        totalWeight += weight[i];
        
        uv += uvStepScale;
    }

    return totalColor / totalWeight;
}

[numthreads(DIMX, DIMY, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= dim.x || dispatchThreadID.y >= dim.y)
        return;
    
    const float2 centerUV = float2(dispatchThreadID.xy + 0.5f) * dim.zw;
    destMap[dispatchThreadID.xy] = Blur(centerUV);  
}
