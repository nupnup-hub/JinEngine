#pragma once  
#include<vector>
#include<memory>
#include<string>
#include"../../Core/JDataType.h"

namespace JinEngine
{
	namespace Editor
	{ 
		class JEditorAttribute;
		struct JMenuNode
		{
		private:
			const std::string nodeName;
			const std::string windowName;
			bool isRoot;
			bool isLeaf;
			bool* isOpend; // Leaf Only window attribute isOpen ptr
			JMenuNode* parent;
			std::vector<JMenuNode*> children;
		public:
			JMenuNode(const std::string& windowName, bool isRoot, bool isLeaf, bool* isOpend = nullptr, JMenuNode* parent = nullptr);
			~JMenuNode();
			JMenuNode(const JMenuNode& rhs) = delete;
			JMenuNode& operator=(const JMenuNode& rhs) = delete;
			JMenuNode(JMenuNode&& rhs) = default;
			JMenuNode& operator=(JMenuNode&& rhs) = default;

			std::string GetMenuNodeName()const noexcept;
			std::string GetWindowName()const noexcept;
			const uint GetChildrenCount()const noexcept;
			JMenuNode* GetChild(const uint index)const noexcept;
			bool IsRootNode()const noexcept;
			bool IsLeafNode()const noexcept;
			bool IsOpendNode()const noexcept; // LeftNode only
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