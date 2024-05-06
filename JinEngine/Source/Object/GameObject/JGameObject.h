/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once    
#include"../JObject.h"
#include"../Component/JComponentType.h"
#include"../../Core/Utility/JTypeTraitUtility.h"  
#include<fstream>

namespace JinEngine
{
	class JScene;
	class JAnimator;
	class JTransform;
	class JRenderItem;
	class JComponent;

	class JGameObjectPrivate;
	class JFileIOTool;
	class JGameObject final : public JObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JGameObject)
	public: 
		class InitData : public JObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		private:
			friend class JGameObject;
			friend class JGameObjectPrivate;
		public:
			JUserPtr<JGameObject> parent = nullptr;
			JUserPtr<JScene> ownerScene = nullptr;
		private:
			bool makeTransform = true;
		public: 
			InitData(const JUserPtr<JGameObject>& parent);
			InitData(const JUserPtr<JScene>& ownerScene);
			InitData(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JUserPtr<JGameObject> parent, JUserPtr<JScene> ownerScene = nullptr);
		public:
			bool IsValidData()const noexcept final;
		};
	protected: 
		class LoadData final : public Core::JDITypeDataBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(LoadData)
		public:
			JFileIOTool& tool;
			JUserPtr<JGameObject> parent = nullptr;
			JUserPtr<JScene> ownerScene = nullptr;
		public:
			LoadData(JUserPtr<JScene> owenerScene, JFileIOTool& tool);
			~LoadData();
		public:
			bool IsValidData()const noexcept final;
		};
		 
		class StoreData final : public JObject::StoreData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(StoreData)
		public:
			JFileIOTool& tool;
		public:
			StoreData(JUserPtr<JGameObject> obj, JFileIOTool& tool);
		public:
			bool IsValidData()const noexcept final;
		};
	private:
		friend class JGameObjectPrivate;
		class JGameObjectImpl;
	private:
		std::unique_ptr<JGameObjectImpl> impl;
	public:
		size_t GetOwnerGuid()const noexcept;
		JUserPtr<JScene> GetOwnerScene()const noexcept;
		JUserPtr<JAnimator> GetAnimator()const noexcept;
		JUserPtr<JTransform> GetTransform() const noexcept;
		JUserPtr<JRenderItem> GetRenderItem()const noexcept;
		JUserPtr<JComponent> GetComponent(const J_COMPONENT_TYPE compType)const noexcept;
		JUserPtr<JComponent> GetComponentWithParent(const J_COMPONENT_TYPE compType)const noexcept;
		std::vector<JUserPtr<JComponent>> GetComponents(const J_COMPONENT_TYPE compType)const noexcept;
		//std::vector<JComponent*> GetExceptedComponents(const J_COMPONENT_TYPE compType)const noexcept; 
		std::vector<JUserPtr<JComponent>> GetComponentsWithParent(const J_COMPONENT_TYPE compType)const noexcept;
		std::vector<JUserPtr<JComponent>> GetAllComponent()const noexcept;
		uint GetAllComponentCount()const noexcept;
		uint GetComponentCount(const J_COMPONENT_TYPE type)const noexcept;
	public:
		uint GetChildrenCount()const noexcept;
		JUserPtr<JGameObject> GetParent()const noexcept;
		JUserPtr<JGameObject> GetChild(const uint index)const noexcept;
		std::vector<JUserPtr<JGameObject>> GetChildren()const noexcept;
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		J_OBJECT_TYPE GetObjectType()const noexcept final;
	public:
		void SetName(const std::wstring& newName)noexcept final;
	public:
		bool IsRoot()const noexcept;
		bool IsParentLine(JUserPtr<JGameObject> child)const noexcept;
		bool IsSelected()const noexcept;
		bool IsEditorObject()const noexcept;
		bool HasComponent(const J_COMPONENT_TYPE type)const noexcept;
		bool HasRenderItem()const noexcept;
		bool HasAnimator()const noexcept;
		bool CanAddComponent(const J_COMPONENT_TYPE type)const noexcept;
	private:
		bool CanActivate() const noexcept final;
		bool CanDeActivate() const noexcept final;
	public:
		void ChangeParent(JUserPtr<JGameObject> newParent)noexcept;
		JUserPtr<JComponent> FindComponent(const size_t guid)const noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	public:
		template<typename T>
		auto GetComponent()const noexcept -> typename Core::TypeCondition<JUserPtr<T>, std::is_base_of_v<JComponent, T>>::Type
		{ 
			return JUserPtr<T>::ConvertChild(GetComponent(T::GetStaticComponentType()));
		}
		template<typename T>
		auto GetComponentWithParent()const noexcept -> typename Core::TypeCondition<JUserPtr<T>, std::is_base_of_v<JComponent, T>>::Type
		{
			return JUserPtr<T>::ConvertChild(GetComponentWithParent(T::GetStaticComponentType()));
		}
	private:
		JGameObject(const InitData& initData);
		~JGameObject();
	};
}
