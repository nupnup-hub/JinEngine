#pragma once
#include"JAnimationFSMstatePrivate.h" 

namespace JinEngine
{  
	class JAnimationFSMstateClipPrivate : public JAnimationFSMstatePrivate
	{
	public:
		class AssetDataIOInterface final : public JAnimationFSMstatePrivate::AssetDataIOInterface
		{
		private:
			Core::J_FILE_IO_RESULT LoadAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) final;
			Core::J_FILE_IO_RESULT StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) final;
		};
		class CreateInstanceInterface final : public JAnimationFSMstatePrivate::CreateInstanceInterface
		{
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData)final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class DestroyInstanceInterface final : public JAnimationFSMstatePrivate::DestroyInstanceInterface
		{
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced)final;
		};
		class UpdateInterface final : public JAnimationFSMstatePrivate::UpdateInterface
		{
		private:
			void Initialize(const JUserPtr<Core::JFSMstate>& state)noexcept final;
			void Enter(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
			void Update(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
			void Close(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData)noexcept final;
			void GetRegisteredSkeleton(const JUserPtr<JAnimationFSMstate>& state, std::vector<JUserPtr<JSkeletonAsset>>& skeletonVec)noexcept final;
		};
	public:
		JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JAnimationFSMstatePrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
		JFSMstatePrivate::UpdateInterface& GetUpdateInterface()const noexcept final;
	};
}