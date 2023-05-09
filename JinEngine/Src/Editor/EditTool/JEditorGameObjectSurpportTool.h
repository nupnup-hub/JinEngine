#pragma once
#include"JEditorGameObjectSurpportToolType.h"
#include"../Interface/JEditorObjectHandleInterface.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Callable/JCallable.h" 
#include"../../Core/Interface/JActivatedInterface.h"
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/SpaceSpatial/JSpaceSpatialType.h"
#include"../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../Utility/JVector.h"

namespace JinEngine
{
	class JGameObject;
	class JCamera;
	class JObject;
	class JScene;
	class JMaterial;
	class JMeshGeometry;
	namespace Editor
	{ 
		namespace Constants
		{
			static constexpr uint arrowCount = 3;
		} 
	
		class JEditorGameObjectSurpportTool : public Core::JActivatedInterface,  public JEditorObjectHandlerInterface
		{  
		public:
			virtual ~JEditorGameObjectSurpportTool() = default;
		public:
			virtual void Update(const JUserPtr<JGameObject>& selected, const JUserPtr<JCamera>& cam, const JVector2<float>& viewLocalPos) = 0;
		public:
			virtual	J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE GetToolType()const noexcept = 0; 
			static JUserPtr<JGameObject> SceneIntersect(JUserPtr<JScene> scene,
				JUserPtr<JCamera> cam,
				Core::J_SPACE_SPATIAL_LAYER layer, 
				const JVector2<float>& viewLocalPos) noexcept;
		public:
			bool IsEditable(JGameObject* obj)const noexcept;
		};

		class JEditorTransformTool : public JEditorGameObjectSurpportTool, public Core::JValidInterface
		{  
		private:
			struct Arrow
			{
			public:
				JUserPtr<JGameObject> arrow; 
				JUserPtr<JMaterial> material; 
				JVector4<float> matColor;
			public:
				void CreateMaterial(const JVector4<float> matColor);
			public:
				void Initialze(const JUserPtr<JGameObject>& debugRoot,
					const J_DEFAULT_SHAPE shape,  
					const JVector3<float> initRotation, 
					const JVector3<float> initMovePos);
				void Clear();
			public:
				bool IsValid()const noexcept; 
			public:
				void SetHoveredColor()noexcept;
				void OffHoveredColor()noexcept;
			};
		private:
			using UpdateTransformT = Core::JStaticCallableType<void, JEditorTransformTool*, const JUserPtr<JGameObject>&, const JUserPtr<JCamera>&>;
		private:
			const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType;
			const J_DEFAULT_SHAPE shape;
			JUserPtr<JMeshGeometry> mesh;
			const float sizeRate = 0;
			const bool hasCenter;
		private:
			UpdateTransformT::Ptr transformUpdatePtr;
		private:
			bool isDraggingObject = false;
		private:
			int hoveringIndex = -1; 
			int draggingIndex = -1;
		private:
			JVector2<float> preWorldMousePos;
			//Mid to nowLocalPos
			JVector2<float> preLocalMouseMidGap;
		private:
			JUserPtr<JGameObject> debugRoot;
			JUserPtr<JGameObject> transformArrowRoot;
			JUserPtr<JGameObject> arrowCenter;
			JUserPtr<JMaterial> arrowCenterMaterial;
			Arrow arrow[Constants::arrowCount];
		public:
			JEditorTransformTool(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType,
				const J_DEFAULT_SHAPE shape, 
				const float sizeRate);
			~JEditorTransformTool();
		public:
			void Update(const JUserPtr<JGameObject>& selected, const JUserPtr<JCamera>& cam, const JVector2<float>& viewLocalPos)final;
		private: 
			void UpdateArrowPosition(const JUserPtr<JGameObject>& selected, const JUserPtr<JCamera>& cam);
			void UpdateArrowDragging(const JUserPtr<JGameObject>& selected, const JUserPtr<JCamera>& cam, const JVector2<float>& viewLocalPos);
		private:
			static UpdateTransformT::Ptr GetUpdateTransformPtr(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType)noexcept;
			static void UpdateSelectedPosition(JEditorTransformTool* tool, const JUserPtr<JGameObject>& selected, const JUserPtr<JCamera>& cam)noexcept;
			static void UpdateSelectedRotation(JEditorTransformTool* tool, const JUserPtr<JGameObject>& selected, const JUserPtr<JCamera>& cam)noexcept;
			static void UpdateSelectedScale(JEditorTransformTool* tool, const JUserPtr<JGameObject>& selected, const JUserPtr<JCamera>& cam)noexcept;
		public:
			void ActivateTool()noexcept;
			void DeActivateTool()noexcept;
		private:
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		private:
			void CreateToolObject()noexcept;
			void DestroyToolObject()noexcept;
		public:
			J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE GetToolType()const noexcept final;
			uint GetShapeLength()const noexcept;
		public:
			void SetDebugRoot(JUserPtr<JGameObject> debugRoot);
		private:
			void OnHovering(const int newArrowIndex)noexcept;
			void OffHovering()noexcept;
			void OnDragging()noexcept;
			void OffDragging()noexcept;
			//static void SetTransformPosition()
		};
	}
}