#include"JShaderData.h"

namespace JinEngine
{ 
	void JGraphicShaderData::Clear()
	{
		vs.Reset();
		hs.Reset();
		ds.Reset();
		gs.Reset();
		ps.Reset();
		inputLayout.clear();
		pso.Reset();
		for (int i = 0; i < (int)J_GRAPHIC_EXTRA_PSO_TYPE::COUNT; ++i)
			extraPso[i].Reset(); 
	}
	void JComputeShaderData::Clear()
	{
		cs.Reset(); 
		pso.Reset();
		RootSignature = nullptr;
	}
	uint JComputeShaderData::DispatchInfo::GetTotalThreadCount()const noexcept
	{
		return (groupDim.x * groupDim.y * groupDim.z) * (threadDim.x * threadDim.y * threadDim.z);
	}
}