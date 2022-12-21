#pragma once
#include"../JComponent.h" 
#include"../../JFrameUpdate.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceHandleInterface.h"
namespace JinEngine
{
	namespace Graphic
	{
		struct JLightConstants;
		struct JShadowMapLightConstants;
		struct JShadowMapConstants;
	}

	namespace
	{
		using ILitFrameUpdate = IFrameUpdate3 <IFrameUpdateBase<Graphic::JLightConstants&>,
			IFrameUpdateBase<Graphic::JShadowMapLightConstants&>,
			IFrameUpdateBase<Graphic::JShadowMapConstants&>>;
	}

	class JLightInterface : public JComponent,
		public JFrameUpdate<ILitFrameUpdate, JFrameDirtyListener, FrameUpdate::nonBuff>,
		public Graphic::JGraphicResourceHandleInterface
	{
	protected:
		JLightInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}