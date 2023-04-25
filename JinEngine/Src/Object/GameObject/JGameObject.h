#pragma once    
#include"../JObject.h"
#include"../Component/JComponentType.h"
#include"../../Utility/JTypeUtility.h"
#include<vector>
#include<fstream>

namespace JinEngine
{
	class JScene;
	class JAnimator;
	class JTransform;
	class JRenderItem;
	class JComponent;

	class JGameObjectPrivate;
	class JGameObject final : public JObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JGameObject)
	public: 
		class InitData : public JObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		private:
			friend class JGameObject;
		public:
			JGameObject* parent = nullptr;
			JScene* ownerScene = nullptr;
		private:
			bool makeTransform = true;
		public: 
			InitData(JGameObject* parent);
			InitData(JScene* ownerScene);
			InitData(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* parent, JScene* ownerScene = nullptr);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadData final : public Core::JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadData)
		public:
			std::wifstream& stream;
			JGameObject* parent = nullptr;
			JScene* ownerScene = nullptr;
		public:
			LoadData(JScene* owenerScene, std::wifstream& stream);
			~LoadData();
		public:
			bool IsValidData()const noexcept final;
		};
		 
		class StoreData final : public JObject::StoreData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)
		public:
			std::wofstream& stream;
		public:
			StoreData(JGameObject* obj, std::wofstream& stream);
		public:
			bool IsValidData()const noexcept final;
		};
	private:
		friend class JGameObjectPrivate;
		class JGameObjectImpl;
	private:
		std::unique_ptr<JGameObjectImpl> impl;
	public:
		JScene* GetOwnerScene()const noexcept;
		JAnimator* GetAnimator()const noexcept;
		JTransform* GetTransform() const noexcept;
		JRenderItem* GetRenderItem()const noexcept;
		JComponent* GetComponent(const J_COMPONENT_TYPE compType)const noexcept; 
		JComponent* GetComponentWithParent(const J_COMPONENT_TYPE compType)const noexcept;  
		std::vector<JComponent*> GetComponents(const J_COMPONENT_TYPE compType)const noexcept; 
		//std::vector<JComponent*> GetExceptedComponents(const J_COMPONENT_TYPE compType)const noexcept; 
		std::vector<JComponent*> GetComponentsWithParent(const J_COMPONENT_TYPE compType)const noexcept; 
		std::vector<JComponent*> GetAllComponent()const noexcept;
		uint GetAllComponentCount()const noexcept;
		uint GetComponentCount(const J_COMPONENT_TYPE type)const noexcept;
	public:
		uint GetChildrenCount()const noexcept;
		JGameObject* GetParent()const noexcept;
		JGameObject* GetChild(const uint index)const noexcept;
		std::vector<JGameObject*> GetChildren()const noexcept;
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
	public:
		void SetName(const std::wstring& newName)noexcept final;
	public:
		bool IsRoot()const noexcept;
		bool IsParentLine(JGameObject* child)const noexcept;
		bool IsSelected()const noexcept;
		bool HasComponent(const J_COMPONENT_TYPE type)const noexcept;
		bool HasRenderItem()const noexcept;
		bool HasAnimator()const noexcept;
		bool CanAddComponent(const J_COMPONENT_TYPE type)const noexcept;
	public:
		void ChangeParent(JGameObject* newParent)noexcept;
		JComponent* FindComponent(const size_t guid)const noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	public:
		template<typename T>
		auto GetComponent()const noexcept -> typename Core::TypeCondition<T*, std::is_base_of_v<JComponent, T>>::Type
		{ 
			return static_cast<T*>(GetComponent(T::GetStaticComponentType()));
		}
		template<typename T>
		auto GetComponentWithParent()const noexcept -> typename Core::TypeCondition<T*, std::is_base_of_v<JComponent, T>>::Type
		{
			return static_cast<T*>(GetComponentWithParent(T::GetStaticComponentType()));
		}
	private:
		JGameObject(const InitData& initData);
		~JGameObject();
	};
}
