#pragma once
#include"../JResourceObject.h" 
#include"../JClearableInterface.h"
#include"../Mesh/JMeshType.h"
#include"../../Component/JComponentType.h"
#include"../../Component/RenderItem/JRenderLayer.h"  

namespace JinEngine
{
	class PreviewResourceScene;
	class JComponent;
	class JCamera;
	class JGameObject;
	class JResourceManager;

	namespace Graphic
	{
		class JGraphicDrawList;
		class JGraphicImpl;
	}
 
	class JSceneCashInterface : public JResourceObject
	{
	private:
		friend class Graphic::JGraphicImpl;
	protected:
		JSceneCashInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	public:
		virtual JSceneCashInterface* CashInterface() = 0;
	private:
		virtual std::vector<JGameObject*>& GetGameObjectCashVec(const J_RENDER_LAYER rLayer, const J_MESHGEOMETRY_TYPE meshType)noexcept = 0;
		virtual std::vector<JComponent*>& GetComponentCashVec(const J_COMPONENT_TYPE cType)noexcept = 0;
	};

	class JSceneGameObjInterface : public JSceneCashInterface
	{
	private:
		friend class JGameObject;
	protected:
		JSceneGameObjInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	public:
		virtual JSceneGameObjInterface* GameObjInterface() = 0;
	private:
		virtual bool EraseGameObject(JGameObject& gameObj)noexcept = 0;
	};

	class JSceneCompInterface : public JSceneGameObjInterface
	{
	private:
		friend class JCamera;
	protected:
		JSceneCompInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	public:
		virtual JSceneCompInterface* CompInterface() = 0;
	private:
		virtual JCamera* SetMainCamera(JCamera* camera)noexcept = 0;
		virtual void SetAnimation()noexcept = 0;
	};

	class JSceneRegisterInterface : public JSceneCompInterface
	{
	private:
		friend class JComponent; 
	protected:
		JSceneRegisterInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	public:
		virtual JSceneRegisterInterface* RegisterInterface() = 0;
	private: 
		virtual bool RegisterComponent(JComponent& component)noexcept = 0;
		virtual bool DeRegisterComponent(JComponent& component)noexcept = 0;
	};

	class JSceneFrameInterface : public JSceneRegisterInterface
	{
	private:
		friend class JLight;
		friend class Graphic::JGraphicDrawList;
	protected:
		JSceneFrameInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	public:
		virtual JSceneFrameInterface* FrameInterface() = 0;
	private:
		virtual void SetAllComponentDirty()noexcept = 0;
		virtual void SetComponentDirty(const J_COMPONENT_TYPE cType)noexcept = 0;
		virtual void SetBackSideComponentDirty(JComponent& jComp)noexcept = 0;
		virtual void SetBackSideComponentDirty(JComponent& jComp, bool(*condition)(JComponent&))noexcept = 0;
	};

	class JSceneSpaceSpatialInterface : public JSceneFrameInterface
	{
	private:
		friend class JResourceManager;
		friend class Graphic::JGraphicImpl;
	protected:
		JSceneSpaceSpatialInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	public:
		virtual JSceneSpaceSpatialInterface* SpaceSpatialInterface() = 0;
	private:
		virtual void ViewCulling()noexcept = 0;   
		virtual void OnSceneSpatialStructure() noexcept = 0;
		virtual void OffSceneSpatialStructure() noexcept = 0;
		virtual void OnDebugBoundingBox(bool onlyLeafNode)noexcept = 0;
		virtual void OffDebugBoundingBox()noexcept = 0;
	};

	class JSceneInterface : public JSceneSpaceSpatialInterface , public JClearableInterface
	{
	protected:
		JSceneInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}