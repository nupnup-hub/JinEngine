 
//Not use
/*
#pragma once
#include<string>
#include<memory>
#include"../../Core/Func/Functor/JFunctor.h"

//Not use
namespace JinEngine
{
	namespace Editor
	{
		//Caution!
		class JEditorMenuNode;
		class JEditorMenuBar;
		template<typename Object, typename EnumType, int enumCount>
		class JWindowMenubarHelper
		{
		private:
			using OpenMenuNodePtr = typename Core::JMFunctorType<Object, void, const EnumType>::Ptr;
			using ActivateMenuNodePtr = typename Core::JMFunctorType<Object, void, const EnumType>::Ptr;
			using DeActivateMenuNodePtr = typename Core::JMFunctorType<Object, void, const EnumType>::Ptr;
			using UpdateMenuNodePtr = typename Core::JMFunctorType<Object, void, const EnumType>::Ptr;
		private:
			using OpenMenuNodeFunctor = typename Core::JMFunctorType<Object, void, const EnumType>::Functor;
			using ActivateMenuNodeFunctor = typename Core::JMFunctorType<Object, void, const EnumType>::Functor;
			using DeActivateMenuNodeFunctor = typename Core::JMFunctorType<Object, void, const EnumType>::Functor;
			using UpdateMenuNodeFunctor = typename Core::JMFunctorType<Object, void, const EnumType>::Functor;
		private:
			using OpenMenuNodeBind = typename Core::JMFunctorType<Object, void, const EnumType>::CompletelyBind;
		public:
			struct MenuNodeData
			{
			public:
				bool isOpen = false;
				int selectedIndex = 0;
			};
		private:
			MenuNodeData nodeData[enumCount];
		private:
			std::unique_ptr<JEditorMenuBar> menubar;
			std::unique_ptr<OpenMenuNodeFunctor> openFunctor;
			std::unique_ptr<ActivateMenuNodeFunctor> activateFunctor;
			std::unique_ptr<DeActivateMenuNodeFunctor> deActivateFunctor;
			std::unique_ptr<UpdateMenuNodeFunctor> updateFunctor;
		public:
			JWindowMenubarHelper(Object* obj,
				OpenMenuNodePtr newOpenPtr,
				UpdateMenuNodePtr newUpdatePtr,
				const bool isMainMenubar,
				ActivateMenuNodePtr newActivatePtr = nullptr,
				DeActivateMenuNodePtr newDeActivatePtr = nullptr)
				:openFunctor(std::make_unique<OpenMenuNodeFunctor>(newOpenPtr, obj)),
				updateFunctor(std::make_unique<UpdateMenuNodeFunctor>(newUpdatePtr, obj))
			{
				menubar = std::make_unique<JEditorMenuBar>(std::make_unique<JEditorMenuNode>("Root", true, false), isMainMenubar);
				if (newActivatePtr != nullptr && newDeActivatePtr != nullptr)
				{
					activateFunctor = std::make_unique<ActivateMenuNodeFunctor>(newActivatePtr, obj);
					deActivateFunctor = std::make_unique<DeActivateMenuNodeFunctor>(newDeActivatePtr, obj);
				}
			}
		public:
			JEditorMenuNode* GetRootNode()const noexcept
			{
				return menubar->GetRootNode();
			}
			MenuNodeData* GetNodeData(const EnumType type)noexcept
			{
				return &nodeData[(int)type];
			}
		public:
			void CreateNode(const std::string& name, bool isRoot, bool isLeaf, JEditorMenuNode* parent, EnumType type)
			{
				std::unique_ptr<JEditorMenuNode> node = std::make_unique<JEditorMenuNode>(name,
					isRoot, isLeaf,
					&nodeData[(int)type].isOpen,
					parent);

				node->RegisterBind(std::make_unique<OpenMenuNodeBind>(*openFunctor, std::move(type)));
				AddNode(std::move(node));
			}
			template<typename CompletelyBind>
			void CreateNode(const std::string& name, bool isRoot, bool isLeaf, JEditorMenuNode* parent, std::unique_ptr<CompletelyBind> bind)
			{
				std::unique_ptr<JEditorMenuNode> node = std::make_unique<JEditorMenuNode>(name,
					isRoot, isLeaf,
					&nodeData[(int)type].isOpen,
					parent);

				node->RegisterBind(std::move(bind));
				AddNode(std::move(node));
			}
			void AddNode(std::unique_ptr<JEditorMenuNode> newNode)
			{
				menubar->AddNode(std::move(newNode));
			}
		public:
			void Update()
			{
				if (menubar->UpdateMenuBar())
					menubar->GetSelectedNode()->ExecuteBind();

				if (updateFunctor != nullptr)
				{
					for (int i = 0; i < (int)enumCount; ++i)
					{
						if (nodeData[i].isOpen)
							(*updateFunctor)((EnumType)i);
					}
				}
			}
		public:
			void ActivateOpenNode()
			{
				if (activateFunctor == nullptr)
					return;

				for (int i = 0; i < enumCount; ++i)
				{
					if (nodeData[i].isOpen)
						(*activateFunctor)((EnumType)i);
				}
			}
			void DeActivateOpenNode()
			{
				if (deActivateFunctor == nullptr)
					return;

				for (int i = 0; i < enumCount; ++i)
				{
					if (nodeData[i].isOpen)
						(*deActivateFunctor)((EnumType)i);
				}
			}
		};
	}
}
*/