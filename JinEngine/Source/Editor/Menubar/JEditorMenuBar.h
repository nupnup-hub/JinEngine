#pragma once  
#include<vector>
#include<memory>
#include<string>
#include"../../Core/JDataType.h"
#include"../../Core/Func/Functor/JFunctor.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"../../Core/Interface/JGroupInterface.h"

namespace JinEngine
{ 
	namespace Graphic
	{ 
		class JGraphicSingleResourceUserAccess;	//texture has one gResource
	}

	namespace Editor
	{  
		class JEditorStringMap;
		class JEditorMenuNode
		{
		private:
			const std::string nodeName; 
			bool isRoot;
			bool isLeaf;
			bool isControlOpendPtr;
			bool* isOpend; // Leaf Only window attribute isOpen ptr 
			bool isCreateOpendPtr = false;
			JEditorMenuNode* parent;
			std::vector<JEditorMenuNode*> children;
		private:
			std::unique_ptr<Core::JBindHandleBase> openBindHandle = nullptr;
			std::unique_ptr<Core::JBindHandleBase> closeBindHandle = nullptr;
			std::unique_ptr<Core::JBindHandleBase> activateBindHandle = nullptr;
			std::unique_ptr<Core::JBindHandleBase> deActivateBindHandle = nullptr;
			std::unique_ptr<Core::JBindHandleBase> updateBindHandle = nullptr;
		public:
			JEditorMenuNode(const std::string& windowName, 
				bool isRoot, 
				bool isLeaf,
				bool isControlOpendPtr = true,
				bool* isOpend = nullptr, 
				JEditorMenuNode* parent = nullptr);
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
				std::unique_ptr<Core::JBindHandleBase>&& newCloseBindHandle = nullptr,
				std::unique_ptr<Core::JBindHandleBase>&& newActivateBindHandle = nullptr,
				std::unique_ptr<Core::JBindHandleBase>&& newDeActivateBindHandle = nullptr,
				std::unique_ptr<Core::JBindHandleBase>&& newUpdateBindHandle = nullptr);
			void ExecuteOpenBind();
			void ExecuteCloseBind();
			void ExecuteActivateBind();
			void ExecuteDeActivateBind();
			void ExecuteUpdateBind();
		};
		class JEditorMenuBar
		{
		public:
			enum class EXTRA_WIDGET_TYPE
			{
				SWITCH,
				BUTTON
			};
		public:
			class ExtraWidget
			{ 
			private:  
				const size_t guid;  
			public:
				ExtraWidget(const size_t guid);
				virtual ~ExtraWidget() = default;
			public:
				virtual void Update(const JEditorStringMap* tooltipMap) = 0;
			public: 
				std::string GetUniqueLabel()const noexcept;  
			public:
				J_SIMPLE_GET(size_t, guid, Guid) 
			};
			class Icon : public ExtraWidget
			{
			public:
				using GetGResourceF = Core::JSFunctorType<Graphic::JGraphicSingleResourceUserAccess*>;
			private:
				std::unique_ptr<GetGResourceF::Functor> getGResourceFunctor;	//get icon image access
			public:
				Icon(const size_t guid, std::unique_ptr<GetGResourceF::Functor>&& getGResourceFunctor);
			protected:
				GetGResourceF::Functor* GetGResourceFunctor()const noexcept;
			protected:
				void DisplayTooltip(const JEditorStringMap* tooltipMap, 
					const JVector2<float> pos, 
					const JVector2<float> size);
			};
			class ButtonIcon : public Icon
			{
			private:
				std::unique_ptr<Core::JBindHandleBase> pressBind; 
			public:
				ButtonIcon(const size_t guid,
					std::unique_ptr<GetGResourceF::Functor>&& getGResourceFunctor,
					std::unique_ptr<Core::JBindHandleBase>&& pressBind);
			public:
				void Update(const JEditorStringMap* tooltipMap)final;
			};
			class SwitchIcon : public Icon
			{	 
			private:
				std::unique_ptr<Core::JBindHandleBase> onBind;
				std::unique_ptr<Core::JBindHandleBase> offBind; 
			private:
				bool* isActivatedPtr;
			public:
				SwitchIcon(const size_t guid,
					std::unique_ptr<GetGResourceF::Functor>&& getGResourceFunctor,
					std::unique_ptr<Core::JBindHandleBase>&& onBind,
					std::unique_ptr<Core::JBindHandleBase>&& offBind,
					bool* isActivatedPtr);
			public:
				void Update(const JEditorStringMap* tooltipMap)final;
			public: 
				bool IsActivated()const noexcept;
			};
		private:
			using LoopNodePtr = Core::JStaticCallableType<void, JEditorMenuNode*>::Ptr;
		private:
			JEditorMenuNode* rootNode;
			JEditorMenuNode* selectedNode;
			std::vector<std::unique_ptr<JEditorMenuNode>> allNode;
			std::vector<JEditorMenuNode*> leafNode;
		private:
			std::vector<std::unique_ptr<ExtraWidget>> extraWidgetVec; 
			std::unique_ptr<JEditorStringMap> editStrMap;
		private:
			const bool isMainMenu = false;
		public:
			JEditorMenuBar(std::unique_ptr<JEditorMenuNode> root, const bool isMainMenu);
			~JEditorMenuBar();
		public:
			JEditorMenuNode* GetRootNode()noexcept;
			JEditorMenuNode* GetSelectedNode()noexcept;
		public:
			void AddNode(std::unique_ptr<JEditorMenuNode> newNode)noexcept;
			void RegisterExtraWidget(std::unique_ptr<ExtraWidget>&& extraWidget)noexcept;
			//언어별 문자등록
			//주로 extraWidget에서 사용
			bool RegisterEditorString(const size_t guid, const std::vector<std::string>& strVec)noexcept;
		public:
			void Update(const bool leafNodeOnly);
		private:
			bool UpdateMenuBar();
			void UpdateExtraWidget();
			void LoopNode(JEditorMenuNode* node);
			void LoopNode(JEditorMenuNode* node, LoopNodePtr ptr);
		public:
			void ActivateOpenNode(const bool leafNodeOnly);
			void DeActivateOpenNode(const bool leafNodeOnly);
		};
	}
}