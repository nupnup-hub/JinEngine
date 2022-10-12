#pragma once
#include"../JComponent.h" 
#include"../../JFrameUpdate.h"
#include"../../Resource/JResourceUserInterface.h"

namespace JinEngine
{
	class JScene;
	namespace Graphic
	{
		struct JAnimationConstants;
	}
	class JAnimatorInterface : public JComponent, 
		public JFrameUpdate<IFrameUpdate<Graphic::JAnimationConstants&>, JFrameDirty, false>,
		public JResourceUserInterface
	{
	protected:
		JAnimatorInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}