/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"JThreadType.h"
#include"JThread.h"
#include"../Func/Functor/JFunctor.h"
#include"../Singleton/JSingletonHolder.h"
#include <windows.h>  

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
			void ExtendCommonThreadCapacity(const uint count);
			void ReduceCommonThreadCapacity(const uint count);
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