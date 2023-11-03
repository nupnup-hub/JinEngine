#pragma once
#include "../../../../Core/FSM/JFSMdiagramPrivate.h" 
#include"../../../../Core/File/JFileIOResult.h"

namespace JinEngine
{
	class JAnimationController;
	namespace Graphic
	{
		struct JAnimationConstants;
	}
	namespace Core
	{
		class JFSMdiagramOwnerInterface;
	}
	class JAnimationUpdateData;
	class JAnimationFSMdiagram;
	class JFileIOTool;
	class JAnimationFSMdiagramPrivate final : public Core::JFSMdiagramPrivate
	{
	public:
		class AssetDataIOInterface
		{
		private:
			friend class JAnimationController;
		private:
			static JUserPtr<JAnimationFSMdiagram> LoadAssetData(JFileIOTool& tool, Core::JFSMdiagramOwnerInterface* fsmOwner);	//load diagram data and create state
			static Core::J_FILE_IO_RESULT StoreAssetData(JFileIOTool& tool, const JUserPtr<JAnimationFSMdiagram>& diagram);
		};
		class CreateInstanceInterface final : public Core::JFSMdiagramPrivate::CreateInstanceInterface
		{
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData)final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class UpdateInterface final
		{
		private:
			friend class JAnimationController;
		private:
			static void Initialize(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber)noexcept;
			static void Enter(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, const uint layerNumber);
			static void Update(const JUserPtr<JAnimationFSMdiagram>& diagram, JAnimationUpdateData* updateData, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
	};
}