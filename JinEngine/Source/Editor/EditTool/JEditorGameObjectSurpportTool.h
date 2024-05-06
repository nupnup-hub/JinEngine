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
#include"JEditorGameObjectSurpportToolType.h" 
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Func/Callable/JCallable.h" 
#include"../../Core/Interface/JActivatedInterface.h"
#include"../../Core/Interface/JValidInterface.h"
#include"../../Core/Math/JVector.h"
#include"../../Object/JObjectModifyInterface.h"
#include"../../Object/Resource/Mesh/JDefaultShapeType.h"
#include"../../Object/Resource/Material/JDefaultMaterialType.h"
#include"../../Object/Resource/Scene/Accelerator/JAcceleratorType.h"
#include<set>
namespace JinEngine
{
	class JGameObject;
	class JCamera;
	class JRectLight;
	class JPointLight;
	class JSpotLight;
	class JObject;
	class JScene;
	class JMaterial;
	class JMeshGeometry;
	namespace Editor
	{ 
		class JEditorGameObjectSurpportTool : public Core::JActivatedInterface,  public JObjectModifyInterface
		{  
		public:
			virtual ~JEditorGameObjectSurpportTool() = default;
		public:
			virtual	J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE GetToolType()const noexcept = 0; 
		public:
			bool IsEditable(JGameObject* obj)const noexcept;
		public:
			//calculate ui object scale in the scene
			//카메라에서 멀어질수록 높은 scale값을 return
			static JVector2F CalUiScale(const JVector3F& posW, const JVector2F fixedScale, const JUserPtr<JCamera>& cam);
		};

		class JEditorTransformTool : public JEditorGameObjectSurpportTool, public Core::JValidInterface
		{    
		private:
			struct Arrow
			{
			public:
				JUserPtr<JGameObject> arrow; 
				JUserPtr<JMaterial> material; 
				JVector4F matColor;
			public:
				void CreateMaterial(const JVector4F matColor);
			public:
				void Initialze(const JUserPtr<JGameObject>& debugRoot,
					const J_DEFAULT_SHAPE shape,  
					const JVector3F initRotation, 
					const JVector3F initMovePos);
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
				JVector3F lastPos = InvalidPos();
				JUserPtr<JGameObject> lastSelected;		//single click만 유효
			public:
				JVector2F preWorldMousePos;
				JVector2F preLocalMouseMidGap;		//Mid to nowLocalPos  
				int hoveringIndex = invalidIndex;
			public:
				bool isDragging = false;
				bool isLastUpdateSelected= false;
			public:
				JVector3F InvalidPos()const noexcept;
			public:
				void Clear();
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
			//추후 추가사항(낮은 우선순위)
			//update 이전 transform, 이후 tranform값을 mouse dragging off시 addBind해서 
			//do, undo가 가능하게 한다.
			void Update(const JUserPtr<JGameObject>& selected,
				const JUserPtr<JCamera>& cam,
				const JVector2F& sceneImageScreenMinPoint,
				const bool canSelectToolObject);
			/**
			* @param selected owner scene is same
			* @param cam is selected object owner scene cam
			*/
			void Update(const std::vector<JUserPtr<JGameObject>>& selected,
				const JUserPtr<JCamera>& cam,
				const JVector2F& sceneImageScreenMinPoint,
				const bool canSelectToolObject);
		private:
			void UpdateStart();
			void UpdateToolObject(const bool isValidSelected);
		private: 
			void UpdateArrowPosition(const JVector3F& posW, const JUserPtr<JCamera>& cam);
			void UpdateArrowDragging(const JUserPtr<JGameObject>& selected,
				const JUserPtr<JCamera>& cam,
				const JVector2F& sceneImageScreenMinPoint,
				const bool canSelectToolObject);
			void UpdateArrowDragging(const std::vector<JUserPtr<JGameObject>>& selected, 
				const JUserPtr<JCamera>& cam, 
				const JVector2F& sceneImageScreenMinPoint,
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
			JUserPtr<JGameObject> GetLastSelected()const noexcept;
		public:
			void SetDebugRoot(JUserPtr<JGameObject> debugRoot);
		public: 
			bool IsLastUpdateSelected()const noexcept; 
			bool IsHovering()const noexcept;
			bool IsDragging()const noexcept; 
			bool IsHitDebugObject(const JUserPtr<JCamera>& cam, const JVector2F& sceneImageScreenMinPoint)const noexcept;
		private: 
			void OnDragging()noexcept;
			void OffDragging()noexcept;
			void OnHovering(const int newArrowIndex)noexcept;
			void OffHovering()noexcept;
			//static void SetTransformPosition()
		};
		class JEditorGeometryTool : public JEditorGameObjectSurpportTool
		{ 
		public:
			enum class VIEW_TYPE
			{
				FRUSTUM,
				SPHERE,
				SPOT,
				RECT
			};
		private:
			class GeometryView
			{
			public:
				virtual ~GeometryView() = default;
			public:
				virtual void Clear() = 0;
			public:
				virtual void Update() = 0;
			public:
				virtual size_t GetTargetGuid() const noexcept = 0;
				virtual Core::JTypeInfo& GetTargetTypeInfo() const noexcept = 0; 
			public:
				virtual void SetMaterial(const JUserPtr<JMaterial>& mat) = 0;
			public:
				virtual bool IsValid()const noexcept = 0;
			};
			class FrustumView : public GeometryView
			{
			public:
				JUserPtr<JGameObject> root;
				JUserPtr<JGameObject> nearFrustum;
				JUserPtr<JGameObject> farFrustum;
			public:
				JUserPtr<JCamera> targetCam;
			public:
				FrustumView(const JUserPtr<JCamera>& cam, const JUserPtr<JGameObject>& parent);
				~FrustumView();
			public: 
				void Clear()final;
			public:
				void Update()final;
			public:
				size_t GetTargetGuid()const noexcept final;
				Core::JTypeInfo& GetTargetTypeInfo() const noexcept final; 
			public:
				void SetMaterial(const JUserPtr<JMaterial>& mat) final;
			public:
				bool IsValid()const noexcept final;
			};
			class SphereView : public GeometryView
			{
			public:
				JUserPtr<JGameObject> root;
				JUserPtr<JGameObject> xyCircle;
				JUserPtr<JGameObject> xzCircle;
				JUserPtr<JGameObject> yzCircle;
			public:
				JUserPtr<JPointLight> targetPoint;
			public:
				SphereView(const JUserPtr<JPointLight>& lit, const JUserPtr<JGameObject>& parent);
				~SphereView();
			public: 
				void Clear()final;
			public:
				void Update()final;
			public:
				size_t GetTargetGuid()const noexcept final;
				Core::JTypeInfo& GetTargetTypeInfo() const noexcept final; 
			public:
				void SetMaterial(const JUserPtr<JMaterial>& mat) final;
			public:
				bool IsValid()const noexcept;
			};
			class ConeView : public GeometryView
			{
			public:
				JUserPtr<JGameObject> root; 
				JUserPtr<JGameObject> boundingCone;
			public:
				JUserPtr<JSpotLight> targetSpot;
			public:
				ConeView(const JUserPtr<JSpotLight>& lit, const JUserPtr<JGameObject>& parent);
				~ConeView();
			public: 
				void Clear()final;
			public:
				void Update()final;
			public:
				size_t GetTargetGuid()const noexcept final;
				Core::JTypeInfo& GetTargetTypeInfo() const noexcept final; 
			public:
				void SetMaterial(const JUserPtr<JMaterial>& mat) final;
			public:
				bool IsValid()const noexcept;
			};
			class RectView : public GeometryView
			{
			public:
				JUserPtr<JGameObject> root;
				JUserPtr<JGameObject> innerLine[4];
				JUserPtr<JGameObject> edgeLine[4];
				JUserPtr<JGameObject> outerLine[4];
			public:
				JUserPtr<JRectLight> targetRect;
			public:
				RectView(const JUserPtr<JRectLight>& lit, const JUserPtr<JGameObject>& parent);
				~RectView();
			public: 
				void Clear()final;
			public:
				void Update()final;
			public:
				size_t GetTargetGuid()const noexcept final;
				Core::JTypeInfo& GetTargetTypeInfo() const noexcept final; 
			public:
				void SetMaterial(const JUserPtr<JMaterial>& mat) final;
			public:
				bool IsValid()const noexcept;
			}; 
		private: 
			std::vector<std::unique_ptr<GeometryView>> geoView;
			std::set<size_t> geoSet;
		public:
			~JEditorGeometryTool();
		public:
			J_EDITOR_GAMEOBJECT_SUPPORT_TOOL_TYPE GetToolType()const noexcept final;
		public:
			void TryCreateGeoView(const std::vector<JUserPtr<JGameObject>>& idenVec, const JUserPtr<JGameObject>& parent);
		public:
			bool CreateFrustumView(const JUserPtr<JCamera>& cam, const JUserPtr<JGameObject>& parent);
			bool CreateSphereView(const JUserPtr<JPointLight>& lit, const JUserPtr<JGameObject>& parent);
			bool CreateSpotView(const JUserPtr<JSpotLight>& lit, const JUserPtr<JGameObject>& parent);
			bool CreateRectView(const JUserPtr<JRectLight>& lit, const JUserPtr<JGameObject>& parent);
		private:
			bool CreateGeoView(std::unique_ptr<GeometryView>&& view, const size_t guid);
		public:
			void DestroyView(const size_t guid);
		public:
			void Clear();
			void ClearTarget(Core::JTypeInfo& type); 
		public:
			void Update();
		public:
			bool HasGeo(const size_t guid);
		};
	}
}