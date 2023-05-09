#pragma once
#include"JAnimationFSMstatePrivate.h"
#include<vector>
namespace JinEngine
{ 
	namespace Core
	{ 
		class JAnimationFSMstateClipPrivate : public JAnimationFSMstatePrivate
		{
		public:
			class AssetDataIOInterface final: public JAnimationFSMstatePrivate::AssetDataIOInterface
			{
			private:
				J_FILE_IO_RESULT LoadAssetData(std::wifstream& stream, const JUserPtr<JAnimationFSMstate>& state) final;
				J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMstate>& state) final;
			};
			class CreateInstanceInterface final : public JAnimationFSMstatePrivate::CreateInstanceInterface
			{
			private:
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)final;
				void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept final; 
				bool CanCreateInstance(JDITypeDataBase* initData)const noexcept final;
			};
			class DestroyInstanceInterface final : public JAnimationFSMstatePrivate::DestroyInstanceInterface
			{
			protected:
				void Clear(JIdentifier* ptr, const bool isForced)final; 
			};
			class UpdateInterface final : public JAnimationFSMstatePrivate::UpdateInterface
			{
			private:
				void Initialize(const JUserPtr<JFSMstate>& state)noexcept final;
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
}