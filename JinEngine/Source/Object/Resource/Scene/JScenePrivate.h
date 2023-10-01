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
		class JEditorBinaryTreeView;
	}
	namespace Graphic
	{ 
		class JGraphic;
		class JGraphicDrawList; 
		class JFrustumCulling;
		struct JDrawHelper;
	}
	class JGameObject;
	class JScene;
	class JComponent; 
	class JCamera;
	class JLight;
	class JRenderItem;
	class JMeshGeometry;

	class JScenePrivate : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface final : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			JUserPtr<Core::JIdentifier> LoadAssetData(Core::JDITypeDataBase* data) final;
			Core::J_FILE_IO_RESULT StoreAssetData(Core::JDITypeDataBase* data) final;
		private:
			Core::J_FILE_IO_RESULT LoadMetaData(const std::wstring& path, Core::JDITypeDataBase* data)final;	//use clipMetaData
			Core::J_FILE_IO_RESULT StoreMetaData(Core::JDITypeDataBase* data)final;	//use storeData	 
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
		class CashInterface
		{
		private:
			friend struct Graphic::JDrawHelper;
			friend class Graphic::JGraphic;
			friend class Editor::JSceneObserver; //Debug
		private:
			static const std::vector<JUserPtr<JGameObject>>& GetGameObjectCashVec(JScene* scene, const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)noexcept;
			static const std::vector<JUserPtr<JGameObject>>& GetGameObjectCashVec(const JUserPtr<JScene>& scene, const J_RENDER_LAYER rLayer, const Core::J_MESHGEOMETRY_TYPE meshType)noexcept;
			static const std::vector<JUserPtr<JComponent>>& GetComponentCashVec(JScene* scene, const J_COMPONENT_TYPE cType)noexcept;
			static const std::vector<JUserPtr<JComponent>>& GetComponentCashVec(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType)noexcept;
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
		private: 
			static void UpdateTransform(const JUserPtr<JGameObject>& gObject)noexcept;
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
		class CompFrameInterface
		{
		private: 
			friend class Graphic::JGraphicDrawList; 
		public:
			using SetCompCondition = bool(*)(const JUserPtr<JComponent>&);
		private:
			//range: all comp vector
			static void SetAllComponentFrameDirty(const JUserPtr<JScene>& scene)noexcept;
			//range: comp vector
			static void SetComponentFrameDirty(const JUserPtr<JScene>& scene, const J_COMPONENT_TYPE cType, JUserPtr<JComponent> stComp = nullptr, SetCompCondition condiiton = nullptr)noexcept;
		};
		class CullingInterface
		{
		private:
			friend class Graphic::JFrustumCulling;
		private:
			static void ViewCulling(const JUserPtr<JScene>& scene, const JUserPtr<JComponent>& comp)noexcept; 
		};
		class DebugInterface
		{
		private:
			friend class Editor::JSceneObserver;
		private:
			static void BuildDebugTree(const JUserPtr<JScene>& scene, J_ACCELERATOR_TYPE type, const J_ACCELERATOR_LAYER layer, _Out_ Editor::JEditorBinaryTreeView& tree)noexcept;
		};
		class FrameIndexInterface
		{
		private:
			friend class Graphic::JGraphic;
			friend struct Graphic::JDrawHelper;
		private:
			static uint GetPassFrameIndex(JScene* scene);
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}