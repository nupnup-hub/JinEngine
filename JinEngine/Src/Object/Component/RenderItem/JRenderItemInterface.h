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
	}
	 
	class JRenderItemInterface : public JComponent,
		public JFrameUpdate<IFrameUpdate<Graphic::JObjectConstants&, Graphic::JBoundingObjectConstants&, const uint, const bool>, JFrameDirtyListener, FrameUpdate::hasFrameBuff>,
		public JFrameBuffUserInterface,
		public JResourceUserInterface
	{
	protected:
		JRenderItemInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}