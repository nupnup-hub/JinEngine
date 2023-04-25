#pragma once
#include"../JResourceObjectPrivate.h" 

namespace JinEngine
{
	namespace Core
	{
		class JFSMdiagram;
		class JAnimationUpdateData;
	}
	namespace Graphic
	{
		struct JAnimationConstants;
	}
	class JAnimator;
	class JAnimationController;
	class JAnimationControllerPrivate final : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface final : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) final;
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
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		};
		class FrameUpdateInterface final
		{
		private:
			friend class JAnimator;
		private:
			static void Initialize(JAnimationController* aniCont, Core::JAnimationUpdateData* updateData)noexcept;
			static void Update(JAnimationController* aniCont, Core::JAnimationUpdateData* updateData, Graphic::JAnimationConstants& constant)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}