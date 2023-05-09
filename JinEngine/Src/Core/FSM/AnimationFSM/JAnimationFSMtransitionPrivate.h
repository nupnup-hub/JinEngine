#pragma once
#include"../JFSMtransitionPrivate.h"
#include"../../File/JFileIOResult.h" 

namespace JinEngine
{
	namespace Core
	{
		class JAnimationFSMstate;
		class JAnimationFSMstatePrivate;
		class JAnimationFSMtransition;
		class JAnimationFSMtransitionPrivate final: public JFSMtransitionPrivate
		{
		public:
			class AssetDataIOInterface final
			{
			private:
				friend class JAnimationFSMstate;
				friend class JAnimationFSMstatePrivate;
			private:
				J_FILE_IO_RESULT LoadAssetData(std::wifstream& stream, const JUserPtr<JAnimationFSMtransition>& trans);
				J_FILE_IO_RESULT StoreAssetData(std::wofstream& stream, const JUserPtr<JAnimationFSMtransition>& trans);
			};
			class CreateInstanceInterface final : public JFSMtransitionPrivate::CreateInstanceInterface
			{
			private:
				JOwnerPtr<JIdentifier> Create(JDITypeDataBase* initData)final;
				void Initialize(JIdentifier* createdPtr, JDITypeDataBase* initData)noexcept final;
				bool CanCreateInstance(JDITypeDataBase* initData)const noexcept final;
			};
			class UpdateInterface final : public JFSMtransitionPrivate::UpdateInterface
			{
			private:
				void Initialize(const JUserPtr<JFSMtransition>& trans)noexcept final;
			};
		public:
			JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
			JAnimationFSMtransitionPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept;
			JFSMtransitionPrivate::UpdateInterface& GetUpdateInterface()const noexcept final;
		};
	}
}