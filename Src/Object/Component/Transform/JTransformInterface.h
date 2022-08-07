#pragma once
#include"../JComponent.h"
#include"../../JFrameInterface.h"

namespace JinEngine
{
	class JGameObject;
	class JRenderItem;
	namespace Graphic
	{
		struct JObjectConstants;
	}

	class JTransformInterface : public JComponent , public JShareFrameInterface<JRenderItem>
	{
	private:
		friend class JGameObject;
	protected:
		JTransformInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JGameObject* owner);
	protected:
		virtual void ChangeParent() = 0;
	};
}