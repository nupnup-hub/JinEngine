#pragma once 
#include"JAnimationFSMstate.h" 

namespace JinEngine
{ 
	class JAnimationClip;
	namespace Core
	{
		struct JAnimationTime;
		class JAnimationFSMstateClipPrivate;
		class JAnimationFSMdiagram;
		class JAnimationFSMstateClip final : public JAnimationFSMstate
		{
			REGISTER_CLASS_IDENTIFIER_LINE(JAnimationFSMstateClip)
		public: 
			class InitData : public JAnimationFSMstate::InitData
			{
				REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
			public:
				InitData(JUserPtr<JAnimationFSMdiagram> ownerDiagram);
				InitData(const std::wstring& name, const size_t guid, JUserPtr<JAnimationFSMdiagram> ownerDiagram);
			};
		private:
			friend class JAnimationFSMstateClipPrivate;
			class JAnimationFSMstateClipImpl;
		private:
			std::unique_ptr<JAnimationFSMstateClipImpl> impl;
		public: 
			Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
			J_ANIMATION_STATE_TYPE GetStateType()const noexcept final; 
			Core::JUserPtr<JAnimationClip> GetClip()const noexcept;
		public:
			void SetClip(Core::JUserPtr<JAnimationClip> newClip)noexcept;
		public:
			bool CanLoop()const noexcept final;   
		private:
			JAnimationFSMstateClip(const InitData& initData);
			~JAnimationFSMstateClip();
			JAnimationFSMstateClip(const JAnimationFSMstateClip& rhs) = delete;
			JAnimationFSMstateClip& operator=(const JAnimationFSMstateClip& rhs) = delete;
		};
	}
}

/*]
#pragma once
#include"JAnimationFSMstate.h"
#include"JAnimationTime.h"
#include"../../../Object/Resource/JResourceObjectUserInterface.h"
#include"../../../Object/Resource/AnimationClip/JAnimationClip.h"

namespace JinEngine
{
	namespace Core
	{
		struct JAnimationTime;
		class JAnimationFSMstateClip final : public JAnimationFSMstate, public JResourceObjectUserInterface
		{
			REGISTER_CLASS(JAnimationFSMstateClip)
		private:
			REGISTER_PROPERTY_EX(clip, GetClip, SetClip, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::ICON, true))
			Core::JUserPtr<JAnimationClip> clip;
		public:
			void Initialize()noexcept final;
			J_ANIMATION_STATE_TYPE GetStateType()const noexcept final;
		public:
			void Enter(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
			void Update(JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
			void Close(JAnimationUpdateData* updateData)noexcept final;
		public:
			void GetRegisteredSkeleton(std::vector<JSkeletonAsset*>& skeletonVec)noexcept final;
			Core::JUserPtr<JAnimationClip> GetClip()const noexcept;
			void SetClip(Core::JUserPtr<JAnimationClip> newClip)noexcept;
		public:
			bool CanLoop()const noexcept final;
		protected:
			void Clear()noexcept final;
		private:
			void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
		private:
			J_FILE_IO_RESULT StoreData(std::wofstream& stream)final;
			J_FILE_IO_RESULT LoadData(std::wifstream& stream)final;
		private:
			static void RegisterCallOnce();
		private:
			JAnimationFSMstateClip(const JFSMstateInitData& initData);
			~JAnimationFSMstateClip();
			JAnimationFSMstateClip(const JAnimationFSMstateClip& rhs) = delete;
			JAnimationFSMstateClip& operator=(const JAnimationFSMstateClip& rhs) = delete;
		};
	}
}
*/