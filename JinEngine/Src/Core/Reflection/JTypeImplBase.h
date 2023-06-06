#include<type_traits>

namespace JinEngine
{
	namespace Core
	{
		class JTypeImplBase
		{ 
		protected:
			JTypeImplBase() = default;
			virtual ~JTypeImplBase() = default;
		};

		template<typename T, typename = void>
		struct HasNotifyReAlloc : std::false_type
		{
		public:
			static void CallNotifyReAlloc(T* ptr) {}
		};
		template<typename T>
		struct HasNotifyReAlloc<T, std::void_t<decltype(&T::NotifyReAlloc)>> : std::true_type
		{
		public:
			static void CallNotifyReAlloc(T* ptr)
			{
				ptr->NotifyReAlloc();
			}
		};

#define IMPL_REALLOC_BIND(implTypeName, thisPointerName)										\
																								\
			using JAllocationDesc = JinEngine::Core::JAllocationDesc;							\
			using NotifyReAllocPtr = JAllocationDesc::NotifyReAllocF::Ptr;						\
			using NotifyReAllocF =JAllocationDesc::NotifyReAllocF::Functor;						\
			using ReceiverPtr = JAllocationDesc::ReceiverPtr;									\
			using ReAllocatedPtr = JAllocationDesc::ReAllocatedPtr;								\
			using MemIndex = JAllocationDesc::MemIndex;											\
																								\
			 NotifyReAllocPtr notifyPtr = [] (ReceiverPtr receiver, ReAllocatedPtr movedPtr, MemIndex index)\
			{																									\
				implTypeName* movedImpl = static_cast<implTypeName*>(movedPtr);									\
				auto& thisPtr = movedImpl->thisPointerName;														\
				thisPtr->impl.release();																		\
				thisPtr->impl.reset(movedImpl);																	\
				if constexpr(JinEngine::Core::HasNotifyReAlloc<implTypeName>::value)							\
					JinEngine::Core::HasNotifyReAlloc<implTypeName>::CallNotifyReAlloc(movedImpl);				\
			};																									\
			auto reAllocF = std::make_unique<NotifyReAllocF>(notifyPtr);										\
																												\
			std::unique_ptr<JAllocationDesc> desc = std::make_unique<JAllocationDesc>();						\
			desc->notifyReAllocB = UniqueBind(std::move(reAllocF), static_cast<ReceiverPtr>(&typeInfo), JinEngine::Core::empty,  JinEngine::Core::empty);\
			typeInfo.SetAllocationOption(std::move(desc));																\


	}
}

//Debug
/*
	template<typename T, typename = void>
	struct HasDebug : std::false_type
	{
	public:
		static void CallDebug(T* ptr) {}
	};
	template<typename T>
	struct HasDebug<T, std::void_t<decltype(&T::NotifyReAllocDebug)>> : std::true_type
	{
	public:
		static void CallDebug(T* ptr)
		{
			ptr->NotifyReAllocDebug();
		}
	};
																							\
		using PreAllocatedPtr= JAllocationDesc::PreAllocatedPtr;											\
		using NotifyReAllocDebugPtr = JAllocationDesc::NotifyReAllocDebugF::Ptr;							\
		using NotifyReAllocDebugF =JAllocationDesc::NotifyReAllocDebugF::Functor;							\
																											\
		NotifyReAllocDebugPtr debugPtr = [](ReceiverPtr receiver, PreAllocatedPtr prePtr, ReAllocatedPtr movedPtr, MemIndex index)	\
		{																									\
			implTypeName* preImpl = static_cast<implTypeName*>(prePtr);										\
			implTypeName* movedImpl = static_cast<implTypeName*>(movedPtr);									\
			if constexpr(JinEngine::Core::HasDebug<implTypeName>::value)									\
			{																								\
				JinEngine::Core::HasDebug<implTypeName>::CallDebug(preImpl);								\
				JinEngine::Core::HasDebug<implTypeName>::CallDebug(movedImpl);								\
			}																								\
		};																									\
		auto debugF = std::make_unique<NotifyReAllocDebugF>(debugPtr);										\
			desc->notifyDebugB = UniqueBind(std::move(debugF), static_cast<ReceiverPtr>(&typeInfo), JinEngine::Core::empty,  JinEngine::Core::empty, JinEngine::Core::empty);\
*/