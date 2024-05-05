#pragma once
#include"../../../../Core/FSM/JFSMstatePrivate.h"
#include"../../../../Core/File/JFileIOResult.h"
#include"../../../../Core/Math/JVector.h" 

namespace JinEngine
{
	class JSkeletonAsset;
	class JAnimationFSMdiagram;
	class JAnimationFSMstate;
	class JAnimationUpdateData;

	class JFileIOTool;
	class JAnimationFSMstatePrivate : public Core::JFSMstatePrivate
	{
	public:
		class AssetDataIOInterface
		{
		private:
			friend class JAnimationFSMdiagram;
		private:
			virtual Core::J_FILE_IO_RESULT LoadAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) = 0;	//load state data and create transition
			virtual Core::J_FILE_IO_RESULT StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state) = 0;
		protected:
			static Core::J_FILE_IO_RESULT LoadAssetCommonData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state);
			static Core::J_FILE_IO_RESULT StoreAssetCommonData(JFileIOTool& tool, const JUserPtr<JAnimationFSMstate>& state);
		};
		class CreateInstanceInterface : public JFSMstatePrivate::CreateInstanceInterface
		{
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;
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