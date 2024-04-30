#pragma once
#include"../JSampler.h"

namespace JinEngine::Graphic
{
	class JDx12Sampler final: public JSampler
	{
	public:
		J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
	public:
		void Build1DSampleBuffer(JGraphicDevice* device, const std::vector<float>& sample) final;
		void Build2DSampleBuffer(JGraphicDevice* device, const std::vector<JVector2F>& sample) final;
		void Build3DSampleBuffer(JGraphicDevice* device, const std::vector<JVector3F>& sample) final;
	};
}