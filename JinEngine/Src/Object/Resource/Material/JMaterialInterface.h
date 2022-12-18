#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h"
#include"../JResourceUserInterface.h"
#include"../../JFrameUpdate.h"
#include"../../../Graphic/JGraphicTextureUserInterface.h"
#include"../../../Graphic/JGraphicBufInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JMaterialConstants;
	}
	class JResourceManagerImpl;
	class JRenderItem;

	class JMaterialInterface : public JResourceObject,
		public JFrameUpdate<IFrameUpdate1<IFrameUpdateBase<Graphic::JMaterialConstants&>>, JFrameDirty, FrameUpdate::singleBuff>,
		public JResourceUserInterface,
		public Graphic::JGraphicTextureUserInterface
	{
	protected:
		JMaterialInterface(const JResourceObject::JResourceInitData& initdata);
	};
}