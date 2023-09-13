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
		* impl class�� OwnerPointer�� �ƴ϶� std::unique_ptr�� �Ҵ�ȴ�
		* �׷��Ƿ� Memory Relocation�� �������� ������ ������ �ʿ��ϰԵȴ�
		* �̰�� impl�� �ڽ��� raw �����͸� �����ϴ� ��ü�� �˰ų� Ȥ�� �׷� ��ü�� event�� �� �ʿ䰡�ִ�
		* impl�����͸� ���� �����ϴ� ��ü�� �ϳ��� �����ϸ� �����ϴ� ���� interface class�μ� �����ϰԵȴ�
		* �� interface class�� ����ڿ��� �������������� event�� ����Ҹ�ŭ �󵵰����������Ƿ� �������� �����ϸ�
		* ���� ����� ��ü�� ���� ������ �����͸� �����ϰų� PointerWrapper�� ���� �����Ѵ�.
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
			//wrapper�� default heap�� ����ϹǷ�
			//impl�� memory reallocate�� �Ͽ��� ����Ű�°��� �޶������ʴ´�.
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
		//Impl�� �ڱ��ڽ��� �����ϴ� Interface pointer�� �����ؾ��Ѵ�.
		//CallNotifyReAlloc�� �߰����� ó���� �ϴ� �Լ��� ȣ���Ҽ��ִ�.
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