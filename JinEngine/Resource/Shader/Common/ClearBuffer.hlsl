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


#ifndef DIMX
#define DIMX 1 
#endif

#ifndef DIMY
#define DIMY 1 
#endif
 
#ifndef DIMZ
#define DIMZ 1 
#endif

#ifndef INDEX_PER_BYTE
#define INDEX_PER_BYTE 0
#endif
  
#ifndef OFFER_BUFFER_CLEAR_VALUE
#define OFFER_BUFFER_CLEAR_VALUE 0 
#endif

RWByteAddressBuffer buffer : register(u0);
cbuffer cbPass : register(b0)
{
	uint range; 
};

[numthreads(DIMX, DIMY, DIMZ)]
void ClearBuffer(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	const uint index = (dispatchThreadID.x + DIMX * dispatchThreadID.y + DIMX * DIMY * dispatchThreadID.z);
	if (index >= range)
		return;
	
	const uint address = INDEX_PER_BYTE * index;
	buffer.Store(address, OFFER_BUFFER_CLEAR_VALUE);
}