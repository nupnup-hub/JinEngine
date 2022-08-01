#pragma once 
#include<utility>
#include<memory> 
#include<vector> 
#include"../../Typelist/Typelist.h"  
#include"../../Reflection/JReflection.h"
#include"../../../Utility/JTypeUtility.h"

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
		class FunctionImpl : public IFunctionImpl<Ret, Param...>
		{
		private:
			Pointer ptr;
		public:
			FunctionImpl(Pointer ptr)
				:ptr(ptr)
			{}
			Ret operator()(Param... var)
			{
				return (*ptr)(std::forward<Param>(var)...);
			}
		};

		template<typename Pointer, typename Object, typename Ret, typename ...Param>
		class MemberFunctionImpl : public IFunctionImpl<Ret, Param...>
		{
		private:
			Pointer ptr;
			Object* object;
		public:
			MemberFunctionImpl(Pointer ptr, Object* object)
				:ptr(ptr), object(object)
			{}
			Ret operator()(Param... var)
			{
				return ((object)->*ptr)(std::forward<Param>(var)...);
			}
		};

#pragma endregion
#pragma region JFunctor 
		class JFunctorBase
		{
			//REGISTER_CLASS(JFunctorBase);
		public:
			virtual ~JFunctorBase() {}
		};

		template<typename Ret, typename ...Param>
		class JFunctor : public JFunctorBase
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
			JFunctor(const JFunctor&) = default;
			JFunctor& operator=(const JFunctor&) = default;
			~JFunctor() {};

			JFunctor(std::unique_ptr<Impl> impl)
				:impl(std::move(impl))
			{}
			template<typename Pointer>
			JFunctor(Pointer ptr)
				: impl(std::make_unique<FunctionImpl<Pointer, Ret, Param...>>(ptr))
			{}

			template<typename Pointer, typename Object>
			JFunctor(Pointer ptr, Object* object)
				: impl(std::make_unique<MemberFunctionImpl<Pointer, Object, Ret, Param...>>(ptr, object))
			{}

			Ret operator()(Param... var)
			{
				return (*impl)(std::forward<Param>(var)...);
			}

			Ret Invoke(Param... var)
			{
				return (*impl)(std::forward<Param>(var)...);
			}
		private:

		};
#pragma endregion
#pragma region Bind
		class JBindHandleBase
		{
			//REGISTER_CLASS(JBindHandleBase);
		public:
			virtual ~JBindHandleBase() {}
		};

		template<typename OriFunctor, typename ...BindParam>
		class JBindHandle : public JBindHandleBase
		{
		private: 
			using BindTuple = std::tuple<BindParam&&...>;
			using Ret = typename OriFunctor::Return;
			using OriParamList = typename OriFunctor::ParamList;
			using DecayParamList = typename OriFunctor::DecayParamList;
		private:
			OriFunctor& oriFunctor;
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
				static constexpr bool isBindParam = !std::is_same_v <RemoveAll_T<std::tuple_element_t<Index, BindTuple>>, EmptyType>;
				static constexpr int nextPassIndex = ParamSelector<isBindParam, PreSequence::nextPassIndex>::nextPassIndex;
				static constexpr int passIndex = nextPassIndex - 1;
			};
			template<typename BindTuple>
			struct ParamSequence<BindTuple, 0>
			{
			public:
				static constexpr bool isBindParam = !std::is_same_v <RemoveAll_T<std::tuple_element_t<0, BindTuple>>, EmptyType>;
				static constexpr int nextPassIndex = ParamSelector<isBindParam, 0>::nextPassIndex;
				static constexpr int passIndex = nextPassIndex - 1;
			};
		public:
			JBindHandle(OriFunctor& oriFunctor, BindParam&&... var)
				:oriFunctor(oriFunctor), bindTuple(std::forward<BindParam>(var)...)
			{}
			~JBindHandle()
			{}
		private:
			template<size_t ParamIndex, typename PassTuple>
			constexpr decltype(auto) GetParam(const PassTuple& passTuple)
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

			template<size_t ...Is, typename PassTuple>
			Ret CallFunc(std::index_sequence<Is...>, const PassTuple& passTuple)
			{	
				return oriFunctor(GetParam<Is>(passTuple)...);
			}
		public:
			template<typename ...PassP>
			auto operator()(PassP&&... var)
			{
				return CallFunc(std::make_index_sequence<std::tuple_size_v<BindTuple>>(), std::forward_as_tuple(std::forward<PassP>(var)...));
			}
			template<typename ...PassP>
			auto Invoke(PassP&&... var)
			{
				return CallFunc(std::make_index_sequence<std::tuple_size_v<BindTuple>>(), std::forward_as_tuple(std::forward<PassP>(var)...));
			}
		};
		class JBindSocket
		{
		private:
			std::unique_ptr<JBindHandleBase> handleBase;
		};
		template<typename Functor, typename ...BindParam>
		JBindHandle<Functor, BindParam...> Bind(Functor& functor, BindParam&&... bindVar)
		{
			return JBindHandle<Functor, BindParam...>(functor, std::forward<BindParam>(bindVar)...);
		}
#pragma endregion
	}
}