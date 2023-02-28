#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h"  
#include"../../../Core/FSM/JFSMownerInterface.h"

namespace JinEngine
{
	class JAnimator;
	class JSkeletonAsset;
	namespace Core
	{
		class JFSMdiagram;
		class JGameTimer;
		class JAnimationUpdateData;
		struct JAnimationTime;
	}
	namespace Graphic
	{
		struct JAnimationConstants;
	}

	class JAnimationControllerFrameUpdateInterface
	{
	private:
		friend class JAnimator;
	protected:
		virtual ~JAnimationControllerFrameUpdateInterface() = default;
	public:
		virtual JAnimationControllerFrameUpdateInterface* FrameUpdateInterface() = 0;
	private:
		virtual void Initialize(Core::JAnimationUpdateData* updateData)noexcept = 0;
		virtual void Update(Core::JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept = 0;
	};

	class JAnimationControllerInterface : public JResourceObject, 
		public JClearableInterface,
		public Core::JFSMdiagramOwnerInterface,
		public JAnimationControllerFrameUpdateInterface
	{
	protected:
		JAnimationControllerInterface(const JResourceObject::JResourceInitData& initdata);
	};
}