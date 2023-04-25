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
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(JScene* invoker, std::wifstream& stream);
			static std::unique_ptr<Core::JDITypeDataBase> CreateStoreAssetDIData(JGameObject* root, std::wofstream& stream);
		private:
			static Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data);
			static Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data);
		};
		class CreateInstanceInterface final: public JObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
			void RegisterCash(Core::JIdentifier* createdPtr)noexcept final;
			void SetValidInstance(Core::JIdentifier* createdPtr)noexcept final;
		private:
			bool Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept; 
		};
		class DestroyInstanceInterface final : public Core::JIdentifierPrivate::DestroyInstanceInterface
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
			static bool AddComponent(JComponent* component)noexcept;
			static bool RemoveComponent(JComponent* component)noexcept;
		};
		class ActivateInterface final
		{
		private:
			friend class JScene;
		private:
			static void Activate(JGameObject* ptr)noexcept;
			static void DeActivate(JGameObject* ptr)noexcept;
		};
		class SelectInterface final
		{
		private:
			friend class Editor::JEditorWindow;
		private:
			static void Select(JGameObject* ptr)noexcept;
			static void DeSelect(JGameObject* ptr)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		Core::JIdentifierPrivate::DestroyInstanceInterface& GetDestroyInstanceInterface()const noexcept final;
	};
}