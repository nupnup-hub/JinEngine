#pragma once  
#include"JGameObjectInterface.h" 
#include"../Component/JComponent.h"
#include"../../Utility/JTypeUtility.h"
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
	class JLight;

	class JGameObject final : public JGameObjectInterface
	{
		REGISTER_CLASS(JGameObject) 
	private: 
		std::vector<JGameObject*> children;
		JGameObject* parent = nullptr;
		std::vector<JComponent*> component; 
		std::vector<JBehavior*> behavior;
		JTransform* transform = nullptr;
		JAnimator* animator = nullptr;
		JRenderItem* renderItem = nullptr;
		JScene* ownerScene = nullptr; 
	public:
		JAnimator* GetAnimator()const noexcept;
		JTransform* GetTransform() const noexcept;
		JRenderItem* GetRenderItem()const noexcept;
		std::vector<JComponent*> GetComponentVec()const noexcept;
		JScene* GetOwnerScene()const noexcept;
		uint GetChildrenCount()const noexcept;  
		uint GetComponentCount()const noexcept;
		uint GetComponentCount(const J_COMPONENT_TYPE type)const noexcept;
		JGameObject* GetParent()noexcept;
		JGameObject* GetChild(const uint index)noexcept;
		J_OBJECT_TYPE GetObjectType()const noexcept final;

		void SetName(const std::wstring& newName)noexcept final;

		bool IsRoot()const noexcept;
		bool HasComponent(const J_COMPONENT_TYPE type)const noexcept;
		bool HasRenderItem()const noexcept;
		bool HasAnimator()const noexcept; 
		bool CanAddComponent(const J_COMPONENT_TYPE type)const noexcept;
		void ChangeParent(JGameObject* newParent)noexcept;
		JComponent* FindComponent(const size_t guid)const noexcept;
	public:
		JGameObjectCompInterface* CompInterface() final;
	public:
		void DoCopy(JObject* ori) final;
	private:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		static bool HasSameName(_In_ JGameObject* parent, _In_ const std::wstring& initName) noexcept;
		bool IsChild(JGameObject* obj)noexcept;
		bool AddComponent(JComponent& component)noexcept final;
		bool RemoveComponent(JComponent& component)noexcept final;
	private:
		bool Destroy() final; 
		void Clear();
	private:
		bool RegisterCashData()noexcept final;
		bool DeRegisterCashData()noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreGameObject(std::wofstream& stream) final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JGameObject* gameObject);
		static JGameObject* LoadObject(std::wifstream& stream, JGameObject* parent, JScene* owenerScene);
		static void RegisterJFunc();
	private:
		/*
		* JGameObject Register Parent and initialize JTransform
		* Caution! don't register in parentChildren
		* only ownerScene parameter is valid by root gameobject
		*/
		JGameObject(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* parent, JScene* ownerScene = nullptr);
		~JGameObject();
#pragma region JComponent Template
	public:
		template<typename T>
		auto GetComponent()const noexcept -> typename Core::TypeCondition<T*, std::is_base_of_v<JComponent, T>>::Type
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
			else if constexpr (std::is_same_v<JAnimator, T>)
				return animator;
			else if constexpr (std::is_same_v<JTransform, T>)
				return transform;
			else if constexpr (std::is_same_v<JRenderItem, T>)
				return renderItem;
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
		auto GetComponents()const noexcept -> std::vector<typename  Core::TypeCondition<T*, std::is_base_of_v<JComponent, T>>::Type>
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
			else if constexpr (std::is_same_v<JAnimator, T>)
				return animator;
			else if constexpr (std::is_same_v<JTransform, T>)
				return transform;
			else if constexpr (std::is_same_v<JRenderItem, T>)
				return renderItem;
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
		auto GetExceptedComponents()const noexcept -> std::vector<typename Core::TypeCondition<JComponent*, std::is_base_of_v<JComponent, ExceptParam...>>::Type>
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
		auto GetComponentWithParent()const noexcept -> typename Core::TypeCondition<T*, std::is_base_of_v<JComponent, T>>::Type
		{
			const JGameObject* nowParent = this;
			while (nowParent != nullptr)
			{
				T* res = nowParent->GetComponent<T>();
				if (res != nullptr)
					return res;
				nowParent = nowParent->parent;
			}
			return nullptr;
		}
#pragma endregion
	};
}

