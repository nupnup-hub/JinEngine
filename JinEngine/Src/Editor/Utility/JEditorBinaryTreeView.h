#pragma once
#include"JEditorCoordGrid.h"
#include"../../Core/JDataType.h"
#include"../../Utility/JVector.h"
#include<string>
#include<vector>
#include<memory>

namespace JinEngine
{
	namespace Editor
	{
		class JEditorBinaryTreeView
		{
		private:
			struct JNodeSizeInfo
			{
			public:
				uint width;
				uint height;
				uint paddingX;
				uint paddingY;
				uint spacingX;
				uint spacingY;
			public:
				uint halfWidth;
				uint halfHeight; 
			public:
				float lineThickness;
			public:
				const uint maxDepth;
			public:
				JVector2<float> mouseOffset = JVector2<float>(0, 0);
			public:
				JNodeSizeInfo(const uint width,
					const uint height,
					const uint paddingX,
					const uint paddingY,
					const uint spacingX,
					const uint spacingY,
					const JVector2<float> mouseOffset,
					const float zoomRate,
					const uint maxDepth);
			};
		private:
			class JNode
			{
			public:
				JNode* parent = nullptr;
				JNode* left = nullptr;
				JNode* right = nullptr;
			public:
				std::string name;
				std::string info;
			private: 
				bool isLeft = false;
			public:
				JNode(JNode* parent, const std::string& name, const std::string& info); 
			public:
				void NodeOnScreen(const JNodeSizeInfo& sizeInfo, const float centerX, const float centerY, const uint depth);
			public:
				bool IsRoot()const noexcept;
				bool IsLeaf()const noexcept;
			};
		private:
			std::vector<std::unique_ptr<JNode>> allNodes;
			JNode* nowNode = nullptr;
			uint nowNodeIndex = 0;
		private: 
			uint depth = 0;
			uint maxDepth = 0;
		private:
			uint nodeCount = 0; 
		private:
			float nodeWidthRate = 0.05f;
			float nodeHeightRate = 0.05f;
			float nodePaddingXRate = 0.01f;
			float nodePaddingYRate = 0.01f;
			float nodeSpacingXRate = 0.05f;
			float nodeSpacingYRate = 0.05f; 
		private:
			JEditorGuiCoordGrid coordGrid;
		private:
			bool isClickedTitleBar = false;
		public:
			void Initialize(const uint nodeCount)noexcept;
		public:
			void Clear()noexcept;
		public:
			void BuildNode(const std::string& name, const std::string& info = " ")noexcept;
			void EndLeft()noexcept;
			void EndRight()noexcept;
		public:
			void TreeOnScreen(const std::string& treeWindowName, bool& isOpen);
		};
	}
}