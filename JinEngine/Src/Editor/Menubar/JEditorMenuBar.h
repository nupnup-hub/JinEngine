#pragma once  
#include<vector>
#include<memory>
#include<string>
#include"../../Core/JDataType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{
	namespace Editor
	{  
		class JEditorMenuNode
		{
		private:
			const std::string nodeName; 
			bool isRoot;
			bool isLeaf;
			bool* isOpend; // Leaf Only window attribute isOpen ptr 
			bool isCreateOpendPtr = false;
			JEditorMenuNode* parent;
			std::vector<JEditorMenuNode*> children;
		private:
			std::unique_ptr<Core::JBindHandleBase> openBindHandle = nullptr;
			std::unique_ptr<Core::JBindHandleBase> activateBindHandle = nullptr;
			std::unique_ptr<Core::JBindHandleBase> deActivateBindHandle = nullptr;
			std::unique_ptr<Core::JBindHandleBase> updateBindHandle = nullptr;
		public:
			JEditorMenuNode(const std::string& windowName, bool isRoot, bool isLeaf, bool* isOpend = nullptr, JEditorMenuNode* parent = nullptr);
			~JEditorMenuNode();
			JEditorMenuNode(const JEditorMenuNode& rhs) = delete;
			JEditorMenuNode& operator=(const JEditorMenuNode& rhs) = delete;
			JEditorMenuNode(JEditorMenuNode&& rhs) = default;
			JEditorMenuNode& operator=(JEditorMenuNode&& rhs) = default;
		public:
			std::string GetNodeName()const noexcept; 
			const uint GetChildrenCount()const noexcept;
			JEditorMenuNode* GetParent()const noexcept;
			JEditorMenuNode* GetChild(const uint index)const noexcept;
			bool IsRootNode()const noexcept;
			bool IsLeafNode()const noexcept;
			bool IsOpendNode()const noexcept; // LeftNode only
		public:
			void RegisterBindHandle(std::unique_ptr<Core::JBindHandleBase>&& newOpenBindHandle = nullptr,
				std::unique_ptr<Core::JBindHandleBase>&& newActivateBindHandle = nullptr,
				std::unique_ptr<Core::JBindHandleBase>&& newDeActivateBindHandle = nullptr,
				std::unique_ptr<Core::JBindHandleBase>&& newUpdateBindHandle = nullptr);
			void ExecuteOpenBind();
			void ExecuteActivateBind();
			void ExecuteDeActivateBind();
			void ExecuteUpdateBind();
		};
		class JEditorMenuBar
		{
		private:
			using LoopNodePtr = Core::JStaticCallableType<void, JEditorMenuNode*>::Ptr;
		private:
			JEditorMenuNode* rootNode;
			JEditorMenuNode* selectedNode;
			std::vector<std::unique_ptr<JEditorMenuNode>> allNode;
			std::vector<JEditorMenuNode*> leafNode;
		private:
			const bool isMainMenu = false;
		public:
			JEditorMenuBar(std::unique_ptr<JEditorMenuNode> root, const bool isMainMenu);
			~JEditorMenuBar();
		public:
			JEditorMenuNode* GetRootNode()noexcept;
			JEditorMenuNode* GetSelectedNode()noexcept;
		public:
			void AddNode(std::unique_ptr<JEditorMenuNode> newNode); 
		public:
			void Update(const bool leafNodeOnly);
		private:
			bool UpdateMenuBar();
			void LoopNode(JEditorMenuNode* node);
			void LoopNode(JEditorMenuNode* node, LoopNodePtr ptr);
		public:
			void ActivateOpenNode(const bool leafNodeOnly);
			void DeActivateOpenNode(const bool leafNodeOnly);
		};
	}
}