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
#include"../../Reflection/JParameter.h"
#include"../../Empty/JEmptyType.h"
#include"../../Utility/JTypeTraitUtility.h" 
#include"../../../../ThirdParty/Loki/Typelist/Typelist.h"   
#include<utility> 

namespace JinEngine
{
	namespace Core
	{
#pragma region Impl
		template<typename Ret>
		class FunctionImplBase
		{
		public:
			virtual ~FunctionImplBase() {}
		};

		template <typename Ret, typename ...Param>
		class IFunctionImpl
		{
		public:
			virtual Ret operator()(Param... var) = 0;
		};

		template<typename Pointer, typename Ret, typename ...Param>
		class StaticFunctionImpl final : public IFunctionImpl<Ret, Param...>
		{
		private:
			Pointer ptr;
		public:
			StaticFunctionImpl(Pointer ptr)
				:ptr(ptr)
			{}
		public:
			Ret operator()(Param... var)
			{
				return (*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Pointer, typename Object, typename Ret, typename ...Param>
		class MemberFunctionImpl final : public IFunctionImpl<Ret, Param...>
		{
		private:
			Pointer ptr;
			Object* object;
		public:
			MemberFunctionImpl(Pointer ptr, Object* object)
				:ptr(ptr), object(object)
			{}
		public:
			Ret operator()(Param... var)
			{  
				return ((object)->*ptr)(std::forward<Param>(var)...);
			}
		};
#pragma endregion
#pragma region JFunctor 
		template<typename Ret, typename ...Param>
		class JFunctor
		{
			//REGISTER_CLASS(JFunctor);
		public:
			using Impl = IFunctionImpl<Ret, Param...>;
			using Return = Ret;
			using ParamList = typename MakeTypelist<Param...>::Result;
			using DecayParamList = typename MakeTypelist<std::decay_t<Param>...>::Result;
		protected:
			std::unique_ptr<Impl> impl;
		public: 
			JFunctor(const JFunctor& rhs) = delete;
			JFunctor(JFunctor&& rhs) = default;
			JFunctor& operator=(const JFunctor& rhs) = delete;	 
			JFunctor& operator=(JFunctor&& rhs) = default;
			JFunctor() = default;
			JFunctor(std::unique_ptr<Impl> impl)
				:impl(std::move(impl))
			{}
			JFunctor(Ret(*ptr)(Param...))
				: impl(std::make_unique<StaticFunctionImpl<Ret(*)(Param...), Ret, Param...>>(ptr))
			{}
			template<typename Object>
			JFunctor(Ret(Object::* ptr)(Param...), Object* object)
				: impl(std::make_unique<MemberFunctionImpl<Ret(Object::*)(Param...), Object, Ret, Param...>>(ptr, object))
			{}
			template<typename Object>
			JFunctor(Ret(Object::* ptr)(Param...)const, Object* object)
				:impl(std::make_unique<MemberFunctionImpl<Ret(Object::*)(Param...)const, Object, Ret, Param...>>(ptr, object))
			{}
			~JFunctor() {};
		public:
			Ret operator()(Param... var)
			{
				return (*impl)(std::forward<Param>(var)...);
			}
			Ret Invoke(Param... var)
			{
				return (*impl)(std::forward<Param>(var)...);
			}
		};
#pragma endregion
#pragma region Bind
		class JBindHandleBase
		{
			//REGISTER_CLASS(JBindHandleBase);
		public:
			virtual ~JBindHandleBase() {}
		public:
			virtual void InvokeCompletelyBind() = 0;
			/**
			* @return 반환하는 type이 bool이 아닐시 함수를 수행하지않고 false를 반환
			*/
			virtual bool InvokeCompletelyBindRetBoolean() = 0;
		public:
			virtual bool IsCompletelyBind()const = 0;
			virtual bool IsSameReturnType(const J_PARAMETER_TYPE paramType)const = 0;
		};

		template<typename OriFunctor, typename ...BindParam>
		class JBindHandle final : public JBindHandleBase
		{
		private:
			using BindTuple = std::tuple<BindParam...>;
			using Ret = typename OriFunctor::Return;
			using OriParamList = typename OriFunctor::ParamList;
			using DecayParamList = typename OriFunctor::DecayParamList;
		private:
			OriFunctor* rawFunctor = nullptr;
			std::unique_ptr<OriFunctor> unqFunctor = nullptr;
			BindTuple bindTuple;
		private: 
			//	res: a condition of bindtuple match element type
			//		true : is bindtuple element
			//		false ; is passtuple element
			//	head: a tuple index
			template<bool IsBindParam, size_t Index>
			struct ParamSelector;
			template<size_t Index>
			struct ParamSelector<true, Index>
			{
			public:
				static constexpr int nextPassIndex = Index;
			};
			template<size_t Index>
			struct ParamSelector<false, Index>
			{
			public:
				static constexpr int nextPassIndex = Index + 1;
			};

			template<typename BindTuple, size_t Index>
			struct ParamSequence
			{
			private:
				using PreSequence = ParamSequence<BindTuple, Index - 1>;
			public:
				static constexpr bool isBindParam = !std::is_same_v <RemoveAll_T<std::tuple_element_t<Index, BindTuple>>, JEmptyType>;
				static constexpr bool isCompletely = PreSequence::isCompletely && isBindParam;
				static constexpr int nextPassIndex = ParamSelector<isBindParam, PreSequence::nextPassIndex>::nextPassIndex;
				static constexpr int passIndex = nextPassIndex - 1;
			};
			template<typename BindTuple>
			struct ParamSequence<BindTuple, 0>
			{
			public:
				static constexpr bool isBindParam = !std::is_same_v <RemoveAll_T<std::tuple_element_t<0, BindTuple>>, JEmptyType>;
				static constexpr bool isCompletely = isBindParam;
				static constexpr int nextPassIndex = ParamSelector<isBindParam, 0>::nextPassIndex;
				static constexpr int passIndex = nextPassIndex - 1;
			};
		public:
			JBindHandle(OriFunctor& oriFunctor, BindParam&&... var)
				:rawFunctor(&oriFunctor), bindTuple(std::forward<BindParam>(var)...)
			{}
			JBindHandle(std::unique_ptr<OriFunctor>&& unqFunctor, BindParam&&... var)
				:unqFunctor(std::move(unqFunctor)), bindTuple(std::forward<BindParam>(var)...)
			{}
			~JBindHandle() {}
			//JBindHandle(const JBindHandle& rhs) = default;
			//JBindHandle& operator=(const JBindHandle& rhs) = default;
			//JBindHandle(JBindHandle&& rhs) = default;
			//JBindHandle& operator=(JBindHandle&& rhs) = default;
		private:
			template<size_t ParamIndex, typename PassTuple>
			constexpr decltype(auto) GetParam(PassTuple& passTuple)
			{
				using Seq = ParamSequence<BindTuple, ParamIndex>;
				if constexpr (Seq::isBindParam)
				{
					using BindEleType = std::tuple_element_t<ParamIndex, BindTuple>;
					return std::forward<BindEleType>(std::get<ParamIndex>(bindTuple));
				}
				else
				{
					using PassEleType = std::tuple_element_t<Seq::passIndex, PassTuple>;
					return std::forward<PassEleType>(std::get<Seq::passIndex>(passTuple));
				}
			}
			template<size_t ParamIndex>
			constexpr decltype(auto) GetParam()
			{
				using Seq = ParamSequence<BindTuple, ParamIndex>;
				if constexpr (Seq::isBindParam)
				{
					using BindEleType = std::tuple_element_t<ParamIndex, BindTuple>;
					return std::forward<BindEleType>(std::get<ParamIndex>(bindTuple));
				}
			}
		private:
			template<size_t ...Is, typename PassTuple>
			auto CallFunc(std::index_sequence<Is...>, PassTuple& passTuple)
			{
				if (rawFunctor != nullptr)
					return (*rawFunctor)(GetParam<Is>(passTuple)...);
				else
					return (*unqFunctor)(GetParam<Is>(passTuple)...);
			}
			template<typename FixedRet, size_t ...Is>
			FixedRet CallFunc(std::index_sequence<Is...>)
			{
				constexpr int bindParamCount = sizeof...(BindParam);
				constexpr int seqIndex = bindParamCount - 1;
				if constexpr(std::is_same_v<FixedRet, void>)
				{
					if constexpr (bindParamCount == 0)
					{
						if (rawFunctor != nullptr)
							(*rawFunctor)(GetParam<Is>()...);
						else
							(*unqFunctor)(GetParam<Is>()...);
					}
					else if constexpr (ParamSequence<BindTuple, seqIndex>::isCompletely)
					{
						if (rawFunctor != nullptr)
							(*rawFunctor)(GetParam<Is>()...);
						else
							(*unqFunctor)(GetParam<Is>()...);
					} 
				}
				else
				{
					if constexpr (bindParamCount == 0)
					{
						if (rawFunctor != nullptr)
							return (*rawFunctor)(GetParam<Is>()...);
						else
							return (*unqFunctor)(GetParam<Is>()...);
					}
					else if constexpr (ParamSequence<BindTuple, seqIndex>::isCompletely)
					{
						if (rawFunctor != nullptr)
							return (*rawFunctor)(GetParam<Is>()...);
						else
							return (*unqFunctor)(GetParam<Is>()...);
					}
					else
					{
						if constexpr (IsPointer_V<Ret>)
							return nullptr;
						else if constexpr (IsClass_V<Ret>)
							return Ret();
						else
							return 0;
					}
				}
			}
		public:
			template<typename ...PassP>
			decltype(auto) operator()(PassP&&... var)  
			{
				auto passT = std::forward_as_tuple(std::forward<PassP>(var)...);
				return CallFunc(std::make_index_sequence<std::tuple_size_v<BindTuple>>(), passT);
			}
			template<typename ...PassP>
			decltype(auto) Invoke(PassP&&... var) 
			{
				auto passT = std::forward_as_tuple(std::forward<PassP>(var)...);
				return CallFunc(std::make_index_sequence<std::tuple_size_v<BindTuple>>(), passT);
			}
			void InvokeCompletelyBind()
			{
				CallFunc<void>(std::make_index_sequence<std::tuple_size_v<BindTuple>>());
			}
			bool InvokeCompletelyBindRetBoolean()
			{
				if constexpr (std::is_same_v<Ret, bool>)
					return CallFunc<bool>(std::make_index_sequence<std::tuple_size_v<BindTuple>>());
				else
					return false;
			}
		public:
			constexpr bool IsCompletelyBind()const
			{
				constexpr int bindParamCount = sizeof...(BindParam);
				constexpr int seqIndex = bindParamCount - 1;
				if constexpr (bindParamCount == 0)
					return true;
				else if constexpr (ParamSequence<BindTuple, seqIndex>::isCompletely)
					return true;
				else
					return false;
			}
			bool IsSameReturnType(const J_PARAMETER_TYPE paramType)const
			{
				return GetParameterType<Ret>() == paramType;
			}
			template<size_t index>
			decltype(auto) Get()
			{
				return std::get<index>(bindTuple);
			}
		};

		template<typename Ret, typename ...Param, typename ...BindParam>
		JBindHandle<JFunctor<Ret, Param...>, BindParam...> Bind(JFunctor<Ret, Param...>& functor, BindParam&&... bindVar)
		{
			return JBindHandle<JFunctor<Ret, Param...>, BindParam...>(functor, std::forward<BindParam>(bindVar)...);
		}
		template<typename Ret, typename ...Param, typename ...BindParam>
		JBindHandle<JFunctor<Ret, Param...>, BindParam...> Bind(std::unique_ptr< JFunctor<Ret, Param...>>&& unqFunctor, BindParam&&... bindVar)
		{
			return JBindHandle<JFunctor<Ret, Param...>, BindParam...>(std::move(unqFunctor), std::forward<BindParam>(bindVar)...);
		}

		template<typename Ret, typename ...Param, typename ...BindParam>
		std::unique_ptr<JBindHandle<JFunctor<Ret, Param...>, BindParam...>> UniqueBind(std::unique_ptr<JFunctor<Ret, Param...>>&& unqFunctor, BindParam&&... bindVar)
		{
			return  std::make_unique<JBindHandle<JFunctor<Ret, Param...>, BindParam...>>(std::move(unqFunctor), std::forward<BindParam>(bindVar)...);
		}
		template<typename Ret, typename ...Param, typename ...BindParam>
		std::unique_ptr<JBindHandle<JFunctor<Ret, Param...>, BindParam...>> UniqueBind(JFunctor<Ret, Param...>& functor, BindParam&&... bindVar)
		{
			return  std::make_unique<JBindHandle<JFunctor<Ret, Param...>, BindParam...>>(functor, std::forward<BindParam>(bindVar)...);
		}

		template<typename Ret, typename ...Param>
		struct JSFunctorType
		{
		public:
			using Ptr = Ret(*)(Param...);
			using Functor = JFunctor<Ret, Param...>;
			using CompletelyBind = JBindHandle<Functor, Param...>;
		};
		template<typename Object, typename Ret, typename ...Param>
		struct JMFunctorType
		{
		public:
			using Ptr = Ret(Object::*)(Param...); 
			using CPtr = Ret(Object::*)(Param...)const; 
			using Functor = JFunctor<Ret, Param...>;
			using CompletelyBind = JBindHandle<Functor, Param...>;
		};
#pragma endregion
	}
}