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
				virtual J_FILE_IO_RESULT LoadAssetData(std::wifstream& stream, JAnimationFSMstate* state) = 0;	//load state data and create transition
				virtual J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, JAnimationFSMstate* state) = 0;
			protected:
				static J_FILE_IO_RESULT LoadAssetCommonData(std::wifstream& stream, JAnimationFSMstate* state);
				static J_FILE_IO_RESULT StoreAssetCommonData(std::wofstream& stream, JAnimationFSMstate* state);
			};
			class UpdateInterface : public JFSMstatePrivate::UpdateInterface
			{
			private:
				friend class JAnimationFSMdiagram;
			private: 
				virtual void Enter(JAnimationFSMstate* state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept = 0;
				virtual void Update(JAnimationFSMstate* state, JAnimationUpdateData* updateData, const uint layerNumber, const uint updateNumber)noexcept = 0;
				virtual void Close(JAnimationFSMstate* state, JAnimationUpdateData* updateData)noexcept = 0;
				virtual void GetRegisteredSkeleton(JAnimationFSMstate* state, std::vector<JSkeletonAsset*>& skeletonVec)noexcept = 0;
			};
			class EditorInterface
			{
			private:
				static JVector2<float> GetPos(JAnimationFSMstate* state) noexcept;
				static void SetPos(JAnimationFSMstate* state, const JVector2<float>& newPos)noexcept;
			};
		public:
			virtual JAnimationFSMstatePrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept = 0; 
		};
	}
}