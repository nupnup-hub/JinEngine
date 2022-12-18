#pragma once
#include"../JComponent.h" 
#include"../../JFrameUpdate.h"
#include"../../Resource/JResourceUserInterface.h"
#include"../../../Graphic/JGraphicBufInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JObjectConstants;
		struct JBoundingObjectConstants;
		struct JOcclusionObjectConstants;
	}

	namespace
	{
		using IRitemFrameUpdate = IFrameUpdate2<IFrameUpdateBase<Graphic::JObjectConstants&, const uint>,
			IFrameUpdateBase<Graphic::JBoundingObjectConstants&>>;
	}

	class JRenderItemInterface : public JComponent,
		public JFrameUpdate<IRitemFrameUpdate, JFrameDirtyListener, FrameUpdate::dobuleBuff>,
		public JFrameBuffUserInterface,
		public JResourceUserInterface
	{
	protected:
		JRenderItemInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}