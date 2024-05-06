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
#include"../JObjectPrivate.h" 
 
namespace JinEngine
{
	class JComponent;
	class JScene;
	class JGameObject;
	class JFileIOTool;

	namespace Editor
	{
		class JEditorWindow;
		class JObjectExplorer;
	}
	class JGameObjectPrivate final : public JObjectPrivate
	{
	public:
		class AssetDataIOInterface final
		{
		private:
			friend class JScene;
		private:
			static std::unique_ptr<Core::JDITypeDataBase> CreateLoadAssetDIData(JUserPtr<JScene> invoker, JFileIOTool& tool);
			static std::unique_ptr<Core::JDITypeDataBase> CreateStoreAssetDIData(JUserPtr<JGameObject> root, JFileIOTool& tool);
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
			friend class Editor::JObjectExplorer;
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