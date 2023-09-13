#pragma once
#include"../../../../Core/FSM/JFSMtransitionPrivate.h"
#include"../../../../Core/File/JFileIOResult.h" 

namespace JinEngine
{
	class JAnimationFSMstate;
	class JAnimationFSMstatePrivate;
	class JAnimationFSMtransition;
	class JAnimationFSMtransitionPrivate final : public Core::JFSMtransitionPrivate
	{
	public:
		class AssetDataIOInterface final
		{
		private:
			friend class JAnimationFSMstate;
			friend class JAnimationFSMstatePrivate;
		private:
			Core::J_FILE_IO_RESULT LoadAssetData(std::wifstream& stream, const JUserPtr<JAnimationFSMtransition>& trans);
			Core::J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMtransition>& trans);
		};
		class CreateInstanceInterface final : public JFSMtransitionPrivate::CreateInstanceInterface
		{
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData)final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class UpdateInterface final : public JFSMtransitionPrivate::UpdateInterface
		{
		private:
			void Initialize(const JUserPtr<Core::JFSMtransition>& trans)noexcept final;
		};
	public:
		JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JAnimationFSMtransitionPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept;
		JFSMtransitionPrivate::UpdateInterface& GetUpdateInterface()const noexcept final;
	};
}