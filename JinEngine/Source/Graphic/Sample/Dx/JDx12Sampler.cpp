#include"JDx12Sampler.h"

namespace JinEngine::Graphic
{
	J_GRAPHIC_DEVICE_TYPE JDx12Sampler::GetDeviceType()const noexcept
	{
		return J_GRAPHIC_DEVICE_TYPE::DX12;
	}
	void JDx12Sampler::Build1DSampleBuffer(JGraphicDevice* device, const std::vector<float>& sample)
	{

	}
	void JDx12Sampler::Build2DSampleBuffer(JGraphicDevice* device, const std::vector<JVector2F>& sample)
	{

	}
	void JDx12Sampler::Build3DSampleBuffer(JGraphicDevice* device, const std::vector<JVector3F>& sample)
	{

	}
}