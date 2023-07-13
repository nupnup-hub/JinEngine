#pragma once
#include"../JObjectPrivate.h" 
 
namespace JinEngine
{
	class JComponent;
	class JScene;
	class JGameObject;

	namespace Editor
	{
		class JEditorWindow;
	}
	class JGameObjectPrivate final : public JObjectPrivate
	{
	public:
		class AssetDataIOInterface final
		{
		private:
			friend class JScene;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(JUserPtr<JScene> invoker, std::wifstream& stream);
			static std::unique_ptr<Core::JDITypeDataBase> CreateStoreAssetDIData(JUserPtr<JGameObject> root, std::wofstream& stream);
		private:
			static JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data);
			static Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data);
		};
		class CreateInstanceInterface final: public JObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			void RegisterCash(Core::JIdentifier* createdPtr)noexcept final;
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept final;
		private:
			bool Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept;
		};
		class DestroyInstanceInterface final : public JObjectPrivate::DestroyInstanceInterface
		{ 
		private:
			void Clear(Core::JIdentifier* ptr, const bool isForced) final;  
			void SetInvalidInstance(Core::JIdentifier* ptr)noexcept final; 
			void DeRegisterCash(Core::JIdentifier* ptr)noexcept final;
		}; 
		class OwnTypeInterface final
		{
		private:
			friend class JComponent;
		private:
			static bool AddComponent(const JUserPtr<JComponent>& component)noexcept;
			static bool RemoveComponent(const JUserPtr<JComponent>& component)noexcept;
		};
		class ActivateInterface final
		{
		private:
			friend class JScene;
		private:
			static void Activate(const JUserPtr<JGameObject>& ptr)noexcept;
			static void DeActivate(const JUserPtr<JGameObject>& ptr)noexcept;
		};
		class SelectInterface final
		{
		private:
			friend class Editor::JEditorWindow;
		private:
			static void Select(const JUserPtr<JGameObject>& ptr)noexcept;
			static void DeSelect(const JUserPtr<JGameObject>& ptr)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
	};
}