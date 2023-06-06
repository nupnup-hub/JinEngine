#pragma once
#include"JThreadType.h"
#include"JThread.h"
#include"../Func/Functor/JFunctor.h"
#include"../Singleton/JSingletonHolder.h"
#include <windows.h> 
#include<vector>

namespace JinEngine
{
	namespace Core
	{
		class JThreadManagerPrivate;
		class JThreadManager
		{
		private:
			template<typename T>friend class JCreateUsingNew;
		private:
			friend class JThreadManagerPrivate;
			class JThreadManagerImpl;
		private:
			std::unique_ptr<JThreadManagerImpl> impl; 
		public:
			uint GetReservedSpaceCount(const J_THREAD_USE_CASE_TYPE type);
		public:
			template<typename Pointer, typename ...Param>
			size_t CreateThread(const JThreadInitInfo& initInfo, Pointer pointer, Param&&... param)
			{
				auto bind = UniqueBind(std::make_unique<JFunctor<void, Param...>>(pointer), std::forward<Param>(param)...);
				return DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::COMMON, std::move(bind));
			}
			template<typename Pointer, typename Object, typename ...Param>
			size_t CreateThread(const JThreadInitInfo& initInfo, Pointer pointer, Object* obj, Param&&... param)
			{
				auto bind = UniqueBind(std::make_unique<JFunctor<void, Param...>>(pointer, obj), std::forward<Param>(param)...);
				return DoCreateThread(initInfo, J_THREAD_USE_CASE_TYPE::COMMON, std::move(bind));
			}
			size_t CreateThread(const JThreadInitInfo& initInfo, std::unique_ptr<JBindHandleBase>&& bind);
		private:
			size_t DoCreateThread(const JThreadInitInfo& initInfo, const J_THREAD_USE_CASE_TYPE useCase, std::unique_ptr<JBindHandleBase>&& bind);
		private:
			JThreadManager();
			~JThreadManager();
			JThreadManager(const JThreadManager& rhs) = delete;
			JThreadManager& operator=(const JThreadManager& rhs) = delete;
		};
	}
	using _JThreadManager = Core::JSingletonHolder<JinEngine::Core::JThreadManager>;
}