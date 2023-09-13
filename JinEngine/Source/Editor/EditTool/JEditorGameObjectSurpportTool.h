#pragma once
#include"JEditorGameObjectSurpportToolType.h"
#include"../Interface/JEditorObjectHandleInterface.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Callable/JCallable.h" 
#include"../../Core/Interface/JActivatedInterface.h"
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/Math/JVector.h"
#include"../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../Object/Resource/Scene/Accelerator/JAcceleratorType.h"

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
		class JEditorGameObjectSurpportTool : public Core::JActivatedInterface,  public JEditorObjectHandlerInterface
		{  
		public:
			virtual ~JEditorGameObjectSurpportTool() = default;
		public:
			virtual	J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE GetToolType()const noexcept = 0; 
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
				void SetSelectedColor()noexcept;
				void OffSelectedColor()noexcept;
			};
			struct UpdateData
			{ 
			public:
				JVector2<float> preWorldMousePos;
				JVector2<float> preLocalMouseMidGap;		//Mid to nowLocalPos  
				int hoveringIndex = invalidIndex;
			public:
				bool isDragging = false;
				bool isLastUpdateSelected= false;
			};
		private:
			using UpdateTransformT = Core::JStaticCallableType<void, JEditorTransformTool*, const std::vector<JUserPtr<JGameObject>>&, const JUserPtr<JCamera>&>;
		private:
			static constexpr uint arrowCount = 3;
		private:
			const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType;
			const J_DEFAULT_SHAPE shape;
			const float sizeRate = 0;
			const bool hasCenter;
		private:
			UpdateTransformT::Ptr transformUpdatePtr;
		private:
			//shape
			JUserPtr<JGameObject> debugRoot;
			JUserPtr<JGameObject> transformArrowRoot;
			JUserPtr<JGameObject> arrowCenter;
			JUserPtr<JMaterial> arrowCenterMaterial;
			JUserPtr<JMeshGeometry> mesh;
			Arrow arrow[arrowCount];
		private:
			//update data
			UpdateData uData;
		public:
			JEditorTransformTool(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType,
				const J_DEFAULT_SHAPE shape, 
				const float sizeRate);
			~JEditorTransformTool();
		public:
			void Update(const JUserPtr<JGameObject>& selected, 
				const JUserPtr<JCamera>& cam,
				const JVector2<float>& sceneImageMinPoint,
				const bool canSelectToolObject);
			/**
			* @param selected owner scene is same
			* @param cam is selected object owner scene cam
			*/
			void Update(const std::vector<JUserPtr<JGameObject>>& selected,
				const JUserPtr<JCamera>& cam,
				const JVector2<float>& sceneImageMinPoint,
				const bool canSelectToolObject);
		private:
			void UpdateStart();
			void UpdateToolObject(const bool isValidSelected);
		private: 
			void UpdateArrowPosition(const JVector3<float>& posW, const JUserPtr<JCamera>& cam);
			void UpdateArrowDragging(const std::vector<JUserPtr<JGameObject>>& selected, 
				const JUserPtr<JCamera>& cam, 
				const JVector2<float>& sceneImageMinPoint,
				const bool canSelectToolObject);
		private:
			static UpdateTransformT::Ptr GetUpdateTransformPtr(const J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE toolType)noexcept;
			static void UpdateSelectedPosition(JEditorTransformTool* tool, const std::vector<JUserPtr<JGameObject>>& selected, const JUserPtr<JCamera>& cam)noexcept;
			static void UpdateSelectedRotation(JEditorTransformTool* tool, const std::vector<JUserPtr<JGameObject>>& selected, const JUserPtr<JCamera>& cam)noexcept;
			static void UpdateSelectedScale(JEditorTransformTool* tool, const std::vector<JUserPtr<JGameObject>>& selected, const JUserPtr<JCamera>& cam)noexcept;
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
		public: 
			bool IsLastUpdateSelectedObject()const noexcept; 
			bool IsHoveringObject()const noexcept;
			bool IsDraggingObject()const noexcept;
		private: 
			void OnDragging()noexcept;
			void OffDragging()noexcept;
			void OnHovering(const int newArrowIndex)noexcept;
			void OffHovering()noexcept;
			//static void SetTransformPosition()
		};
	}
}