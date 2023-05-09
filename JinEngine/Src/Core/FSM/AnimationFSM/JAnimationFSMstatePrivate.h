#pragma once
#include"../JFSMstatePrivate.h"
#include"../../File/JFileIOResult.h"
#include"../../../Utility/JVector.h"
#include<vector>

namespace JinEngine
{
	class JSkeletonAsset;
	namespace Core
	{
		class JAnimationFSMdiagram;
		class JAnimationFSMstate;
		class JAnimationUpdateData;
		class JAnimationFSMstatePrivate : public JFSMstatePrivate
		{
		public:
			class AssetDataIOInterface
			{
			private:
				friend class JAnimationFSMdiagram;
			private:
				virtual J_FILE_IO_RESULT LoadAssetData(std::wifstream& stream, const JUserPtr<JAnimationFSMstate>& state) = 0;	//load state data and create transition
				virtual J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMstate>& state) = 0;
			protected:
				static J_FILE_IO_RESULT LoadAssetCommonData(std::wifstream& stream, const JUserPtr<JAnimationFSMstate>& state);
				static J_FILE_IO_RESULT StoreAssetCommonData(std::wofstream& stream, const JUserPtr<JAnimationFSMstate>& state);
			};
			class CreateInstanceInterface : public JFSMstatePrivate::CreateInstanceInterface
			{ 
			protected:
				void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept override;
			};
			class UpdateInterface : public JFSMstatePrivate::UpdateInterface
			{
			private:
				friend class JAnimationFSMdiagram;
			private: 
				virtual void Enter(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept = 0;
				virtual void Update(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept = 0;
				virtual void Close(const JUserPtr<JAnimationFSMstate>& state, JAnimationUpdateData* updateData)noexcept = 0;
				virtual void GetRegisteredSkeleton(const JUserPtr<JAnimationFSMstate>& state, std::vector<JUserPtr<JSkeletonAsset>>& skeletonVec)noexcept = 0;
			};
			class EditorInterface
			{
			private:
				static JVector2<float> GetPos(const JUserPtr<JAnimationFSMstate>& state) noexcept;
				static void SetPos(const JUserPtr<JAnimationFSMstate>& state, const JVector2<float>& newPos)noexcept;
			};
		public:
			virtual JAnimationFSMstatePrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept = 0; 
		};
	}
}