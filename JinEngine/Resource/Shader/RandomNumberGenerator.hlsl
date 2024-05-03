//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RANDOMNUMBERGENERATOR_H
#define RANDOMNUMBERGENERATOR_H

// Ref: http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
struct RandomNumberGenerator
{
    uint state;
    uint2 BlockCipherTEA(uint v0, uint v1, uint iterations = 16)
    {
        uint sum = 0;
        const uint delta = 0x9e3779b9;
        const uint k[4] = { 0xa341316c, 0xc8013ea4, 0xad90777d, 0x7e95761e }; // 128-bit key.
        for (uint i = 0; i < iterations; i++)
        {
            sum += delta;
            v0 += ((v1 << 4) + k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + k[1]);
            v1 += ((v0 << 4) + k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + k[3]);
        }
        return uint2(v0, v1);
    }
    /**
     * 32-bit bit interleave (Morton code).
     * @param[in] v 16-bit values in the LSBs of each component (higher bits don't matter).
     * @return 32-bit value.
     */
    uint Interleave32bit(uint2 v)
    {
        uint x = v.x & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
        x = (x | (x << 8)) & 0x00FF00FF; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
        x = (x | (x << 4)) & 0x0F0F0F0F; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
        x = (x | (x << 2)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
        x = (x | (x << 1)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0

        uint y = v.y & 0x0000ffff;
        y = (y | (y << 8)) & 0x00FF00FF;
        y = (y | (y << 4)) & 0x0F0F0F0F;
        y = (y | (y << 2)) & 0x33333333;
        y = (y | (y << 1)) & 0x55555555;

        return x | (y << 1);
    }
    // Create an initial random number for this thread
    void Initialize(uint seed)
    {
        // Thomas Wang hash 
        // Ref: http://www.burtleburtle.net/bob/hash/integer.html
        seed = (seed ^ 61) ^ (seed >> 16);
        seed *= 9;
        seed = seed ^ (seed >> 4);
        seed *= 0x27d4eb2d;
        seed = seed ^ (seed >> 15);
        state =  seed;
    }
    void Initialize(uint2 pixel, uint sampleNumber)
    { 
        state = BlockCipherTEA(Interleave32bit(pixel), sampleNumber).x;
    }
    // Generate a random 32-bit integer
    uint Random()
    {
        // Xorshift algorithm from George Marsaglia's paper.
        state ^= (state << 13);
        state ^= (state >> 17);
        state ^= (state << 5);
        return state;
    }
    // Generate a random float in the range [0.0f, 1.0f)
    float Random01()
    {
        return asfloat(0x3f800000 | Random() >> 9) - 1.0;
    }
    // Generate a random float in the range [0.0f, 1.0f]
    float Random01inclusive()
    {
        return Random() / float(0xffffffff);
    }
    // Generate a random integer in the range [lower, upper]
    uint Random(uint lower, uint upper)
    {
        return lower + uint(float(upper - lower + 1) * Random01());
    }
    
    // 3D value noise
    // Ref: https://www.shadertoy.com/view/XsXfRH
    // The MIT License
    // Copyright © 2017 Inigo Quilez
    // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    float hash(float3 p)
    {
        p = frac(p * 0.3183099 + .1);
        p *= 17.0;
        return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
    }
};
#endif // RANDOMNUMBERGENERATOR_H