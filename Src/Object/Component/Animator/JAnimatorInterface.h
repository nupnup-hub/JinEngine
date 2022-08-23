#pragma once
#include"../JComponent.h" 
#include"../../JFrameInterface.h"
#include"../../Resource/JResourceUserInterface.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JAnimationConstants;
	}
	class JAnimatorInterface : public JComponent, public JFrameInterface<Graphic::JAnimationConstants>, public JResourceUserInterface
	{
	protected:
		JAnimatorInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}