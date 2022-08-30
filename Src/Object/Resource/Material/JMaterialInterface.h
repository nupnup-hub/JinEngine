#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h"
#include"../JResourceUserInterface.h"
#include"../../JFrameInterface.h"
#include"../../../Graphic/JGraphicTextureUserInterface.h"
#include"../../../Graphic/JGraphicBufInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JMaterialConstants;
	}

	class JMaterialInterface : public JResourceObject, 
		public JFrameInterface<Graphic::JMaterialConstants>,
		public JClearableInterface, 
		public JResourceUserInterface,
		public Graphic::JGraphicTextureUserInterface,
		public Graphic::JGraphicBufElementInterface
	{
	protected:
		JMaterialInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}