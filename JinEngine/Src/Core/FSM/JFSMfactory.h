#pragma once  
#include<string>
#include"JFSMdiagram.h"
#include"JFSMstate.h"
#include"JFSMcondition.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include"../../Core/Factory/JFactory.h"
#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{
	namespace Core
	{
		using JFSMdiagramInitData = JFSMdiagram::JFSMdiagramInitData;
		using JFSMstateInitData = JFSMstate::JFSMstateInitData;
		using JFSMconditionInitData = JFSMcondition::JFSMconditionInitData;

		template<typename T> class JFSMdiagramFactoryImpl;
		template<typename T> class JFSMstateFactoryImpl;
		template<typename T> class JFSMconditionFactoryImpl;
		namespace
		{
			//Core::JFactory<std::string, false, JComponent*, JGameObject*> defaultFactory;
			class JFSMfactory
			{
			private:
				template<typename T> friend class JFSMdiagramFactoryImpl;
				template<typename T> friend class JFSMstateFactoryImpl;
				template<typename T> friend class JFSMconditionFactoryImpl;
			private:
				Core::JFactory<std::string, false, JFSMdiagram*, JOwnerPtr<JFSMdiagramInitData>> diagramFactory;
				Core::JFactory<std::string, false, JFSMstate*, JOwnerPtr<JFSMstateInitData>> stateFactory;
				Core::JFactory<std::string, false, JFSMcondition*, JOwnerPtr<JFSMconditionInitData>> conditionFactory;
			private:		 
				template<typename Type>
				bool RegisterDiagram(Core::JCallableInterface<JFSMdiagram*, JOwnerPtr<JFSMdiagramInitData>>* callable)
				{
					if constexpr (std::is_base_of_v <JFSMdiagram, Type >)
						return diagramFactory.Register(Type::TypeName(), callable);
					else
						return false;
				}
				template<typename Type>
				bool RegisterState(Core::JCallableInterface<JFSMstate*, JOwnerPtr<JFSMstateInitData>>* callable)
				{
					if constexpr (std::is_base_of_v <JFSMstate, Type >)
						return stateFactory.Register(Type::TypeName(), callable);
					else
						return false;
				}
				template<typename Type>
				bool RegisterCondition(Core::JCallableInterface<JFSMcondition*, JOwnerPtr<JFSMconditionInitData>>* callable)
				{
					if constexpr (std::is_base_of_v <JFSMcondition, Type >)
						return conditionFactory.Register(Type::TypeName(), callable);
					else
						return false;
				}
				template<typename Type>
				Type* CreateDiagram(JOwnerPtr<JFSMdiagramInitData> initData)
				{
					return static_cast<Type*>(diagramFactory.Invoke(Type::TypeName(), std::move(initData)));
				}
				template<typename Type>
				Type* CreateState(JOwnerPtr<JFSMstateInitData> initData)
				{
					return static_cast<Type*>(stateFactory.Invoke(Type::TypeName(), std::move(initData)));
				}
				template<typename Type>
				Type* CreateCondition(JOwnerPtr<JFSMconditionInitData> initData)
				{
					return static_cast<Type*>(conditionFactory.Invoke(Type::TypeName(), std::move(initData)));
				}
			};
		}

		using JFF = JSingletonHolder<JFSMfactory>;	 

		template<typename T>
		class JFSMdiagramFactoryImpl
		{
		private:
			friend T;
		private:
			using CreateDiagramF = JStaticCallableType<JFSMdiagram*, JOwnerPtr<JFSMdiagramInitData>>;
		public:
			static T* Create(JOwnerPtr<JFSMdiagramInitData> owner)
			{
				return JFF::Instance().CreateDiagram<T>(std::move(owner));
			}
		private:
			static void RegisterDiagram(CreateDiagramF::Ptr createPtr)
			{
				static CreateDiagramF::Callable createCallable{ createPtr };
				JFF::Instance().RegisterDiagram<T>(&createCallable);
			}
		};
		 
		template<typename T>
		class JFSMstateFactoryImpl
		{
		private:
			friend T;
		private:
			using CreateStateF = JStaticCallableType<JFSMstate*, JOwnerPtr<JFSMstateInitData>>;
		public:
			static T* Create(JOwnerPtr<JFSMstateInitData> owner)
			{
				return JFF::Instance().CreateState<T>(std::move(owner));
			}
		private:
			static void RegisterState(CreateStateF::Ptr createPtr)
			{
				static CreateStateF::Callable createCallable{ createPtr };
				JFF::Instance().RegisterState<T>(&createCallable);
			}
		};

		template<typename T>
		class JFSMconditionFactoryImpl
		{
		private:
			friend T;
		private:
			using CreateConditionF = JStaticCallableType<JFSMcondition*, JOwnerPtr<JFSMconditionInitData>>;
		public:
			static T* Create(JOwnerPtr<JFSMconditionInitData> owner)
			{
				return JFF::Instance().CreateCondition<T>(std::move(owner));
			}
		private:
			static void RegisterCondition(CreateConditionF::Ptr createPtr)
			{
				static CreateConditionF::Callable createCallable{ createPtr };
				JFF::Instance().RegisterCondition<T>(&createCallable);
			}
		};

		template<typename T>
		using JFDFI = JFSMdiagramFactoryImpl<T>;
		template<typename T>
		using JFSFI = JFSMstateFactoryImpl<T>;
		template<typename T>
		using JFCFI = JFSMconditionFactoryImpl<T>;
	}
}