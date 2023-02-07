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
		public:
			JEditorNodeBase(const std::string& name, const size_t guid, const std::string& info);
			virtual ~JEditorNodeBase() = default;
		public:   
			void DrawRect(const JEditorViewUpdateHelper* updateHelper);
			virtual void DrawLine(const JEditorViewUpdateHelper* updateHelper) = 0; 
			void StoreUpdatedNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
			void LoadUpdateNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
		protected:
			virtual void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept = 0;
			virtual bool DoInitializeWhenAddedNewNode()const noexcept = 0; 
		public:
			bool IsSame(JEditorNodeBase* ptr)const noexcept;
			bool IsNewNode()const noexcept;
		public:
			//Apply mouse offset
			JVector2<float> GetValidCenter(const JEditorViewUpdateHelper* updateHelper)noexcept;
			std::string GetCompressName(const JEditorViewUpdateHelper* updateHelper)noexcept;
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
			size_t lastSelectedGuid = 0;
		private:
			std::unique_ptr<JEditorGuiCoordGrid> coordGrid;
			std::unique_ptr<JEditorViewUpdateHelper> updateHelper;
		private:
			uint maxDepth = 0;
			bool isLastViewOpen = false;
			bool hasNewNode = false;
		private:
			uint8 updateBit = 0;
		public:
			JEditorViewBase();
			~JEditorViewBase();
		public:
			void Clear();
		public: 
			static size_t GetDefaultGroupGuid()noexcept;
			uint GetNodeCount()const noexcept; 
			JEditorGuiCoordGrid* GetGrid()const noexcept; 
			void SetGridSize(const uint gridSize)noexcept; 
		protected:
			JEditorNodeBase* GetRootNode()const noexcept; 
			JEditorNodeBase* GetNode(const size_t guid) noexcept;
			JEditorNodeBase* GetNodeByIndex(const uint index)const noexcept;
			JEditorNodeBase* GetLastSelectedNode()noexcept;
			J_SIMPLE_P_GET_SET(JEditorNodeBase, lastAddedNode, LastAddedNode)
		public:
			J_SIMPLE_GET(size_t, guid, Guid)
			J_SIMPLE_GET_SET(uint, maxDepth, MaxDepth)
		protected:
			JEditorNodeBase* AddNode(const size_t groupGuid, std::unique_ptr<JEditorNodeBase>&& newNode);
		public:
			virtual void BuildNode(const std::string& name, const size_t nodeGuid, const size_t groupGuid = GetDefaultGroupGuid(), const std::string& info = "")noexcept = 0;
			void RegisterGroup(const size_t groupGuid, IsValidGroupF::Ptr ptr = nullptr)noexcept; 
			bool HasGroupData(const size_t groupGuid)const noexcept;
		public:
			bool BeginView(const std::string& uniqueName, bool* isOpen, int guiWindowFlag);
			void OnScreen(const size_t groupGuid = GetDefaultGroupGuid());
			void EndView(); 
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
			void BuildNode(const std::string& name, const size_t nodeGuid, const size_t groupGuid = GetDefaultGroupGuid(), const std::string& info = "")noexcept final;
			void BuildEndSplit()noexcept;
		};

		class JEdtiorTreeView : public JEditorTreeViewBase
		{
		public:
			void BuildNode(const std::string& name, const size_t nodeGuid, const size_t groupGuid = GetDefaultGroupGuid(), const std::string& info = "")noexcept final;
		};

		class JEditorGraphView : public JEditorViewBase
		{  
		private:
			bool isConnectModeActivated = false;
			size_t connectFromGuid = 0; 
			size_t connectToGuid = 0;
			std::unique_ptr<Core::JBindHandleBase> successBind = nullptr;
		public:
			void BuildNode(const std::string& name, const size_t nodeGuid, const size_t groupGuid = GetDefaultGroupGuid(), const std::string& info = "")noexcept final;
			void ConnectNode(const size_t from, const size_t to)noexcept;
			void SetConnectNodeMode(std::unique_ptr<Core::JBindHandleBase>&& newSuccessBind = nullptr)noexcept;
		protected:
			void OffConnectNodeMode()noexcept;
			void NodeOnScreen(const JEditorViewUpdateHelper* updateHelper)noexcept final;
		public:
			J_SIMPLE_GET(size_t, connectFromGuid, ConnectFromGuid)
			J_SIMPLE_GET(size_t, connectToGuid, ConnectToGuid)
		};
	}
}