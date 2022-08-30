#pragma once
#include"../JResourceObject.h" 
#include"../JClearableInterface.h"
#include"../Mesh/JMeshType.h"
#include"../../Component/JComponentType.h"
#include"../../Component/RenderItem/JRenderLayer.h"  

namespace JinEngine
{
	class JPreviewResourceScene;
	class JComponent;
	class JCamera;
	class JGameObject;
	class JSceneManagerImpl;
	class JResourceManagerImpl;

	namespace Graphic
	{
		class JGraphicDrawList;
		class JGraphicImpl;
	}
 
	class JSceneCashInterface
	{
	private:
		friend class Graphic::JGraphicImpl;
	protected:
		virtual ~JSceneCashInterface() = default;
	public:
		virtual JSceneCashInterface* CashInterface() = 0;
	private:
		virtual std::vector<JGameObject*>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)noexcept = 0;
		virtual std::vector<JComponent*>& GetComponentCashVec(const J_COMPONENT_TYPE cType)noexcept = 0;
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
		virtual bool RemoveGameObject(JGameObject& gameObj)noexcept = 0;
	};

	class JSceneCompInterface
	{
	private:
		friend class JCamera;
	protected:
		virtual ~JSceneCompInterface() = default;
	public:
		virtual JSceneCompInterface* CompInterface() = 0;
	private:
		virtual JCamera* SetMainCamera(JCamera* camera)noexcept = 0;
		virtual void SetAnimation()noexcept = 0;
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
		virtual JSceneFrameInterface* FrameInterface() = 0;
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
	protected:
		virtual ~JSceneSpaceSpatialInterface() = default;
	public:
		virtual JSceneSpaceSpatialInterface* SpaceSpatialInterface() = 0;
	private:
		virtual void ViewCulling()noexcept = 0;   
		virtual void OnSceneSpatialStructure() noexcept = 0;
		virtual void OffSceneSpatialStructure() noexcept = 0;
		virtual void OnDebugBoundingBox(bool onlyLeafNode)noexcept = 0;
		virtual void OffDebugBoundingBox()noexcept = 0;
	};

	class JSceneInterface :public JResourceObject,
		public JSceneSpaceSpatialInterface, 
		public JSceneFrameInterface,
		public JSceneRegisterInterface,
		public JSceneCompInterface,
		public JSceneGameObjInterface,
		public JSceneCashInterface,
		public JClearableInterface
	{
	protected:
		JSceneInterface(const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}