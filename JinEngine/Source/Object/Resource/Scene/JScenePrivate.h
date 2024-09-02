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
#include"../JResourceObjectPrivate.h"
#include"Accelerator/JAcceleratorType.h"
#include"../../Component/JComponentType.h"
#include"../../Component/RenderItem/JRenderLayer.h"
#include"../../../Core/Geometry/Mesh/JMeshType.h"

namespace JinEngine
{
	namespace Editor
	{
		class JSceneObserver; 
	} 
	class JGameObject;
	class JScene;
	class JComponent; 
	class JCamera;
	class JLight;
	class JRenderItem;
	class JMeshGeometry;
	class JAcceleratorVisualizeInterface;
	struct JAcceleratorCullingInfo;

	class JScenePrivate : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface final : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		private:
			Core::J_FILE_IO_RESULT LoadMetadata(const std::wstring& path, Core::JDITypeDataBase* data)final;	//use clipMetadata
			Core::J_FILE_IO_RESULT StoreMetadata(Core::JDITypeDataBase* data)final;	//use storeData	 
		};
		class CreateInstanceInterface final : public JResourceObjectPrivate::CreateInstanceInterface
		{
		private:
			friend class AssetDataIOInterface;
		private:
			JOwnerPtr<Core::JIdentifier> Create(Core::JDITypeDataBase* initData) final;
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		}; 
		class TimeInterface
		{
		private:
			friend class Editor::JSceneObserver; //Debug
		private:
			static void ActivateSceneTime(const JUserPtr<JScene>& scene)noexcept;
			static void PlaySceneTimer(const JUserPtr<JScene>& scene, const bool value)noexcept;
			static void DeActivateSceneTime(const JUserPtr<JScene>& scene)noexcept;
		};
		class OwnTypeInterface
		{
		private:
			friend class JGameObject;
		private:
			static bool AddGameObject(const JUserPtr<JGameObject>& gObject)noexcept;
			static bool RemoveGameObject(const JUserPtr<JGameObject>& gObject)noexcept;
		};
		class CompSettingInterface
		{
		private: 
			friend class JTransform;
			friend class JLight;
		private: 
			static void UpdateTransform(const JUserPtr<JComponent>& comp)noexcept;
		};
		class CompRegisterInterface
		{ 
		public:
			//Ascending order 
			//return true is insert this point
			using CompSortPtr = bool(*)(const JUserPtr<JComponent>&, const JUserPtr<JComponent>&);
		private:
			friend class JComponent; 
		private:
			static bool RegisterComponent(const JUserPtr<JComponent>& comp, CompSortPtr comparePtr = nullptr)noexcept;
			static bool DeRegisterComponent(const JUserPtr<JComponent>& comp)noexcept;
			static bool ReRegisterComponent(const JUserPtr<JComponent>& comp, CompSortPtr comparePtr = nullptr)noexcept;
		}; 
		class DebugInterface
		{
		private:
			friend class Editor::JSceneObserver; 
		private:
			static void BuildDebugTree(const JUserPtr<JScene>& scene, J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer, JAcceleratorVisualizeInterface* tree)noexcept;
		}; 
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}