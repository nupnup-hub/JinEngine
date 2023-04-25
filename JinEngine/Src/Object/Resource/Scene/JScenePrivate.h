#pragma once
#include"../JResourceObjectPrivate.h"
#include"../Mesh/JMeshType.h"
#include"../../Component/JComponentType.h"
#include"../../Component/RenderItem/JRenderLayer.h"
#include"../../../Core/SpaceSpatial/JSpaceSpatialType.h"

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
	}
	class JGameObject;
	class JScene;
	class JComponent; 
	class JCamera;
	class JLight;
	class JRenderItem;

	class JScenePrivate : public JResourceObjectPrivate
	{
	public:
		class AssetDataIOInterface final : public JResourceObjectPrivate::AssetDataIOInterface
		{
		private:
			Core::JIdentifier* LoadAssetData(Core::JDITypeDataBase* data) final;
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
			Core::JOwnerPtr<Core::JIdentifier> Create(std::unique_ptr<Core::JDITypeDataBase>&& initData) final;
			bool CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept final;
		}; 
		class CashInterface
		{
		private:
			friend class Graphic::JGraphic;
			friend class Editor::JSceneObserver; //Debug
		private:
			static const std::vector<JGameObject*>& GetGameObjectCashVec(JScene* scene, const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)noexcept;
			static const std::vector<JComponent*>& GetComponentCashVec(JScene* scene, const J_COMPONENT_TYPE cType)noexcept;
		};
		class TimeInterface
		{
		private:
			friend class Editor::JSceneObserver; //Debug
		private:
			static void ActivateSceneTime(JScene* scene)noexcept;
			static void PlaySceneTimer(JScene* scene, const bool value)noexcept;
			static void DeActivateSceneTime(JScene* scene)noexcept;
		};
		class OwnTypeInterface
		{
		private:
			friend class JGameObject;
		private:
			static bool AddGameObject(JGameObject* gObject)noexcept;
			static bool RemoveGameObject(JGameObject* gObject)noexcept;
		};
		class CompSettingInterface
		{
		private:
			friend class JCamera;
			friend class JTransform;
		private:
			static void SetMainCamera(JScene* scene, JCamera* camera)noexcept;
			static void UpdateTransform(JGameObject* gObject)noexcept;
		};
		class CompRegisterInterface
		{
		private:
			friend class JComponent; 
		private:
			static bool RegisterComponent(JComponent* comp)noexcept;
			static bool DeRegisterComponent(JComponent* comp)noexcept;
			static bool ReRegisterComponent(JComponent* comp)noexcept;
		};
		class CompFrameInterface
		{
		private:
			friend class JLight;
			friend class Graphic::JGraphicDrawList; 
		public:
			using SetCompCondition = bool(*)(JComponent&);
		private:
			static void SetAllComponentFrameDirty(JScene* scene)noexcept;
			static void SetComponentFrameDirty(JScene* scene, const J_COMPONENT_TYPE cType, JComponent * stComp = nullptr, SetCompCondition condiiton = nullptr)noexcept;
		};
		class CullingInterface
		{
		private:
			friend class Graphic::JGraphic;
		private:
			static void ViewCulling(JScene* scene)noexcept;
		};
		class DebugInterface
		{
		private:
			friend class Editor::JSceneObserver;
		private:
			static void BuildDebugTree(JScene* scene, Core::J_SPACE_SPATIAL_TYPE type, const Core::J_SPACE_SPATIAL_LAYER layer, _Out_ Editor::JEditorBinaryTreeView& tree)noexcept;
		};
	public:
		Core::JIdentifierPrivate::CreateInstanceInterface& GetCreateInstanceInterface()const noexcept final;
		JResourceObjectPrivate::AssetDataIOInterface& GetAssetDataIOInterface()const noexcept final;
	};
}