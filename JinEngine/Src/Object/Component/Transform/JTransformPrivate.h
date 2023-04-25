#pragma once
#include"../JComponentPrivate.h"

namespace JinEngine
{
	class JCamera;
	class JLight;
	class JRenderItem;
	class JTransform;  
	class JGameObject;
	class JFrameDirtyListener;
	class JTransformPrivate final : public JComponentPrivate
	{
	public:
		class AssetDataIOInterface final : public JComponentPrivate::AssetDataIOInterface
		{
		private:
			Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		};
		class CreateInstanceInterface final : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		private:
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		private:
			bool Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept;
		};
		class UpdateWorldInterface final
		{
		private: 
			friend class JGameObject;
		private:
			static void UpdateWorld(JTransform* transform)noexcept;
		};
		class FrameDirtyInterface final
		{
		private:
			friend class JCamera;
			friend class JLight;
			friend class JRenderItem;
		private:
			static void RegisterFrameDirtyListener(JTransform* transform, JFrameDirtyListener* listener)noexcept;
			static void DeRegisterFrameDirtyListener(JTransform* transform, JFrameDirtyListener* listener)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}