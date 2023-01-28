#pragma once 
#include"JEditorCoordGrid.h"
#include"../../Core/JDataType.h"  
#include"../../Core/Guid/GuidCreator.h"
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
			void NodeOnScreen(const JEditorViewUpdateHelper* updateHelper); 
		protected:
			virtual void Initialize(const JEditorViewUpdateHelper* updateHelper)noexcept = 0; 
			virtual void DrawRect(const JEditorViewUpdateHelper* updateHelper) = 0;
			virtual void DrawLine(const JEditorViewUpdateHelper* updateHelper) = 0;
			virtual void CallChildNode(const JEditorViewUpdateHelper* updateHelper)noexcept = 0;
			virtual bool DoInitializeWhenAddedNewNode()const noexcept = 0; 
		private:
			void StoreUpdatedNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
			void LoadUpdateNodeData(const JEditorViewUpdateHelper* updateHelper)noexcept;
		public:
			bool IsSame(JEditorNodeBase* ptr)const noexcept;
			bool IsNewNode()const noexcept;
		public:
			//Apply mouse offset
			JVector2<float> GetValidCenter(const JEditorViewUpdateHelper* updateHelper)noexcept;
			std::string GetComporessName(const JEditorViewUpdateHelper* updateHelper)noexcept;
			void SetNewNodeTrigger(const bool value)noexcept;
		public:
			J_SIMPLE_GET_SET(std::string, name, Name)
			J_SIMPLE_GET_SET(std::string, info, Info)
			J_SIMPLE_GET_SET(size_t, guid, Guid)
			J_SIMPLE_GET_SET(JVector2<float>, center, Center) 
		};
		 
		class JEditorViewBase
		{
		private:
			const size_t guid;
		private:
			std::vector<std::unique_ptr<JEditorNodeBase>> allNodes;
			JEditorNodeBase* selectedNode = nullptr;
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
			size_t GetGuid()const noexcept;
			uint GetNodeCount()const noexcept; 
			JEditorNodeBase* GetRootNode()const noexcept; 
			JEditorGuiCoordGrid* GetGrid()const noexcept; 
			void SetGridSize(const uint gridSize)noexcept; 
		protected:
			JEditorNodeBase* AddNode(std::unique_ptr<JEditorNodeBase>&& newNode);
		public:
			virtual void BuildNode(const std::string& name, const size_t guid = Core::MakeGuid(), const std::string& info = " ")noexcept = 0;
		public:
			bool BeginView(const std::string& uniqueName, bool* isOpen, int windowFlag);
			void OnScreen();
			void EndView();
		public:
			J_SIMPLE_P_GET_SET(JEditorNodeBase*, selectedNode, SelectedNode)
			J_SIMPLE_GET_SET(uint, maxDepth, MaxDepth)
		};

		class JEditorBinaryTreeView : public JEditorViewBase
		{  
		public:
			void BuildNode(const std::string& name, const size_t guid = Core::MakeGuid(), const std::string& info = " ")noexcept final;
			void BuildEndSplit()noexcept;
		};

		class JEdtiorTreeView : public JEditorViewBase
		{
		public:
			void BuildNode(const std::string& name, const size_t guid = Core::MakeGuid(), const std::string& info = " ")noexcept final;
		};

		class JEditorGraphView : public JEditorViewBase
		{
		public:
			void BuildNode(const std::string& name, const size_t guid = Core::MakeGuid(), const std::string& info = " ")noexcept final;
			void ConncetNode(const size_t from, const size_t to);
		};
	}
}