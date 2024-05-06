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
#include"JEditorCoordGrid.h"
#include"../../Core/JCoreEssential.h"  
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Utility/JMacroUtility.h"
#include"../../Core/Math/JVector.h"
#include"../../Object/Resource/Scene/Accelerator/JAcceleratorVisualizeInterface.h" 

namespace JinEngine
{
	namespace Editor
	{
		struct JEditorViewUpdateHelper;

		class JEditorNodeBase
		{
		private:
			std::string name;
			std::string desc;
		private:
			size_t guid;
		private:
			//Center position
			JVector2<float> center;
		private:
			bool isNewNode = false;
			bool isSelected = false;
		public:
			JEditorNodeBase(const std::string& name, const size_t guid, const std::string& desc, const bool isSelected);
			virtual ~JEditorNodeBase() = default;
		public:
			void DrawRect(const JEditorViewUpdateHelper* updateHelper);
			void DrawLine(const JEditorViewUpdateHelper* updateHelper, const JEditorNodeBase* to, const bool isSelecetdEdge, const bool isBilateralEdge);
			void StoreUpdatedNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
			void LoadUpdateNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
		public:
			virtual void SettingDrawLine(const JEditorViewUpdateHelper* updateHelper) = 0;
		protected:
			virtual void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept = 0;
			virtual bool DoInitializeWhenAddedNewNode()const noexcept = 0;
		public:
			/**
			* JEditorNodeBase내부에서 uniqueLabel이 필요한 gui widget 사용시 호출
			* --2023-09-26-- 현재까지는 0개
			*/
			std::string GetUniqueName()const noexcept;
			//Apply mouse offset
			JVector2<float> GetValidCenter(const JEditorViewUpdateHelper* updateHelper)const noexcept;
			std::string GetCompressName(const JEditorViewUpdateHelper* updateHelper)const noexcept;
			/*
			* offset = normal(toCenter - fromCenter) * updateHelper->nodeHalfSize.x * GetBilateralEdgeOffsetPosRate()
			*/
			virtual float GetBilateralEdgeOffsetPosRate()const noexcept = 0;
		public:
			void SetNewNodeTrigger(const bool value)noexcept;
		public:
			J_SIMPLE_GET_SET(std::string, name, Name)
			J_SIMPLE_GET_SET(std::string, desc, Desc)
			J_SIMPLE_GET_SET(size_t, guid, Guid)
			J_SIMPLE_GET_SET(JVector2<float>, center, Center)
		protected:
			virtual JVector4<float> GetRectInnerColor()const noexcept = 0;
			JVector4<float> GetRectFrameColor()const noexcept;
			JVector4<float> GetRectSelectedFrameColor()const noexcept;
			JVector4<float> GetRectSelectedDeltaColor()const noexcept;
			JVector4<float> GetRectHoveredDeltaColor()const noexcept;
			JVector4<float> GetRectDraggingDeltaColor()const noexcept; 
			JVector4<float> GetDescBoxColor()const noexcept;
			JVector4<float> GetLineColor()const noexcept;
			JVector4<float> GetTriangleColor()const noexcept;
			JVector4<float> GetSelectedTriangleColor()const noexcept;
			JVector4<float> GetSelectedLineColor()const noexcept;
		public:
			bool IsSame(JEditorNodeBase* ptr)const noexcept;
			bool IsNewNode()const noexcept;
			virtual bool AllowBilateralEdge()const noexcept = 0;
		};

		class JEditorViewBase
		{
		public:
			using IsValidGroupF = Core::JSFunctorType<bool, const size_t>;
		private:
			const size_t guid;
		private:
			std::vector<std::unique_ptr<JEditorNodeBase>> allNodes;
			std::unordered_map<size_t, JEditorNodeBase*> nodeMap;
			JEditorNodeBase* lastAddedNode = nullptr; 
		private:
			std::unique_ptr<JEditorGuiCoordGrid> coordGrid;
			std::unique_ptr<JEditorViewUpdateHelper> updateHelper;
		private:
			uint maxDepth = 0; 
		private:
			bool isLastViewOpen = false;
			bool hasNewNode = false;
			bool useViewWindow = true;
		private:
			uint8 updateBit = 0;
		public:
			JEditorViewBase();
			virtual ~JEditorViewBase();
		public:
			void ClearNode(); 
		public:
			static size_t GetDefaultGroupGuid()noexcept;
			uint GetNodeCount()const noexcept;
			JEditorGuiCoordGrid* GetGrid()const noexcept;
			size_t GetLastUpdateHoveredNodeGuid()const noexcept;
			size_t GetLastUpdateSeletedNodeGuid()const noexcept;
			void GetLastUpdateHoveredEdgeGuid(_Out_ size_t& fromGuid, _Out_ size_t& toGuid)const noexcept;
			void GetLastUpdateSelectedEdgeGuid(_Out_ size_t& fromGuid, _Out_ size_t& toGuid)const noexcept;
		public:
			void SetGridSize(const uint gridSize)noexcept; 
		public:
			bool IsLastUpdateHoveredNode()const noexcept;
			bool IsLastUpdateHoveredEdge()const noexcept;
			bool IsLastUpdateSeletedNode()const noexcept;
			bool IsLastUpdateSeletedEdge()const noexcept;
		protected:
			JEditorNodeBase* GetRootNode()const noexcept;
			JEditorNodeBase* GetNode(const size_t guid)const noexcept;
			JEditorNodeBase* GetNodeByIndex(const uint index)const noexcept;
			JEditorNodeBase* GetLastHoveredNode()const noexcept;
			JEditorNodeBase* GetLastSelectedNode()const noexcept;
		public:
			J_SIMPLE_P_GET_SET(JEditorNodeBase, lastAddedNode, LastAddedNode)
			J_SIMPLE_GET(size_t, guid, Guid)
			J_SIMPLE_GET_SET(uint, maxDepth, MaxDepth)
		protected:
			JEditorNodeBase* AddNode(const size_t groupGuid, std::unique_ptr<JEditorNodeBase>&& newNode);
		public:
			void RegisterGroup(const size_t groupGuid, IsValidGroupF::Ptr ptr = nullptr)noexcept;
			bool HasGroupData(const size_t groupGuid)const noexcept;
		public:
			bool BeginView(const std::string& uniqueName, bool* isOpen, int guiWindowFlag);
			void OnScreen(const size_t groupGuid = GetDefaultGroupGuid());
			void EndView();
			void UseBeginViewWindow(const bool value)noexcept;
		protected:
			virtual void NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)noexcept = 0;
		public:
			void StoreData(const std::wstring& path);
			void LoadData(const std::wstring& path);
		};
		class JEditorTreeViewBase : public JEditorViewBase
		{
		protected:
			void NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)noexcept final;
		};
		class JEditorBinaryTreeView : public JEditorTreeViewBase, public JAcceleratorVisualizeInterface
		{
		public:
			void Initialize();
		public:
			bool IsMatch(const J_ACCELERATOR_TYPE type)const noexcept final;
		public:
			void BuildNode(const std::string& name,
				const size_t nodeGuid, 
				const size_t groupGuid = GetDefaultGroupGuid(),
				const std::string& desc = "",
				const bool isSelectedNode = false,
				const bool isSelectedParentEdge = false)noexcept;
			void BuildNode(const std::string& name, const size_t nodeGuid, const std::string& desc = "")noexcept final;
			void BuildEndSplit()noexcept final;
		};
		class JEdtiorTreeView : public JEditorTreeViewBase
		{
		public:
			void BuildNode(const std::string& name, 
				const size_t nodeGuid,
				const size_t groupGuid = GetDefaultGroupGuid(),
				const std::string& desc = "",
				const bool isSelectedNode = false,
				const bool isSelectedParentEdge = false)noexcept;
		};
		class JEditorGraphView : public JEditorViewBase
		{
		private:
			bool isConnectModeActivated = false;
			size_t connectFromGuid = 0;
			size_t connectToGuid = 0;
			std::unique_ptr<Core::JBindHandleBase> successBind = nullptr;
		public:
			JEditorGraphView() = default;
			~JEditorGraphView() = default;
		public:
			void BuildNode(const std::string& name,
				const size_t nodeGuid,
				const size_t groupGuid = GetDefaultGroupGuid(), 
				const std::string& desc = "",
				const bool isSelectedNode = false)noexcept;
			void BuildNode(const std::string& name,
				const size_t nodeGuid,
				const size_t groupGuid = GetDefaultGroupGuid(),
				const bool isSelectedNode = false)noexcept;
			void ConnectNode(const size_t from, const size_t to, const bool isSelected)noexcept;
			void SetConnectNodeMode(const size_t from, std::unique_ptr<Core::JBindHandleBase>&& newSuccessBind = nullptr)noexcept;
		protected:
			void OffConnectNodeMode()noexcept;
			void NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)noexcept final;
		public:
			J_SIMPLE_GET(size_t, connectFromGuid, ConnectFromGuid)
			J_SIMPLE_GET(size_t, connectToGuid, ConnectToGuid)
		};
	}
}