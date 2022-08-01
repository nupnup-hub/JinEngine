#pragma once  
#include"JGameObjectInterface.h"
#include"IGameObjectComponentEvent.h"
#include"GameObjectDirty.h" 
#include"../Component/JComponent.h"
#include<type_traits>
#include<memory>
#include<vector> 

namespace JinEngine
{ 
	class JComponent;
	class JAnimator;
	class JBehavior;
	class JCamera;
	class JRenderItem;
	class JTransform;
	class JScene;
	class ISceneGameObjectEvent;
	class JLight;

	class JGameObject : public JGameObjectInterface, public IGameObjectComponentEvent
	{
		REGISTER_CLASS(JGameObject)
		//friend class JScene; 
	private:
		std::unique_ptr<GameObjectDirty> gameObjectDirty;
		std::vector<JGameObject*> children;
		JGameObject* parent = nullptr;
		std::vector<JComponent*> component; 
		std::vector<JBehavior*> behavior;
		JTransform* transform = nullptr;
		JAnimator* animator = nullptr;
		JRenderItem* renderItem = nullptr;
		JScene* ownerScene = nullptr;
		ISceneGameObjectEvent* sceneGameObjectEvent = nullptr;
	public:
		JAnimator* GetAnimator()noexcept;
		JTransform* GetTransform() noexcept;
		JRenderItem* GetRenderItem()noexcept; 
		JScene* GetOwnerScene()noexcept;
		GameObjectDirty* GetGameObjectDirty()noexcept;
		uint GetChildrenCount()const noexcept;  
		uint GetComponentCount()const noexcept;
		uint GetComponentCount(const J_COMPONENT_TYPE type)noexcept;
		JGameObject* GetParent()noexcept;
		JGameObject* GetChild(const uint index)noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
   
		bool IsRoot()const noexcept;
		bool HasComponent(const J_COMPONENT_TYPE type)noexcept;
		bool HasRenderItem()const noexcept;
		bool HasAnimator()const noexcept;  
		void ChangeParent(JGameObject* newParent)noexcept;
	protected:
		void DoActivate()noexcept override;
		void DoDeActivate()noexcept override; 
	private:
		static bool HasSameName(_In_ JGameObject* parent, _In_ const std::string& initName) noexcept;
		bool IsChild(JGameObject* obj)noexcept;
		JComponent* AddComponent(JComponent* component)noexcept;
		bool EraseComponent(JComponent* component)noexcept;

		//IGameObjectComponentEvent
		bool RegisterAnimator(JAnimator* animator)noexcept;
		bool RegisterCamera(JCamera* camera)noexcept;
		bool RegisterLight(JLight* light)noexcept;
		bool RegisterRenderItem(JRenderItem* renderItem)noexcept;
		bool RegisterShadowLight(JLight* light)noexcept;

		bool DeRegisterAnimator(JAnimator* animator)noexcept;
		bool DeRegisterCamera(JCamera* camera)noexcept;
		bool DeRegisterLight(JLight* light)noexcept;
		bool DeRegisterRenderItem(JRenderItem* renderItem)noexcept;
		bool DeRegisterShadowLight(JLight* light)noexcept;

		bool ReRegisterAnimator(JAnimator* animator)noexcept;
		bool ReRegisterRenderItem(JRenderItem* renderItem)noexcept;
		JCamera* SetMainCamera(JCamera* camera)noexcept;		
		void UpdateGameObjectTransform()noexcept;

	public:
		static void EraseGameObject(const size_t guid)noexcept;
		static void EraseGameObject(JGameObject* gameObject)noexcept;
	private:
		static void EraseGameObjectChildren(JGameObject* parent)noexcept;
		Core::J_FILE_IO_RESULT CallStoreGameObject(std::wofstream& stream) final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JGameObject* gameObject);
		static JGameObject* LoadObject(std::wifstream& stream, JGameObject* parent);
		static void RegisterFunc();
	private:
		/*
		* JGameObject Register Parent and initialize JTransform
		* Caution! don't register in parentChildren
		* only ownerScene parameter is valid by root gameobject
		*/
		JGameObject(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JGameObject* parent, JScene* ownerScene = nullptr);
		~JGameObject();
	private:
		template<typename T, bool res>
		struct ToPtr;
		template<typename T>
		struct ToPtr<T, true>
		{
		public:
			using Ptr = T*;
		};
#pragma region JComponent Template
	public:
		template<typename T>
		auto GetComponent()const noexcept -> typename ToPtr<T, std::is_base_of_v<JComponent, T>>::Ptr
		{
			const uint componentCount = (uint)component.size();
			if constexpr (std::is_base_of_v<JBehavior, T>)
			{
				const std::string componentName = T::TypeName();
				for (uint i = 0; i < componentCount; ++i)
				{
					if (component[i]->TypeName() == componentName)
						return static_cast<T*>(component[i]);
				}
				return nullptr;
			}
			else
			{
				const J_COMPONENT_TYPE componentType = T::GetStaticComponentType();
				for (uint i = 0; i < componentCount; ++i)
				{
					if (component[i]->GetComponentType() == componentType)
						return static_cast<T*>(component[i]);
				}
				return nullptr;
			}
		} 
		template<typename T>
		auto GetComponents()const noexcept -> std::vector<typename ToPtr<T, std::is_base_of_v<JComponent, T>>::Ptr>
		{
			std::vector<T*> res;
			const uint componentCount = (uint)component.size();
			if constexpr (std::is_base_of_v<JBehavior, T>)
			{
				const std::string componentName = T::TypeName();
				for (uint i = 0; i < componentCount; ++i)
				{
					if (component[i]->TypeName() == componentName)
						res.push_back(static_cast<T*>(component[i]));
				}
				return res;
			}
			else
			{
				const J_COMPONENT_TYPE componentType = T::GetStaticComponentType();
				for (uint i = 0; i < componentCount; ++i)
				{
					if (component[i]->GetComponentType() == componentType)
						res.push_back(static_cast<T*>(component[i]));
				}
				return res;
			}
		}
		template<typename T>
		bool IsSameType(const std::string typeName)
		{
			return T::TypeName() == typeName;
		} 
		template<typename ...ExceptParam>
		auto GetExceptedComponents()const noexcept -> std::vector<typename ToPtr<JComponent, std::is_base_of_v<JComponent, ExceptParam...>>::Ptr>
		{
			std::vector<JComponent*> res;
			const uint componentCount = (uint)component.size();
			if constexpr (sizeof...(ExceptParam) == 0 || ((std::is_base_of_v<JBehavior, ExceptParam>) && ...))
			{
				res.resize(component.size());
				for (uint i = 0; i < componentCount; ++i)
					res[i] = component[i];
			}
			else
			{
				for (uint i = 0; i < componentCount; ++i)
				{
					const std::string componentName = component[i]->TypeName();
					if (((ExceptParam::TypeName() != componentName) && ...))
						res.push_back(component[i]);
				}
			}
			return res;
		}
		template<typename T>
		auto GetComponentWithParent()const noexcept -> typename ToPtr<T, std::is_base_of_v<JComponent, T>>::Ptr
		{
			if constexpr (std::is_base_of_v<JBehavior, T>)
			{
				const std::string componentName = T::TypeName();
				JGameObject* nowParent = parent;
				while (nowParent != nullptr)
				{
					const uint componentCount = (uint)nowParent->component.size();
					for (uint i = 0; i < componentCount; ++i)
					{
						if (nowParent->component[i]->TypeName() == componentName)
							return static_cast<T*>(nowParent->component[i]);
					}
					nowParent = nowParent->GetParent();
				}
				return nullptr;
			}
			else
			{
				const J_COMPONENT_TYPE componentType = T::GetStaticComponentType();
				JGameObject* nowParent = parent;
				while (nowParent != nullptr)
				{
					const uint componentCount = (uint)nowParent->component.size();
					for (uint i = 0; i < componentCount; ++i)
					{
						if (nowParent->component[i]->GetComponentType() == componentType)
							return static_cast<T*>(nowParent->component[i]);
					}
					nowParent = nowParent->GetParent();
				}
				return nullptr;
			}
		}
		template<typename T>
		auto GetComponentsWithChildren()const noexcept -> std::vector<typename ToPtr<T, std::is_base_of_v<JComponent, T>>::Ptr>
		{
			std::vector<T*> res;
			const uint componentCount = (uint)component.size();
			if constexpr (std::is_base_of_v<JBehavior, T>)
			{
				const std::string componentName = T::TypeName();
				for (uint i = 0; i < componentCount; ++i)
				{
					if (component[i]->TypeName() == componentName)
						res.push_back(static_cast<T*>(component[i]));
				}
				return res;
			}
			else
			{ 
				const J_COMPONENT_TYPE componentType = T::GetStaticComponentType();
				for (uint i = 0; i < componentCount; ++i)
				{
					if (component[i]->GetComponentType() == componentType)
						res.push_back(static_cast<T*>(component[i]));
				}
				return res;
			}
		}
#pragma endregion
	};
}

