#pragma once
#include"../JComponent.h" 
#include"../../JFrameUpdate.h"
#include"../../Resource/JResourceUserInterface.h"

namespace JinEngine
{
	class JScene;
	namespace Core
	{
		class JGameTimer;
	}
	namespace Graphic
	{
		struct JAnimationConstants;
	}
 
	class JAnimatorFrameUpdateTriggerInterface
	{
	private:
		friend class JScene;
	protected:
		virtual ~JAnimatorFrameUpdateTriggerInterface() = default;
	public:
		virtual JAnimatorFrameUpdateTriggerInterface* UpdateTriggerInterface() = 0;
	private:
		virtual void OnAnimationUpdate(Core::JGameTimer* sceneTimer)noexcept = 0;
		virtual	void OffAnimationUpdate()noexcept = 0;
	};

	class JAnimatorInterface : public JComponent, 
		public JFrameUpdate<IFrameUpdate1<IFrameUpdateBase<Graphic::JAnimationConstants&>>, Core::EmptyType, FrameUpdate::nonBuff>,
		public JResourceUserInterface,
		public JAnimatorFrameUpdateTriggerInterface
	{
	protected:
		JAnimatorInterface(const std::string& cTypeName, size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner);
	};
}