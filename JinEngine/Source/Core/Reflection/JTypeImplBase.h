#pragma once
#include<type_traits>
#include<memory>
#include"../Pointer/JOwnerPtr.h" 

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

		/**
		* impl class는 OwnerPointer가 아니라 std::unique_ptr로 할당된다
		* 그러므로 Memory Relocation때 수동으로 포인터 갱신이 필요하게된다
		* 이경우 impl이 자신의 raw 포인터를 소유하는 객체를 알거나 혹은 그런 객체에 event를 할 필요가있다
		* impl포인터를 직접 소유하는 객체는 하나만 존재하며 참조하는 경우는 interface class로서 참조하게된다
		* 이 interface class는 사용자에게 노출하지않으며 event를 사용할만큼 빈도가많지않으므로 수동으로 관리하며
		* 참조 사용자 객체에 직접 접근해 포인터를 수정하거나 PointerWrapper를 통해 관리한다.
		*/

		template<typename InterfaceType> class JTypeImplInterfacePointerHolder;

		template<typename InterfaceType>
		class JTypeImplInterfacePointer
		{
		private:
			template<typename InterfaceType> friend class JTypeImplInterfacePointerHolder;
		private:
			InterfaceType* ptr = nullptr;
		public:
			InterfaceType& operator*()const noexcept
			{
				return *ptr;
			}
			InterfaceType* operator->()const noexcept
			{
				return ptr;
			}
		public:
			InterfaceType* Get()const noexcept
			{
				return ptr;
			}
		public:
			bool IsValid()const noexcept
			{
				return ptr != nullptr;
			}
		};
		 
		template<typename InterfaceType>
		class JTypeImplInterfacePointerHolder
		{
		private:
			//wrapper는 default heap을 사용하므로
			//impl이 memory reallocate을 하여도 가리키는값은 달라지지않는다.
			JOwnerPtr<JTypeImplInterfacePointer<InterfaceType>> pointerWrapper;
		public:
			virtual ~JTypeImplInterfacePointerHolder()
			{ 
				pointerWrapper = nullptr;
			}
		public:
			JOwnerPtr<JTypeImplInterfacePointer<InterfaceType>>& GetPointerWrapper()
			{
				return pointerWrapper;
			}
		protected:
			void SetInterfacePointer(InterfaceType* ptr)
			{
				if (pointerWrapper == nullptr)
					pointerWrapper = JPtrUtil::MakeOwnerPtr<JTypeImplInterfacePointer<InterfaceType>>();
				pointerWrapper->ptr = ptr;
			}
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

		//Caution
		//Impl은 자기자신을 소유하는 Interface pointer를 소유해야한다.
		//CallNotifyReAlloc로 추가적인 처리를 하는 함수를 호출할수있다.
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