#pragma once
#include"../JResourceObjectPrivate.h" 

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;
	}
	namespace Graphic
	{
		struct JAnimationConstants;
	}
	class JAnimator;
	class JAnimationController;
	class JAnimationUpdateData;
	class JAnimationControllerPrivate final : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface final : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		private:
			Core::J_FILE_IO_RESULT LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)final;	//use clipMetaData
			Core::J_FILE_IO_RESULT StoreMetaData(Core::JDITypeDataBase* data)final;	//use storeData	 
		};
		class CreateInstanceInterface final : public JResourceObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class FrameUpdateInterface final
		{
		private:
			friend class JAnimator;
		private:
			static void Initialize(JAnimationController* aniCont, JAnimationUpdateData* updateData)noexcept;
			static void Update(JAnimationController* aniCont, JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}