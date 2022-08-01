#pragma once  
#include<vector>
#include<memory>
#include<string>
#include"../../../Core/JDataType.h"

namespace JinEngine
{ 
	class EditorUtility;
	class EditorAttribute;
	struct MenuNode
	{
	private:
		const std::string nodeName;
		const std::string windowName;
		bool isRoot;
		bool isLeaf;
		bool* isOpend; // Leaf Only window attribute isOpen ptr
		MenuNode* parent;
		std::vector<MenuNode*> children;
	public:
		MenuNode(const std::string& windowName, bool isRoot, bool isLeaf, bool* isOpend = nullptr, MenuNode* parent = nullptr);
		~MenuNode();
		MenuNode(const MenuNode& rhs) = delete;
		MenuNode& operator=(const MenuNode& rhs) = delete;
		MenuNode(MenuNode&& rhs) = default;
		MenuNode& operator=(MenuNode&& rhs) = default;

		std::string GetMenuNodeName()const noexcept; 
		std::string GetWindowName()const noexcept;
		const uint GetChildrenCount()const noexcept;
		MenuNode* GetChild(const uint index)const noexcept;
		bool IsRootNode()const noexcept;
		bool IsLeafNode()const noexcept;
		bool IsOpendNode()const noexcept; // LeftNode only
	};
	class EditorMenuBar 
	{
	public:
		MenuNode* rootNode;
		MenuNode* selectedNode;
		std::vector<std::unique_ptr<MenuNode>> allNode;
	public:
		EditorMenuBar();
		~EditorMenuBar();
		MenuNode* GetSelectedNode()noexcept;
		bool UpdateMenuBar(EditorUtility* editorUtility);
	private:
		void LoopNode(MenuNode* node);
	};
}