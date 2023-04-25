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
				J_FILE_IO_RESULT LoadAssetData(std::wifstream& stream, JAnimationFSMstate* state) final;
				J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, JAnimationFSMstate* state) final;
			};
			class CreateInstanceInterface final : public JAnimationFSMstatePrivate::CreateInstanceInterface
			{
			private:
				JOwnerPtr<JIdentifier> Create(std::unique_ptr<JDITypeDataBase>&& initData)final;
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
				void Initialize(JFSMstate* state)noexcept final;
				void Enter(JAnimationFSMstate* state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
				void Update(JAnimationFSMstate* state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept final;
				void Close(JAnimationFSMstate* state, JAnimationUpdateData* updateData)noexcept final;
				void GetRegisteredSkeleton(JAnimationFSMstate* state, std::vector<JSkeletonAsset*>& skeletonVec)noexcept final;
			}; 
		public:
			Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
			Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
			JAnimationFSMstatePrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
			JFSMstatePrivate::UpdateInterface& GetUpdateInterface()const noexcept final;
		};
	}
}