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
#include "../JCoreEssential.h"    
#include<type_traits>  
#include<atomic>    

#define USE_FAST_OWNER 1

namespace JinEngine
{
	namespace Core
	{
		template<typename T, typename = void>
		struct HasTypeInfo : std::false_type {};
		template<typename T>
		struct HasTypeInfo<T, std::void_t<decltype(&T::StaticTypeInfo)>> : std::true_type {};

		template<typename T, typename = void>
		struct HasTypeList : std::false_type {};
		template<typename T>
		struct HasTypeList<T, std::void_t<typename T::ThisTypeList>> : std::true_type {};

		template<typename T, typename = void>
		struct HasCustomAlloc : std::false_type {};
		template<typename T>
		struct HasCustomAlloc<T, std::void_t<decltype(&T::operator delete)>> : std::true_type {};

		template<typename T>
		static constexpr bool CanConvertChildType() noexcept
		{
			return HasTypeInfo<T>::value || HasTypeList<T>::value;
		}
		class JTypeBase;
		class JTypeInfo;


#ifdef USE_FAST_OWNER
		class JPtrReferenceCount
		{
		public:
			std::atomic<uint> userCount = 0;
			std::atomic<ushort> weakCount = 0;
			bool isValid = false;
		}; 
		template<typename T> class JOwnerPtrInterface;
		template<typename T> class JUserPtrInterface;
		template<typename T> class JWeakPtrInterface;

		template<typename T>
		class JPtrBase
		{
		private:
			template<typename T> friend class JPtrBase;
			template<typename T> friend class JOwnerPtrInterface;
			template<typename T> friend class JUserPtrInterface;
			template<typename T> friend class JWeakPtrInterface;
		private:
			/*
			* JPtrData를 통한 두번의 Access는 std::unique_ptr보다 2배 더 느렸으며 데이터 유효성 검사가 필요하였고
			* void*로 인해 정확한 type을 알 수 없는 단점이 있었다.
			* std::shared_ptr를 참조해서 기존에 JPtrData에 있던 data를 JPtrBase로 옮기고 성능은 1.5배 빨라졌으며
			* std::shared_ptr와 비슷하다.
			*/
			T* ptr = nullptr;
			JPtrReferenceCount* referenceCount = nullptr;
			//JPtrData* ptrData = nullptr;
		public:
			T& operator*()
			{
				return *Get();
			}
			T* operator->() const noexcept
			{
				return referenceCount->isValid ? ptr : nullptr;
			}
		public:
			T* Get()const noexcept
			{
				/*
				* JPtrBase와 다르게 Owner가 ptr을 삭제한 경우
				* User는 잘못된 접근을 할 수 있다.
				* 그러므로 referenceCount의 isValid를 활용해서 검사할 필요가있다.
				* 이러한 조건문을 사용해도 2번 참조하는 JPtrBase보다 여전히 빠르다.
				*/
				return referenceCount->isValid ? ptr : nullptr;
			}
			uint GetUserCount()const noexcept
			{
				return referenceCount != nullptr ? referenceCount->userCount.load() : 0;
			}
			uint GetWeakCount()const noexcept
			{
				return referenceCount != nullptr ? referenceCount->weakCount.load() : 0;
			}
		protected:
			template<typename U>
			void SetValidPtrData(const JPtrBase<U>& otherPtrBase)
			{
				ptr = static_cast<U*>(otherPtrBase.ptr);
				referenceCount = otherPtrBase.referenceCount;
			}
			void SetInvalidPtrData()
			{
				ptr = nullptr;
				referenceCount = nullptr;
			}
		public:
			bool IsValid()const noexcept
			{
				//determine valid pointer
				return referenceCount != nullptr && referenceCount->isValid;
			}
		protected:
			bool HasValidPointer()const noexcept
			{
				return ptr != nullptr;
			}
			bool HasReferenceCount()const noexcept
			{
				return referenceCount != nullptr;
			}
		protected:
			void Initialize(T* newPtr)
			{
				ptr = newPtr;
				referenceCount = new JPtrReferenceCount();
				referenceCount->isValid = true;
			}
		protected:
			//owner or last user(owner deleted) and last weak can destroy ref
			void TryDestroyReferenceData()noexcept
			{
				if (referenceCount->userCount == 0 && referenceCount->weakCount == 0)
					delete referenceCount;
				referenceCount = nullptr;
				ptr = nullptr;
			}
		};

		template<typename T>
		class JOwnerPtrInterface : public JPtrBase<T>
		{
		private:
			friend class JTypeInfo;
		protected:
			using PtrBase = JPtrBase<T>;
		protected:
			/*
			* JTypeInfo와 Custom allocator를 사용하는 경우
			* Element갯수에 따라서 memory heap이 늘어나거나 줄어들며 새로운 공간에 메모리들이 재할당될 수 있으므로
			* 바뀐 메모리주소를 등록하기위한 함수이다.
			* JTypeBase에 파생클래스들은 JTypeInfo에서 이와같은 작업을 자동으로 관리해주나
			* 그렇지않은 클래스들은 JTypeInfo의 SetAllocationOption 함수를 통해서 재할당에 따른 동작을 추가할 수 있다.
			*/
			void SetValidPointer(T* ptr)
			{
				PtrBase::ptr = ptr;
				PtrBase::referenceCount->isValid = true;
			}
			void SetInValidPointer()
			{
				PtrBase::ptr = nullptr;
				PtrBase::referenceCount->isValid = false;
			}
		};
		template<typename T>
		class JUserPtrInterface : public JPtrBase<T>
		{
		protected:
			using PtrBase = JPtrBase<T>;
		protected:
			void AddUserCount()noexcept
			{
				++PtrBase::referenceCount->userCount;
			}
			void MinusUserCount()noexcept
			{
				--PtrBase::referenceCount->userCount;
			}
		};
		template<typename T>
		class JWeakPtrInterface : public JPtrBase<T>
		{
		protected:
			using PtrBase = JPtrBase<T>;
		protected:
			void AddWeakCount()noexcept
			{
				++PtrBase::referenceCount->weakCount;
			}
			void MinusWeakCount()noexcept
			{
				--PtrBase::referenceCount->weakCount;
			}
		};

		/*
		* Typelist처럼 하나의 template parameter type에 따라서 클래스 내부에서 Type이 달라지는것을 의존이름이라하며
		* 이 의존이름이 중첩되어 있을 경우 중첩 의존 이름이라고 한다.
		* 이와같은 경우 template class에서 Type을 사용하려 할때 Type이름이 Type인지 정적변수인지 모호할 수 있으므로
		* Type임을 명시하기위해 사용한다.
		* 원인: template class에서 Type에 중첩 의존적인 다른 template class에 내부 Type을 사용하려 할때.
		* 결론: 정적멤버인지 Type인지 도움을 주는 키워드이다.
		*/
		template<typename T>
		class JOwnerPtr final : public JOwnerPtrInterface<T>
		{
		public:
			using ElementType = T;
			using CustomDestructionPtr = void(*)(void*);
		private:
			template<typename T> friend class JOwnerPtr;
		private:
			using Owner = JOwnerPtrInterface<T>;
			using PtrBase = typename Owner::PtrBase;
		private:
			CustomDestructionPtr destructionPtr = nullptr;
		public:
			JOwnerPtr() = default;
			JOwnerPtr(nullptr_t)noexcept {}
			JOwnerPtr(const JOwnerPtr& rhs) = delete;
			JOwnerPtr& operator=(const JOwnerPtr& rhs) = delete;
			JOwnerPtr(T* newPtr)noexcept
			{
				OwnerConnect(newPtr);
			}
			JOwnerPtr(T* newPtr, CustomDestructionPtr newDestructionPtr)noexcept
			{
				OwnerConnect(newPtr);
				destructionPtr = newDestructionPtr;
			}
			JOwnerPtr(JOwnerPtr&& rhs)noexcept
			{
				OwnerMove(rhs);
			}
			JOwnerPtr& operator=(JOwnerPtr&& rhs)noexcept
			{
				OwnerMove(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JOwnerPtr(JOwnerPtr<NewType>&& rhs)noexcept
			{
				OwnerMove(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JOwnerPtr& operator=(JOwnerPtr<NewType>&& rhs)noexcept
			{
				OwnerMove(rhs);
				return *this;
			}
			JOwnerPtr& operator=(nullptr_t)noexcept
			{
				OwnerDisConnect();
				return *this;
			}
			~JOwnerPtr()noexcept
			{
				Clear();
			}
		public:
			T* operator->() const noexcept
			{
				return PtrBase::Get();
			}
			bool operator==(nullptr_t) const noexcept
			{
				return !PtrBase::IsValid();
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return PtrBase::IsValid();
			}
		public:
			void SetDestructionPtr(CustomDestructionPtr newDestructionPtr)
			{
				destructionPtr = newDestructionPtr;
			}
		public:
			void Reset(T* newPtr)noexcept
			{
				if (newPtr == nullptr)
					Clear();
				else if (PtrBase::HasValidPointer())
					ExchangePointer(newPtr);
				else
					OwnerConnect(newPtr);
			}
			T* Release()noexcept
			{
				T* ptr = PtrBase::Get();
				if (PtrBase::HasValidPointer())
					Owner::SetInValidPointer();
				return ptr;
			}
			void Clear()noexcept
			{
				OwnerDisConnect();
			}
		public:
			//For JTypeBase
			template<typename ChildType>
			static JOwnerPtr ConvertChild(JOwnerPtr<ChildType>&& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JOwnerPtr newOwner;
						newOwner.OwnerMove(child);
						return newOwner;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JOwnerPtr newOwner;
						newOwner.OwnerMove(child);
						return newOwner;
					}
				}
				return nullptr;
			}
		private:
			template<typename NewType>
			void OwnerMove(JOwnerPtr<NewType>& rhs)noexcept
			{
				Clear();
				PtrBase::SetValidPtrData(rhs);
				destructionPtr = rhs.destructionPtr;
				rhs.SetInvalidPtrData();
			}
			void OwnerConnect(T* ptr)noexcept
			{
				PtrBase::Initialize(ptr);
			}
			void OwnerDisConnect()noexcept
			{
				if (!PtrBase::HasValidPointer())
					return;

				ExchangePointer(nullptr);
			}
			void ExchangePointer(T* newPtr = nullptr)noexcept
			{
				if (PtrBase::IsValid())
				{
					T* ptr = PtrBase::Get();
					if (ptr != nullptr)
					{
						if (destructionPtr != nullptr)
							destructionPtr(ptr);
						else
							delete ptr;
					}
				}

				if (newPtr != nullptr)
					Owner::SetValidPointer(newPtr);
				else
				{
					Owner::SetInValidPointer();
					PtrBase::TryDestroyReferenceData();
				}
			}
		};

		template<typename T>
		class JUserPtr final : public JUserPtrInterface<T>
		{
		public:
			using ElementType = T;
		private:
			template<typename T> friend class JUserPtr;
		private:
			using User = JUserPtrInterface<T>;
			using PtrBase = typename User::PtrBase;
		public:
			JUserPtr() = default;
			JUserPtr(nullptr_t) {}
			JUserPtr(const JUserPtr& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			JUserPtr(JUserPtr&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(const JPtrBase<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(const JPtrBase<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(const JUserPtr<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(JUserPtr<NewType>&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			JUserPtr& operator=(const JUserPtr& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			JUserPtr& operator=(JUserPtr&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(const JUserPtr<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(JUserPtr<NewType>&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
				return *this;
			}
			JUserPtr& operator=(nullptr_t)noexcept
			{
				UserDisConnect();
				return *this;
			}
			~JUserPtr()noexcept
			{
				Clear();
			}
		public:
			template<typename OtherType>
			bool operator==(JUserPtr<OtherType> rhs) const noexcept
			{
				return PtrBase::Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(JUserPtr<OtherType> rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs.Get());
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs);
			}
			bool operator==(nullptr_t) const noexcept
			{
				return !PtrBase::IsValid();
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return PtrBase::IsValid();
			}
			T* operator->() const noexcept
			{
				return PtrBase::Get();
			}
		public:
			void Clear()
			{
				UserDisConnect();
			}
			void Reset(PtrBase& rhs)
			{
				Clear();
				UserConnect(rhs);
			}
			T* Release()
			{
				T* res = PtrBase::Get();
				UserDisConnect();
				return res;
			}
		public:
			//For JTypeBase
			template<typename ChildType>
			static JUserPtr<T> CreateChild(JPtrBase<ChildType>& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						return newUser;
					}
				}
				return nullptr;
			}
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ChildType>
			static JUserPtr<T> ConvertChild(JUserPtr<ChildType>&& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						child.Clear();
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						child.Clear();
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ChildType>
			bool ConnnectChild(const JUserPtr<ChildType>& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return false;

				if (!child.IsValid())
					return false;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						UserDisConnect();
						UserConnect(child);
						return true;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						UserDisConnect();
						UserConnect(child);
						return true;
					}
				}
				return false;
			}
		private:
			template<typename NewType>
			void UserConnect(const JPtrBase<NewType>& ptrBase)noexcept
			{
				PtrBase::SetValidPtrData(ptrBase);
				if (PtrBase::HasReferenceCount())
				{
					if constexpr (HasTypeInfo<T>::value && std::is_convertible_v<T*, JTypeBase*>)
					{
						T* ptr = PtrBase::Get();
						if (ptr != nullptr && PtrBase::GetUserCount() == 0)
							ptr->GetTypeInfo().TryCancelLazyDestruction(ptr);
					}
					User::AddUserCount();
				}
			}
			void UserDisConnect()noexcept
			{
				if (PtrBase::HasReferenceCount())
				{
					if constexpr (HasTypeInfo<T>::value && std::is_convertible_v<T*, JTypeBase*>)
					{
						T* ptr = PtrBase::Get();
						if (ptr != nullptr && PtrBase::GetUserCount() == 1)
							ptr->GetTypeInfo().TryLazyDestruction(ptr);
					}
					User::MinusUserCount();
					PtrBase::TryDestroyReferenceData();
				}
			}
		};

		template<typename T>
		class JWeakPtr final : public JWeakPtrInterface<T>
		{
		public:
			using ElementType = T;
		private:
			template<typename T> friend class JWeakPtr;
		private:
			using Weak = JWeakPtrInterface<T>;
			using PtrBase = typename Weak::PtrBase;
		public:
			JWeakPtr() = default;
			JWeakPtr(nullptr_t) {}
			JWeakPtr(const JWeakPtr& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			JWeakPtr(JWeakPtr&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(const JPtrBase<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(const JPtrBase<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(const JWeakPtr<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(JWeakPtr<NewType>&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			JWeakPtr& operator=(const JWeakPtr& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			JWeakPtr& operator=(JWeakPtr&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(const JWeakPtr<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(JWeakPtr<NewType>&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
				return *this;
			}
			JWeakPtr& operator=(nullptr_t)noexcept
			{
				WeakDisConnect();
				return *this;
			}
			~JWeakPtr()noexcept
			{
				Clear();
			}
		public:
			template<typename OtherType>
			bool operator==(JWeakPtr<OtherType> rhs) const noexcept
			{
				return PtrBase::Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(JWeakPtr<OtherType> rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs.Get());
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs);
			}
			bool operator==(nullptr_t) const noexcept
			{
				return !PtrBase::IsValid();
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return PtrBase::IsValid();
			}
			T* operator->() const noexcept
			{
				return PtrBase::Get();
			}
		public:
			void Clear()
			{
				WeakDisConnect();
			}
			void Reset(PtrBase& rhs)
			{
				Clear();
				WeakConnect(rhs);
			}
			T* Release()
			{
				T* res = PtrBase::Get();
				WeakDisConnect();
				return res;
			}
		public:
			//For JTypeBase
			template<typename ChildType>
			static JWeakPtr<T> CreateChild(JPtrBase<ChildType>& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						return newUser;
					}
				}
				return nullptr;
			}
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ChildType>
			static JWeakPtr<T> ConvertChild(JWeakPtr<ChildType>&& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						child.Clear();
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						child.Clear();
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ChildType>
			bool ConnnectChild(const JWeakPtr<ChildType>& child)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
					return false;

				if (!child.IsValid())
					return false;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						WeakDisConnect();
						WeakConnect(child);
						return true;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						WeakDisConnect();
						WeakConnect(child);
						return true;
					}
				}
				return false;
			}
		private:
			template<typename NewType>
			void WeakConnect(const JPtrBase<NewType>& ptrBase)noexcept
			{
				PtrBase::SetValidPtrData(ptrBase);
				if (PtrBase::HasReferenceCount())
					Weak::AddWeakCount();
			}
			void WeakDisConnect()noexcept
			{
				if (PtrBase::HasReferenceCount())
				{
					Weak::MinusWeakCount();
					PtrBase::TryDestroyReferenceData();
				}
			}
		};
#else
		template<typename T>
		class JPtrData
		{
		public:
			T* ptr = nullptr;
			std::atomic<uint> userCount = 0;
			std::atomic<uint> weakCount = 0;
		};

		template<typename T> class JOwnerPtrInterface;
		template<typename T> class JUserPtrInterface;
		template<typename T> class JWeakPtrInterface;

		template<typename T>
		class JPtrBase
		{
		private:
			template<typename T> friend class JPtrBase;
			template<typename T> friend class JOwnerPtrInterface;
			template<typename T> friend class JUserPtrInterface;
			template<typename T> friend class JWeakPtrInterface;
		private:
			JPtrData<T>* ptrData = nullptr;
		public:
			T& operator*()
			{
				return *ptrData->ptr;
			}
			T* operator->() const noexcept
			{
				return ptrData->ptr;
				//return ptrData != nullptr ? ptrData->ptr : nullptr;
			}
		public:
			T* Get()const noexcept
			{
				/*
				* Owner가 파괴되어도 User와 Weak가 있다면 ptrData은 남아있으며 ptr은 nullptr을 가리킨다.
				* 잘못된 접근은
				* 1. 처음부터 할당되지 않은 경우.
				* 2. Release()후에 접근하는 경우
				* 이며 두가지 모두 사용자의 부주의가 원인이므로
				* 성능을 위해 이전의 사용하던
				* return ptrData != nullptr ? ptrData->ptr : nullptr;을 아래와 같이 변경한다.
				*/
				return ptrData->ptr;
			}
			T* UnsafeGet()noexcept
			{
				return ptrData->ptr;
			}
			uint GetUserCount()const noexcept
			{
				return ptrData != nullptr ? ptrData->userCount.load() : 0;
			}
			uint GetWeakCount()const noexcept
			{
				return ptrData != nullptr ? ptrData->weakCount.load() : 0;
			}
		protected:
			template<typename U>
			void SetValidPtrData(const JPtrBase<U>& otherPtrBase)
			{
				ptrData = new JPtrData<T>();
				ptrData->ptr = static_cast<T*>(otherPtrBase.ptrData->ptr);
				ptrData->userCount = otherPtrBase.ptrData->userCount.load();
				ptrData->weakCount = otherPtrBase.ptrData->weakCount.load();
			}
			void SetValidPtrData(JPtrData<T>* newPtrData)
			{
				ptrData = newPtrData;
			}
			void SetInValidPtrData()
			{
				ptrData = nullptr;
			}
		public:
			bool IsValid()const noexcept
			{
				return ptrData != nullptr && ptrData->ptr != nullptr;
			}
		protected:
			bool HasPtrData()const noexcept
			{
				return ptrData != nullptr;
			}
		protected:
			//owner or last user(owner deleted) and last weak can destroy ref
			void TryDestroyPtrData()noexcept
			{
				if (ptrData == nullptr)
					return;

				if (ptrData->ptr == nullptr && ptrData->userCount == 0 && ptrData->weakCount == 0)
					delete ptrData;
				ptrData = nullptr;
			}
		};

		template<typename T>
		class JOwnerPtrInterface : public JPtrBase<T>
		{
		private:
			friend class JTypeInfo;
		protected:
			using PtrBase = JPtrBase<T>;
		protected:
			void SetValidPointer(T* ptr)
			{
				PtrBase::ptrData->ptr = ptr;
			}
			void SetInValidPointer()
			{
				PtrBase::ptrData->ptr = nullptr;
			}
		};
		template<typename T>
		class JUserPtrInterface : public JPtrBase<T>
		{
		protected:
			using PtrBase = JPtrBase<T>;
		protected:
			void AddUserCount()noexcept
			{
				++PtrBase::ptrData->userCount;
			}
			void MinusUserCount()noexcept
			{
				--PtrBase::ptrData->userCount;
			}
		};
		template<typename T>
		class JWeakPtrInterface : public JPtrBase<T>
		{
		protected:
			using PtrBase = JPtrBase<T>;
		protected:
			void AddWeakCount()noexcept
			{
				++PtrBase::ptrData->weakCount;
			}
			void MinusWeakCount()noexcept
			{
				--PtrBase::ptrData->weakCount;
			}
		};

		template<typename T>
		class JOwnerPtr final : public JOwnerPtrInterface<T>
		{
		public:
			using ElementType = T;
			using CustomDestructionPtr = void(*)(void*);
		private:
			template<typename T> friend class JOwnerPtr;
		private:
			using Owner = JOwnerPtrInterface<T>;
			using PtrBase = typename Owner::PtrBase;
		private:
			CustomDestructionPtr destructionPtr = nullptr;
		public:
			JOwnerPtr() = default;
			JOwnerPtr(nullptr_t)noexcept {}
			JOwnerPtr(const JOwnerPtr& rhs) = delete;
			JOwnerPtr& operator=(const JOwnerPtr& rhs) = delete;
			JOwnerPtr(T* newPtr)noexcept
			{
				OwnerConnect(newPtr);
			}
			JOwnerPtr(T* newPtr, CustomDestructionPtr newDestructionPtr)noexcept
			{
				OwnerConnect(newPtr);
				destructionPtr = newDestructionPtr;
			}
			JOwnerPtr(JOwnerPtr&& rhs)noexcept
			{
				OwnerMove(rhs);
			}
			JOwnerPtr& operator=(JOwnerPtr&& rhs)noexcept
			{
				OwnerMove(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JOwnerPtr(JOwnerPtr<NewType>&& rhs)noexcept
			{
				OwnerMove(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JOwnerPtr& operator=(JOwnerPtr<NewType>&& rhs)noexcept
			{
				OwnerMove(rhs);
				return *this;
			}
			JOwnerPtr& operator=(nullptr_t)noexcept
			{
				OwnerDisConnect();
				return *this;
			}
			~JOwnerPtr()noexcept
			{
				Clear();
			}
		public:
			T* operator->() const noexcept
			{
				return PtrBase::Get();
			}
			bool operator==(nullptr_t) const noexcept
			{
				return !PtrBase::IsValid();
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return PtrBase::IsValid();
			}
		public:
			void SetDestructionPtr(CustomDestructionPtr newDestructionPtr)
			{
				destructionPtr = newDestructionPtr;
			}
		public:
			void Reset(T* newPtr)noexcept
			{
				if (newPtr == nullptr)
					Clear();
				else if (PtrBase::HasPtrData())
					ExchangePointer(newPtr);
				else
					OwnerConnect(newPtr);
			}
			/*
			template<typename ...Param>
			void Reset(Param&&... var)
			{
				T* newPtr = new T(std::forward<Param>(var)...);
				Reset(newPtr);
			}
			*/
			T* Release()noexcept
			{
				T* ptr = PtrBase::Get();
				if (PtrBase::HasPtrData())
					Owner::SetInValidPointer();
				return ptr;
			}
			void Clear()noexcept
			{
				OwnerDisConnect();
			}
		public:
			//For JTypeBase
			template<typename ChildType>
			static JOwnerPtr ConvertChild(JOwnerPtr<ChildType>&& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JOwnerPtr newOwner;
						newOwner.OwnerMove(child);
						return newOwner;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JOwnerPtr newOwner;
						newOwner.OwnerMove(child);
						return newOwner;
					}
				}
				return nullptr;
			}
		private:
			template<typename NewType>
			void OwnerMove(JOwnerPtr<NewType>& rhs)noexcept
			{
				Clear();
				PtrBase::SetValidPtrData(rhs);
				destructionPtr = rhs.destructionPtr;
				rhs.SetInValidPtrData();
			}
			void OwnerConnect(T* ptr)noexcept
			{
				JPtrData<T>* newData = new JPtrData<T>();
				newData->ptr = ptr;

				PtrBase::SetValidPtrData(newData);
				Owner::SetValidPointer(ptr);
			}
			void OwnerDisConnect()noexcept
			{
				if (!PtrBase::HasPtrData())
					return;

				ExchangePointer(nullptr);
			}
			void ExchangePointer(T* newPtr = nullptr)noexcept
			{
				if (PtrBase::IsValid())
				{
					T* ptr = PtrBase::UnsafeGet();
					if (ptr != nullptr)
					{
						if (destructionPtr != nullptr)
							destructionPtr(ptr);
						else
							delete ptr;
					}
				}

				if (newPtr != nullptr)
					Owner::SetValidPointer(newPtr);
				else
				{
					Owner::SetInValidPointer();
					PtrBase::TryDestroyPtrData();
				}
			}
		};

		template<typename T>
		class JUserPtr final : public JUserPtrInterface<T>
		{
		public:
			using ElementType = T;
		private:
			template<typename T> friend class JUserPtr;
		private:
			using User = JUserPtrInterface<T>;
			using PtrBase = typename User::PtrBase;
		public:
			JUserPtr() = default;
			JUserPtr(nullptr_t) {}
			JUserPtr(const JUserPtr& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			JUserPtr(JUserPtr&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(const JPtrBase<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(const JPtrBase<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(const JUserPtr<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(JUserPtr<NewType>&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			JUserPtr& operator=(const JUserPtr& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			JUserPtr& operator=(JUserPtr&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(const JUserPtr<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(JUserPtr<NewType>&& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
				return *this;
			}
			JUserPtr& operator=(nullptr_t)noexcept
			{
				UserDisConnect();
				return *this;
			}
			~JUserPtr()noexcept
			{
				Clear();
			}
		public:
			template<typename OtherType>
			bool operator==(JUserPtr<OtherType> rhs) const noexcept
			{
				return PtrBase::Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(JUserPtr<OtherType> rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs.Get());
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs);
			}
			bool operator==(nullptr_t) const noexcept
			{
				return !PtrBase::IsValid();
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return PtrBase::IsValid();
			}
			T* operator->() const noexcept
			{
				return PtrBase::Get();
			}
		public:
			void Clear()
			{
				UserDisConnect();
			}
			void Reset(PtrBase& rhs)
			{
				Clear();
				UserConnect(rhs);
			}
			T* Release()
			{
				T* res = PtrBase::Get();
				UserDisConnect();
				return res;
			}
		public:
			//For JTypeBase
			template<typename ChildType>
			static JUserPtr<T> CreateChild(JPtrBase<ChildType>& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						return newUser;
					}
				}
				return nullptr;
			}
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ChildType>
			static JUserPtr<T> ConvertChild(JUserPtr<ChildType>&& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						child.Clear();
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(child);
						child.Clear();
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ChildType>
			bool ConnnectChild(const JUserPtr<ChildType>& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return false;

				if (!child.IsValid())
					return false;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						UserDisConnect();
						UserConnect(child);
						return true;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						UserDisConnect();
						UserConnect(child);
						return true;
					}
				}
				return false;
			}
		private:
			template<typename NewType>
			void UserConnect(const JPtrBase<NewType>& ptrBase)noexcept
			{
				PtrBase::SetValidPtrData(ptrBase);
				if (PtrBase::HasPtrData())
				{
					if constexpr (HasTypeInfo<T>::value && std::is_convertible_v<T*, JTypeBase*>)
					{
						T* ptr = PtrBase::UnsafeGet();
						if (ptr != nullptr && PtrBase::GetUserCount() == 0)
							ptr->GetTypeInfo().TryCancelLazyDestruction(ptr);
					}
					User::AddUserCount();
				}
			}
			void UserDisConnect()noexcept
			{
				if (PtrBase::HasPtrData())
				{
					if constexpr (HasTypeInfo<T>::value && std::is_convertible_v<T*, JTypeBase*>)
					{
						T* ptr = PtrBase::UnsafeGet();
						if (ptr != nullptr && PtrBase::GetUserCount() == 1)
							ptr->GetTypeInfo().TryLazyDestruction(ptr);
					}
					User::MinusUserCount();
					PtrBase::TryDestroyPtrData();
				}
			}
		};

		template<typename T>
		class JWeakPtr final : public JWeakPtrInterface<T>
		{
		public:
			using ElementType = T;
		private:
			template<typename T> friend class JWeakPtr;
		private:
			using Weak = JWeakPtrInterface<T>;
			using PtrBase = typename Weak::PtrBase;
		public:
			JWeakPtr() = default;
			JWeakPtr(nullptr_t) {}
			JWeakPtr(const JWeakPtr& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			JWeakPtr(JWeakPtr&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(const JPtrBase<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(const JPtrBase<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(const JWeakPtr<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(JWeakPtr<NewType>&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			JWeakPtr& operator=(const JWeakPtr& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			JWeakPtr& operator=(JWeakPtr&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(const JWeakPtr<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(JWeakPtr<NewType>&& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
				return *this;
			}
			JWeakPtr& operator=(nullptr_t)noexcept
			{
				WeakDisConnect();
				return *this;
			}
			~JWeakPtr()noexcept
			{
				Clear();
			}
		public:
			template<typename OtherType>
			bool operator==(JWeakPtr<OtherType> rhs) const noexcept
			{
				return PtrBase::Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(JWeakPtr<OtherType> rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs.Get());
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return !(PtrBase::Get() == rhs);
			}
			bool operator==(nullptr_t) const noexcept
			{
				return !PtrBase::IsValid();
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return PtrBase::IsValid();
			}
			T* operator->() const noexcept
			{
				return PtrBase::Get();
			}
		public:
			void Clear()
			{
				WeakDisConnect();
			}
			void Reset(PtrBase& rhs)
			{
				Clear();
				WeakConnect(rhs);
			}
			T* Release()
			{
				T* res = PtrBase::Get();
				WeakDisConnect();
				return res;
			}
		public:
			//For JTypeBase
			template<typename ChildType>
			static JWeakPtr<T> CreateChild(JPtrBase<ChildType>& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						return newUser;
					}
				}
				return nullptr;
			}
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ChildType>
			static JWeakPtr<T> ConvertChild(JWeakPtr<ChildType>&& child)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!child.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						child.Clear();
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(child);
						child.Clear();
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ChildType>
			bool ConnnectChild(const JWeakPtr<ChildType>& child)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
					return false;

				if (!child.IsValid())
					return false;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						WeakDisConnect();
						WeakConnect(child);
						return true;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (child->IsChildOf(T::TypeGuid()))
					{
						WeakDisConnect();
						WeakConnect(child);
						return true;
					}
				}
				return false;
			}
		private:
			template<typename NewType>
			void WeakConnect(const JPtrBase<NewType>& ptrBase)noexcept
			{
				PtrBase::SetValidPtrData(ptrBase);
				if (PtrBase::HasPtrData())
					Weak::AddWeakCount();
			}
			void WeakDisConnect()noexcept
			{
				if (PtrBase::HasPtrData())
				{
					Weak::MinusWeakCount();
					PtrBase::TryDestroyPtrData();
				}
			}
		};
#endif
		template<typename  T>
		static JUserPtr<T> ConnectChildUserPtr(const JUserPtr<JTypeBase>& user)
		{
			JUserPtr<T> res;
			res.ConnnectChild(user);
			return res;
		}
		template<typename  T>
		static JUserPtr<T> ConvertChildUserPtr(JUserPtr<JTypeBase>&& user)
		{
			return JUserPtr<T>::ConvertChild(std::move(user));
		}
		template<typename  T>
		static JWeakPtr<T> ConnectChildWeakPtr(JWeakPtr<JTypeBase>&& user)
		{
			JWeakPtr<T> res;
			res.ConnnectChild(user);
			return res;
		}
		template<typename  T>
		static JWeakPtr<T> ConvertChildWeakPtr(JWeakPtr<JTypeBase>&& user)
		{
			return JWeakPtr<T>::ConvertChild(std::move(user));
		}
	}

	namespace Core
	{
		class JPtrUtil
		{
		public:
			template<typename T, typename ...Param>
			static JOwnerPtr<T> MakeOwnerPtr(Param&&... var)
			{
				return JOwnerPtr<T>(new T(std::forward<Param>(var)...));
			}
		};
	}

	template<typename T>
	using JOwnerPtr = Core::JOwnerPtr<T>;
	template<typename T>
	using JUserPtr = Core::JUserPtr<T>;
	template<typename T>
	using JWeakPtr = Core::JWeakPtr<T>;
}
