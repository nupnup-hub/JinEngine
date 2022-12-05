#pragma once  
#include<vector>
#include<memory>
#include<string>
#include"../../Core/JDataType.h"
#include"../../Core/Func/Functor/JFunctor.h"

namespace JinEngine
{
	namespace Editor
	{  
		struct JMenuNode
		{
		private:
			const std::string nodeName; 
			bool isRoot;
			bool isLeaf;
			bool* isOpend; // Leaf Only window attribute isOpen ptr
			bool isCreateOpendPtr = false;
			JMenuNode* parent;
			std::vector<JMenuNode*> children;
		private:
			std::unique_ptr<Core::JBindHandleBase> bindHandle = nullptr;
		public:
			JMenuNode(const std::string& windowName, bool isRoot, bool isLeaf, bool* isOpend = nullptr, JMenuNode* parent = nullptr);
			~JMenuNode();
			JMenuNode(const JMenuNode& rhs) = delete;
			JMenuNode& operator=(const JMenuNode& rhs) = delete;
			JMenuNode(JMenuNode&& rhs) = default;
			JMenuNode& operator=(JMenuNode&& rhs) = default;
		public:
			std::string GetNodeName()const noexcept; 
			const uint GetChildrenCount()const noexcept;
			JMenuNode* GetChild(const uint index)const noexcept;
			bool IsRootNode()const noexcept;
			bool IsLeafNode()const noexcept;
			bool IsOpendNode()const noexcept; // LeftNode only
		public:
			void RegisterBind(std::unique_ptr<Core::JBindHandleBase>&& newBindHandle);
			void ExecuteBind();
		};
		class JEditorMenuBar
		{
		public:
			JMenuNode* rootNode;
			JMenuNode* selectedNode;
			std::vector<std::unique_ptr<JMenuNode>> allNode;
		public:
			JEditorMenuBar();
			~JEditorMenuBar();
			JMenuNode* GetSelectedNode()noexcept;
			bool UpdateMenuBar();
		private:
			void LoopNode(JMenuNode* node);
		};
	}
}