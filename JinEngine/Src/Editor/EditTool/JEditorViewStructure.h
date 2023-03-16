#pragma once 
#include"JEditorCoordGrid.h"
#include"../../Core/JDataType.h"  
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Utility/JMacroUtility.h"
#include"../../Utility/JVector.h"
#include<string>
#include<vector>
#include<memory>

namespace JinEngine
{
	namespace Editor
	{
		struct JEditorViewUpdateHelper;

		class JEditorNodeBase
		{
		private:
			std::string name;
			std::string info;
		private:
			size_t guid;
		private:
			//Center position
			JVector2<float> center;
		private:
			bool isNewNode = false;
			bool isSelected = false;
		public:
			JEditorNodeBase(const std::string& name, const size_t guid, const std::string& info, const bool isSelected);
			virtual ~JEditorNodeBase() = default;
		public:
			void DrawRect(const JEditorViewUpdateHelper* updateHelper);
			void DrawLine(const JEditorViewUpdateHelper* updateHelper, const JEditorNodeBase* to, const bool isSelecetdEdge);
			void StoreUpdatedNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
			void LoadUpdateNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
		public:
			virtual void SettingDrawLine(const JEditorViewUpdateHelper* updateHelper) = 0;
		protected:
			virtual void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept = 0;
			virtual bool DoInitializeWhenAddedNewNode()const noexcept = 0;
		public:
			bool IsSame(JEditorNodeBase* ptr)const noexcept;
			bool IsNewNode()const noexcept;
		public:
			//Apply mouse offset
			JVector2<float> GetValidCenter(const JEditorViewUpdateHelper* updateHelper)const noexcept;
			std::string GetCompressName(const JEditorViewUpdateHelper* updateHelper)const noexcept;
			void SetNewNodeTrigger(const bool value)noexcept;
		public:
			J_SIMPLE_GET_SET(std::string, name, Name)
			J_SIMPLE_GET_SET(std::string, info, Info)
			J_SIMPLE_GET_SET(size_t, guid, Guid)
			J_SIMPLE_GET_SET(JVector2<float>, center, Center)
		protected:
			virtual uint GetRectInnerColor()const noexcept = 0;
			uint GetRectFrameColor()const noexcept;
			uint GetRectSelectedFrameColor()const noexcept;
			uint GetRectHoveredDeltaColor()const noexcept;
			uint GetRectDraggingDeltaColor()const noexcept;
			uint GetRectVertexDeltaColor()const noexcept;
			uint GetTextColor()const noexcept;
			uint GetInfoBoxColor()const noexcept;
			uint GetLineColor()const noexcept;
			uint GetSelectedLineColor()const noexcept;
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
			bool IsLastUpdateHoveredNode()const noexcept;
			bool IsLastUpdateHoveredEdge()const noexcept;
			bool IsLastUpdateSeletedNode()const noexcept;
			bool IsLastUpdateSeletedEdge()const noexcept;
			void SetGridSize(const uint gridSize)noexcept;
		protected:
			JEditorNodeBase* GetRootNode()const noexcept;
			JEditorNodeBase* GetNode(const size_t guid) noexcept;
			JEditorNodeBase* GetNodeByIndex(const uint index)const noexcept;
			JEditorNodeBase* GetLastSelectedNode()noexcept;
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

		class JEditorBinaryTreeView : public JEditorTreeViewBase
		{
		public:
			void BuildNode(const std::string& name,
				const size_t nodeGuid, 
				const size_t groupGuid = GetDefaultGroupGuid(),
				const std::string& info = "",
				const bool isSelectedNode = false,
				const bool isSelectedParentEdge = false)noexcept;
			void BuildEndSplit()noexcept;
		};

		class JEdtiorTreeView : public JEditorTreeViewBase
		{
		public:
			void BuildNode(const std::string& name, 
				const size_t nodeGuid,
				const size_t groupGuid = GetDefaultGroupGuid(),
				const std::string& info = "",
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
				const std::string& info = "",
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