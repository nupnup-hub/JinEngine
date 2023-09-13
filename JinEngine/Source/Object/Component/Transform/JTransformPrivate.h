#pragma once
#include"../JComponentPrivate.h"

namespace JinEngine
{
	class JCamera;
	class JLight;
	class JDirectionalLight;
	class JPointLight;
	class JSpotLight;
	class JRenderItem;
	class JTransform;  
	class JGameObject;
	namespace Graphic
	{
		class JFrameDirtyTriggerBase;
	}
	class JTransformPrivate final : public JComponentPrivate
	{
	public:
		class AssetDataIOInterface final : public JComponentPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		};
		class CreateInstanceInterface final : public JComponentPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface; 
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		private:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept;
		};
		class UpdateWorldInterface final
		{
		private: 
			friend class JGameObject;
		private:
			static void UpdateWorld(const JUserPtr<JTransform>& transform)noexcept;
		};
		class FrameDirtyInterface final
		{
		private:
			friend class JCamera;
			friend class JLight;
			friend class JRenderItem;
		private:
			//JFrameDirtyListener is impl class
			static void RegisterFrameDirtyListener(JTransform* transform, Graphic::JFrameDirtyTriggerBase* listener, const size_t guid)noexcept;
			static void DeRegisterFrameDirtyListener(JTransform* transform, const size_t guid)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JComponentPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}