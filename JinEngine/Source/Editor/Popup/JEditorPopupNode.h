#pragma once
#include"JEditorPopupType.h" 
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Math/JVector.h"
#include<vector>
#include<string> 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorStringMap;
		class JEditorWindow;
		class JEditorPopupNode
		{
		public:
			using EnableF = Core::JSFunctorType<bool, JEditorWindow*>;
		private:
			const std::string name;
			const J_EDITOR_POPUP_NODE_TYPE nodeType;
			const size_t nodeId;
		private:
			JEditorPopupNode* parent;
			std::vector<JEditorPopupNode*> children;
		private:
			const bool hasTooltip;
			const bool hasShortCut;
			const size_t tooltipId;
			const size_t shortCutId;
		private:
			JVector2<float> popupPos;	//Root Internal
			JVector2<float> popupSize;	//Root Internal
			bool isOpen; 		//Internal
			bool isActivated;	//Toggle
		private:
			std::unique_ptr<Core::JBindHandleBase> selectBind;	//valid in leaf node 
			std::unique_ptr<EnableF::CompletelyBind> enableBind;	//valid in leaf node 
		public:
			JEditorPopupNode(const std::string name,
				const J_EDITOR_POPUP_NODE_TYPE nodeType,
				JEditorPopupNode* parent,
				const bool hasTooltip = false,
				const bool hasShortCut = false);
			~JEditorPopupNode();
		public:
			void RegisterSelectBind(std::unique_ptr<Core::JBindHandleBase>&& newSelectBind)noexcept;
			void RegisterEnableBind(std::unique_ptr<EnableF::CompletelyBind>&& newEnableBind)noexcept;
			void InvokeSelectBind()noexcept;
		public:
			JEditorPopupNode* PopupOnScreen(_In_ JEditorStringMap* editorString);
			void PrintTooltip(_In_ JEditorStringMap* editorString)noexcept;
		public:
			size_t GetNodeId()const noexcept;
			size_t GetTooltipId()const noexcept;
			size_t GetShortCutId()const noexcept;
			J_EDITOR_POPUP_NODE_TYPE GetNodeType()const noexcept;
		public:
			bool IsOpen()const noexcept;
			bool IsMouseInPopup()const noexcept;
		};
	}
}