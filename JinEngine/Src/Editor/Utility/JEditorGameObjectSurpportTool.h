#pragma once
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Utility/JValidInterface.h"
#include"../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../Utility/JVector.h"

namespace JinEngine
{
	class JGameObject;
	class JCamera;
	class JObject;
	class JMaterial;
	namespace Editor
	{ 
		namespace Constants
		{
			static constexpr uint arrowCount = 3;
		}
		enum class J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE
		{
			POSITION_ARROW,
			ROTATION_ARROW,
		};

		class JEditorGameObjectSurpportTool
		{ 
		public:
			virtual ~JEditorGameObjectSurpportTool() = default;
		public:
			virtual void Activate() = 0;
			virtual void DeActivate() = 0;
		public:
			virtual void Update(Core::JUserPtr<JObject> selected, Core::JUserPtr<JCamera> cam) = 0;
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
					const JVector3<float> initScale,
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
			const J_DEFAULT_SHAPE shape;
			const float sizeRate = 0;
			float shapeLength = 0;
		private:
			bool isActivated = false;
			bool isDraggingObject = false;
		private:
			int draggingIndex = -1;
		private:
			Core::JUserPtr<JGameObject> debugRoot;
			Core::JUserPtr<JGameObject> transformArrowRoot;
			Core::JUserPtr<JGameObject> arrowCenter;
			Core::JUserPtr<JMaterial> arrowCenterMaterial;
			Arrow arrow[Constants::arrowCount];
		public:
			JEditorTransformTool(const J_DEFAULT_SHAPE shape, const float sizeRate); 
			~JEditorTransformTool();
		public:
			void Activate() final;
			void DeActivate()final;
		public:
			void Update(Core::JUserPtr<JObject> selected, Core::JUserPtr<JCamera> cam)final;
		private:
			void UpdateSelectedTransform(JGameObject* selected); 
			void UpdateArrowPosition(JGameObject* selected, Core::JUserPtr<JCamera> cam);
			void UpdateArrowDragging(JGameObject* selected, Core::JUserPtr<JCamera> cam);
		private:
			static void UpdateSelectedPosition(JEditorTransformTool* tool, const JVector2<float> mouseDelta);
			static void UpdateSelectedRotation(JEditorTransformTool* tool, const JVector2<float> mouseDelta);
		public:
			bool IsValid()const noexcept final; 
		public:
			void SetDebugRoot(Core::JUserPtr<JGameObject> debugRoot);
			//static void SetTransformPosition()
		};
	}
}