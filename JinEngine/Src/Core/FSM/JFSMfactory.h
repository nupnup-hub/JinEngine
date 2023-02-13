#pragma once  
#include<string>
#include"JFSMInterface.h"
//#include"JFSMdiagram.h"
//#include"JFSMstate.h"
//#include"JFSMparameter.h"
#include"../../Core/Singleton/JSingletonHolder.h"
#include"../../Core/Factory/JFactory.h"
#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{
	namespace Core
	{  
		using JFSMIdentifierInitData = JFSMInterface::JFSMIdentifierInitData;
		template<typename T> class JFSMFactoryImpl; 
		 
		class JFSMfactory
		{
		private:
			template<typename T> friend class JFSMFactoryImpl; 
		private:
			Core::JFactory<std::string, false, JFSMInterface*, JOwnerPtr<JFSMIdentifierInitData>> factory;
		private:
			bool Register(const std::string& iden, Core::JCallableInterface<JFSMInterface*, JOwnerPtr<JFSMIdentifierInitData>>* callable);
			JFSMInterface* Create(const std::string& iden, JOwnerPtr<JFSMIdentifierInitData> initData);
		};

		using JFF = JSingletonHolder<JFSMfactory>;

		template<typename T>
		class JFSMFactoryImpl
		{
		private:
			friend T;
		private:
			using CreateF = JStaticCallableType<JFSMInterface*, JOwnerPtr<JFSMIdentifierInitData>>;
		public:
			static T* Create(JOwnerPtr<JFSMIdentifierInitData> owner)
			{
				return static_cast<T*>(JFF::Instance().Create(T::TypeName(), std::move(owner)));
			}
		private:
			static void Register(CreateF::Ptr createPtr)
			{
				static CreateF::Callable createCallable{ createPtr };
				JFF::Instance().Register(T::TypeName(), &createCallable);
			}
		};
		template<typename T>
		using JFFI = JFSMFactoryImpl<T>;
	}
}