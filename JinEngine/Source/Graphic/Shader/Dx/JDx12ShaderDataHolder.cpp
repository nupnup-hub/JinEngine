#include"JDx12ShaderDataHolder.h"

namespace JinEngine::Graphic
{
	JDx12GraphicShaderDataHolder::JDx12GraphicShaderDataHolder()
		:JGraphicShaderDataHolder()
	{}
	JDx12GraphicShaderDataHolder::~JDx12GraphicShaderDataHolder()
	{
		Clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12GraphicShaderDataHolder::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	ID3D12PipelineState* JDx12GraphicShaderDataHolder::GetExtraPso(const J_GRAPHIC_SHADER_EXTRA_FUNCTION type)const noexcept
	{
		return extraPso[(uint)type].Get();
	}
	bool JDx12GraphicShaderDataHolder::HasExtra(const J_GRAPHIC_SHADER_EXTRA_FUNCTION type)const noexcept
	{
		return 	extraPso[(uint)type] != nullptr;
	}
	void JDx12GraphicShaderDataHolder::Clear()
	{
		vs.Reset();
		hs.Reset();
		ds.Reset();
		gs.Reset();
		ps.Reset();
		inputLayout.clear();
		pso.Reset();
		for (int i = 0; i < (int)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT; ++i)
			extraPso[i].Reset();
	}

	JDx12ComputeShaderDataHolder::JDx12ComputeShaderDataHolder()
		:JComputeShaderDataHolder()
	{}
	JDx12ComputeShaderDataHolder::~JDx12ComputeShaderDataHolder()
	{
		Clear();
	}
	J_GRAPHIC_DEVICE_TYPE JDx12ComputeShaderDataHolder::GetDeviceType()const noexcept 
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12ComputeShaderDataHolder::Clear()
	{
		cs.Reset();
		pso.Reset();
		RootSignature = nullptr;
	}
}