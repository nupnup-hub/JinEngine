#pragma once
#include"JEditorGameObjectSurpportToolType.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Callable/JCallable.h" 
#include"../../Core/Utility/JActivatedInterface.h"
#include"../../Core/Utility/JValidInterface.h"
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
	namespace Editor
	{ 
		namespace Constants
		{
			static constexpr uint arrowCount = 3;
		} 
	
		class JEditorGameObjectSurpportTool : public Core::JActivatedInterface
		{  
		public:
			virtual ~JEditorGameObjectSurpportTool() = default;
		public:
			virtual void Update(Core::JUserPtr<JObject> selected, Core::JUserPtr<JCamera> cam) = 0;
		public:
			virtual	J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE GetToolType()const noexcept = 0; 
			static JGameObject* SceneIntersect(Core::JUserPtr<JScene> scene,
				Core::JUserPtr<JCamera> cam,
				Core::J_SPACE_SPATIAL_LAYER layer) noexcept;
		public:
			bool IsEditable(JGameObject* obj)const noexcept;
		};

		class JEditorTransformTool : public JEditorGameObjectSurpportTool, public Core::JValidInterface
		{  
		private:
			struct Arrow
			{
			public:
				Core::JUserPtr<JGameObject> arrow;
				Core::JUserPtr<JMaterial> material; 
				JVector4<float> matColor;
			public:
				void CreateMaterial(const JVector4<float> matColor);
			public:
				void Initialze(JGameObject* debugRoot,
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
			using UpdateTransformT = Core::JStaticCallableType<void, JEditorTransformTool*, JGameObject*>;
		private:
			const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType;
			const J_DEFAULT_SHAPE shape;
			const float sizeRate = 0;
			float shapeLength = 0;
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
			Core::JUserPtr<JGameObject> debugRoot;
			Core::JUserPtr<JGameObject> transformArrowRoot;
			Core::JUserPtr<JGameObject> arrowCenter;
			Core::JUserPtr<JMaterial> arrowCenterMaterial;
			Arrow arrow[Constants::arrowCount];
		public:
			JEditorTransformTool(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType,
				const J_DEFAULT_SHAPE shape, 
				const float sizeRate);
			~JEditorTransformTool();
		public:
			void Update(Core::JUserPtr<JObject> selected, Core::JUserPtr<JCamera> cam)final;
		private: 
			void UpdateArrowPosition(JGameObject* selected, Core::JUserPtr<JCamera> cam);
			void UpdateArrowDragging(JGameObject* selected, Core::JUserPtr<JCamera> cam);
		private:
			static UpdateTransformT::Ptr GetUpdateTransformPtr(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType)noexcept;
			static void UpdateSelectedPosition(JEditorTransformTool* tool, JGameObject* selected)noexcept;
			static void UpdateSelectedRotation(JEditorTransformTool* tool, JGameObject* selected)noexcept;
			static void UpdateSelectedScale(JEditorTransformTool* tool, JGameObject* selected)noexcept;
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
		public:
			void SetDebugRoot(Core::JUserPtr<JGameObject> debugRoot);
		private:
			void OnHovering(const int newArrowIndex)noexcept;
			void OffHovering()noexcept;
			void OnDragging()noexcept;
			void OffDragging()noexcept;
			//static void SetTransformPosition()
		};
	}
}