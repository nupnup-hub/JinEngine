#include"LightClusterCommon.hlsl"
  
#ifndef OFFER_BUFFER_CLEAR_VALUE
#define OFFER_BUFFER_CLEAR_VALUE CLUSTER_LIGHT_INVALID_ID 
#endif

//Root constant that tells the compute shader what light type is being computed. Set this to the light type ID before each light type pass.
cbuffer cbPass : register(b0)
{
	uint lightType;
	uint lightOffset;  
}

//This array has NUM_LIGHTS slices and contains the near and far cluster z for each tile
Texture2DArray<float2> conservativeRTs : register(t0);

//Linked list of light IDs.
//See "OIT and Indirect Illumination using DX11 Linked Lists" from GDC 10 and "Light Linked List" from Insomniac Games
RWByteAddressBuffer startOffsetBuffer : register(u0);
RWStructuredBuffer<LinkedLightID> linkedLightList : register(u1);
RWStructuredBuffer<uint> outBuffer : register(u2);

#ifdef CLEAR_BUFFER
[numthreads(DIMX, DIMY, DIMZ)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	const uint address = LINKED_LIST_INDEX_PER_BYTE * (dispatchThreadID.x + CLUSTER_DIM_X * dispatchThreadID.y + CLUSTER_DIM_X * CLUSTER_DIM_Y * dispatchThreadID.z);	 
	startOffsetBuffer.Store(address, OFFER_BUFFER_CLEAR_VALUE);
}
#else
[numthreads(DIMX, DIMY, DIMZ)]
void CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	//Load near and far values(x is near and y is far)
	float2 nearAndFar = conservativeRTs.Load(int4(dispatchThreadID, 0)); 
	
	//Inactive tile, return
	if (nearAndFar.x == 1.0f && nearAndFar.y == 1.0f)
		return;
	
	//Unpack to clusterZ space([0,1] to [0,255]). Also handle cases where no near of far cluster was written.
	uint near = (nearAndFar.x == 1.0f) ? 0 : uint(nearAndFar.x * CLUSTER_MAX_DEPTH + 0.5f);
	uint far = (nearAndFar.y == 1.0f) ? (CLUSTER_DIM_Z - 1) : uint(((CLUSTER_DIM_Z - 1.0f) / CLUSTER_MAX_DEPTH - nearAndFar.y) * CLUSTER_MAX_DEPTH + 0.5f);

	//Loop through near to far and fill the light linked list
	uint offsetIndexBase = LINKED_LIST_INDEX_PER_BYTE * (dispatchThreadID.x + CLUSTER_DIM_X * dispatchThreadID.y);
	uint offsetIndexStep = LINKED_LIST_INDEX_PER_BYTE * CLUSTER_DIM_X * CLUSTER_DIM_Y;
	
	for (uint i = near; i <= far; ++i)
	{
		uint indexCount = linkedLightList.IncrementCounter();
		uint startOffsetAddress = offsetIndexBase + offsetIndexStep * i;
		 
		uint preOffset;
		startOffsetBuffer.InterlockedExchange(startOffsetAddress, indexCount, preOffset);

		LinkedLightID linkedNode;
		//Light type is encoded in the last CLUSTER_LIGHT_TYPE_PER_BIT of the node.
		//lightID and lightID in the first CLUSTER_LIGHT_ID_PER_BIT.
		linkedNode.lightID = (lightType << CLUSTER_LIGHT_ID_PER_BIT) | (dispatchThreadID.z & CLUSTER_LIGHT_ID_RANGE);
		linkedNode.link = preOffset;

		linkedLightList[indexCount] = linkedNode;
	}
	//group dim z is light count
	++outBuffer[dispatchThreadID.z + lightOffset];
}
#endif

