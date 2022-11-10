#pragma once
#include"../JResourceObject.h" 
#include"../JClearableInterface.h"
#include"../Mesh/JMeshType.h"
#include"../../Component/JComponentType.h"
#include"../../Component/RenderItem/JRenderLayer.h"  
#include"../../JFrameUpdate.h"
#include"../../../Core/SpaceSpatial/Bvh/JBvhOption.h" 
#include"../../../Core/SpaceSpatial/Octree/JOctreeOption.h" 
#include"../../../Core/SpaceSpatial/Kd-tree/JKdTreeOption.h" 
#include<DirectXCollision.h> 

namespace JinEngine
{
	class JPreviewResourceScene;
	class JComponent;
	class JCamera;
	class JTransform;
	class JGameObject;
	class JSceneManagerImpl;
	class JResourceManagerImpl;

	namespace Graphic
	{
		class JGraphicDrawList;
		class JGraphicImpl;
	}
 
	namespace Editor
	{
		class JSceneObserver;
		class JEditorBinaryTreeView;
	}
	class JSceneCashInterface
	{
	private:
		friend class Graphic::JGraphicImpl; 
		friend class Editor::JSceneObserver; //Debug
	protected:
		virtual ~JSceneCashInterface() = default;
	public:
		virtual JSceneCashInterface* CashInterface() = 0;
	private:
		virtual const std::vector<JGameObject*>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)const noexcept = 0;
		virtual const std::vector<JComponent*>& GetComponentCashVec(const J_COMPONENT_TYPE cType)const noexcept = 0;
	};

	class JSceneGameObjInterface
	{
	private:
		friend class JGameObject;
	protected:
		virtual ~JSceneGameObjInterface() = default;
	public:
		virtual JSceneGameObjInterface* GameObjInterface() = 0;
	private:
		virtual bool AddGameObject(JGameObject& gameObj)noexcept = 0;
		virtual bool RemoveGameObject(JGameObject& gameObj)noexcept = 0;
	};

	class JSceneCompInterface
	{
	private:
		friend class JCamera;
		friend class JTransform;
	protected:
		virtual ~JSceneCompInterface() = default;
	public:
		virtual JSceneCompInterface* CompInterface() = 0;
	private:
		virtual void SetMainCamera(JCamera* camera)noexcept = 0;
		virtual void SetAnimation()noexcept = 0;
		virtual void UpdateTransform(JGameObject* owner)noexcept = 0;
	};

	class JSceneRegisterInterface 
	{
	private:
		friend class JComponent; 
	protected:
		virtual ~JSceneRegisterInterface() = default;
	public:
		virtual JSceneRegisterInterface* RegisterInterface() = 0;
	private: 
		virtual bool RegisterComponent(JComponent& component)noexcept = 0;
		virtual bool DeRegisterComponent(JComponent& component)noexcept = 0;
	};

	class JSceneFrameInterface
	{
	private:
		friend class JLight;
		friend class Graphic::JGraphicDrawList;
	protected:
		virtual ~JSceneFrameInterface() = default;
	public:
		virtual JSceneFrameInterface* AppInterface() = 0;
	private:
		virtual void SetAllComponentDirty()noexcept = 0;
		virtual void SetComponentDirty(const J_COMPONENT_TYPE cType)noexcept = 0;
		virtual void SetBackSideComponentDirty(JComponent& jComp)noexcept = 0;
		virtual void SetBackSideComponentDirty(JComponent& jComp, bool(*condition)(JComponent&))noexcept = 0;
	};

	class JSceneSpaceSpatialInterface
	{
	private:
		friend class JResourceManagerImpl; 
		friend class Graphic::JGraphicImpl;
		friend class Editor::JSceneObserver;
	protected:
		virtual ~JSceneSpaceSpatialInterface() = default;
	public:
		virtual JSceneSpaceSpatialInterface* SpaceSpatialInterface() = 0;
	private:
		virtual void ViewCulling()noexcept = 0;  
		virtual void ActivateSpaceSpatial(bool setInitValue = false)noexcept = 0;
		virtual void DeActivateSpaceSpatial()noexcept = 0;	 
		virtual std::vector<JGameObject*> GetAlignedObject(const DirectX::BoundingFrustum& frustum)const noexcept = 0;
		virtual Core::JOctreeOption GetOctreeOption()const noexcept = 0;
		virtual Core::JBvhOption GetBvhOption()const noexcept = 0;
		virtual Core::JKdTreeOption GetKdTreeOption()const noexcept = 0;
		virtual void SetOctreeOption(const Core::JOctreeOption& newOption)noexcept = 0;
		virtual void SetBvhOption(const Core::JBvhOption& newOption)noexcept = 0;
		virtual void SetKdTreeOption(const Core::JKdTreeOption& newOption)noexcept = 0;
		//Test
		virtual void BuildDebugTree(Core::J_SPACE_SPATIAL_TYPE type, Editor::JEditorBinaryTreeView& tree)noexcept = 0; 
	};

	class JSceneInterface :public JResourceObject,
		public JSceneSpaceSpatialInterface, 
		public JSceneFrameInterface,
		public JSceneRegisterInterface,
		public JSceneCompInterface,
		public JSceneGameObjInterface,
		public JSceneCashInterface,
		public JFrameBuffManagerInterface,
		public JClearableInterface
	{
	protected:
		JSceneInterface(const JResourceObject::JResourceInitData& initdata);
	};
}