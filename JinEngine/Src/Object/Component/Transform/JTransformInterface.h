#pragma once
#include"../JComponent.h"
#include"../../JFrameUpdate.h"

namespace JinEngine
{
	class JGameObject;
	class JRenderItem;
	namespace Graphic
	{
		struct JObjectConstants;
	} 
	class JTransformGameObjectInterface
	{
	private:
		friend class JGameObject;
	protected:
		virtual void ChangeParent() = 0;
	};

	class JTransformInterface : public JComponent, 
		public JFrameDirtyObserver<JFrameDirtyTrigger>,
		public JTransformGameObjectInterface
	{
	protected:
		JTransformInterface(const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner);
	};
}