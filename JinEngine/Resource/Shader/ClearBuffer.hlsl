
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