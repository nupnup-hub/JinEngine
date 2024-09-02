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
 
namespace JinEngine
{
	namespace Core
	{
		namespace
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
		}

		class JTypeBase;
		class JTypeInfo;

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
		//template<typename T> class JFastPtrInterface;

		template<typename T>
		class JOwnerPtrBase
		{
		private:
			template<typename T> friend class JOwnerPtrBase;
			template<typename T> friend class JOwnerPtrInterface;
			template<typename T> friend class JUserPtrInterface;
			template<typename T> friend class JWeakPtrInterface;
			//template<typename T> friend class JFastPtrInterface;
		private:
			JPtrData<T>* ptrData = nullptr;
		public:
			T& operator*() noexcept
			{
				return *Get();
			}
			const T& operator*() const noexcept
			{
				return *Get();
			}
			T* operator->() const noexcept
			{
				return (ptrData != nullptr && ptrData->ptr != nullptr) ? ptrData->ptr : nullptr;
			}
		public:
			T* Get()const noexcept
			{
				return (ptrData != nullptr && ptrData->ptr != nullptr) ? ptrData->ptr : nullptr;
			}
			T* UnsafeGet()const noexcept
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
			/**
			* @brief 호출하는 클래스에서 타입에 대한 유효성검사를 해야한다.
			*/
			template<typename U>
			void SetValidPtrData(const JOwnerPtrBase<U>& otherPtrBase)
			{
				ptrData = static_cast<JPtrData<T>*>(static_cast<void*>(otherPtrBase.ptrData));
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
		class JOwnerPtrInterface : public JOwnerPtrBase<T>
		{
		private:
			friend class JTypeInfo;
		protected:
			using PtrBase = JOwnerPtrBase<T>;
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
		class JUserPtrInterface : public JOwnerPtrBase<T>
		{
		protected:
			using PtrBase = JOwnerPtrBase<T>;
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
		class JWeakPtrInterface : public JOwnerPtrBase<T>
		{
		protected:
			using PtrBase = JOwnerPtrBase<T>;
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
				if(newPtr != nullptr)
					OwnerConnect(newPtr);
			}
			JOwnerPtr(T* newPtr, CustomDestructionPtr newDestructionPtr)noexcept
			{
				if (newPtr != nullptr)
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
			T* Release()noexcept
			{
				T* ptr = PtrBase::Get();
				if (PtrBase::HasPtrData())
					Owner::SetInValidPointer();
				return ptr;
			}
			T* Swap(T* newPtr)
			{ 
				T* exist = PtrBase::Get();
				if (newPtr != nullptr)
					Owner::SetValidPointer(newPtr); 
				return exist;
			}
			void Clear()noexcept
			{
				OwnerDisConnect();
			}
		public:
			//For JTypeBase
			template<typename ParentType>
			static JOwnerPtr ConvertChild(JOwnerPtr<ParentType>&& owner)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!owner.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (owner->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JOwnerPtr newOwner;
						newOwner.OwnerMove(owner);
						return newOwner;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (owner->IsChildOf(T::TypeGuid()))
					{
						JOwnerPtr newOwner;
						newOwner.OwnerMove(owner);
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
				PtrBase::SetValidPtrData(new JPtrData<T>());
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
			JUserPtr(const JOwnerPtrBase<NewType>& rhs)noexcept
			{
				UserDisConnect();
				UserConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(const JOwnerPtrBase<NewType>& rhs)noexcept
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
				return PtrBase::Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return (PtrBase::Get() != rhs);
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
			template<typename ParentType>
			static JUserPtr<T> CreateChild(JOwnerPtrBase<ParentType>& user)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!user.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (user->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(user);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (user->IsChildOf(T::TypeGuid()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(user);
						return newUser;
					}
				}
				return nullptr;
			}
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ParentType>
			static JUserPtr<T> ConvertChild(JUserPtr<ParentType>&& user)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!user.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (user->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(user);
						user.Clear();
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (user->IsChildOf(T::TypeGuid()))
					{
						JUserPtr<T> newUser;
						newUser.UserConnect(user);
						user.Clear();
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ParentType>
			bool ConnnectChild(const JUserPtr<ParentType>& user)
			{
				if constexpr (!CanConvertChildType<T>())
					return false;

				if (!user.IsValid())
					return false;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (user->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						UserDisConnect();
						UserConnect(user);
						return true;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (user->IsChildOf(T::TypeGuid()))
					{
						UserDisConnect();
						UserConnect(user);
						return true;
					}
				}
				return false;
			}
		private:
			template<typename NewType>
			void UserConnect(const JOwnerPtrBase<NewType>& ptrBase)noexcept
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
			JWeakPtr(const JOwnerPtrBase<NewType>& rhs)noexcept
			{
				WeakDisConnect();
				WeakConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(const JOwnerPtrBase<NewType>& rhs)noexcept
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
				return PtrBase::Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return !PtrBase::Get() != rhs;
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
			template<typename ParentType>
			static JWeakPtr<T> CreateChild(JOwnerPtrBase<ParentType>& weak)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!weak.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (weak->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(weak);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (weak->IsChildOf(T::TypeGuid()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(weak);
						return newUser;
					}
				}
				return nullptr;
			}
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ParentType>
			static JWeakPtr<T> ConvertChild(JWeakPtr<ParentType>&& weak)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!weak.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (weak->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(weak);
						weak.Clear();
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (weak->IsChildOf(T::TypeGuid()))
					{
						JWeakPtr<T> newUser;
						newUser.WeakConnect(weak);
						weak.Clear();
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ParentType>
			bool ConnnectChild(const JWeakPtr<ParentType>& weak)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ParentType> || !std::is_base_of_v<JTypeBase, T>)
					return false;

				if (!weak.IsValid())
					return false;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (weak->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						WeakDisConnect();
						WeakConnect(weak);
						return true;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (weak->IsChildOf(T::TypeGuid()))
					{
						WeakDisConnect();
						WeakConnect(weak);
						return true;
					}
				}
				return false;
			}
		private:
			template<typename NewType>
			void WeakConnect(const JOwnerPtrBase<NewType>& ptrBase)noexcept
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
		 
		template<typename T>
		class JUniquePtrBase
		{ 
		private:
			T* ptr = nullptr;  
		public:
			T& operator*()
			{
				return *ptr;
			}
			const T& operator*()const noexcept
			{
				return *ptr;
			}
			T* operator->() const noexcept
			{
				return ptr;
			}
		public:
			T* Get()const noexcept
			{ 
				return ptr;
			}
			T** GetAddress()noexcept
			{
				return &ptr;
			} 
		protected:
			template<typename U>
			void SetValidPointer(JUniquePtrBase<U>& otherPtrBase)
			{
				ptr = static_cast<T*>(otherPtrBase.Get());  
			} 
			void SetValidPointer(T* newPtr)
			{
				ptr = newPtr;
			}
			void SetInvalidPointer()
			{
				ptr = nullptr; 
			}
		public:
			bool IsValid()const noexcept
			{ 
				return ptr != nullptr;
			} 
		protected: 
			void DestroyPointer()
			{
				delete ptr;
				ptr = nullptr;
			}
		};

		template<typename T>
		class JUniquePtr final : public JUniquePtrBase<T>
		{
		public:
			using ElementType = T; 
		private:
			template<typename T> friend class JUniquePtr;
		private:
			using PtrBase = typename JUniquePtrBase<T>; 
		public:
			JUniquePtr() = default;
			JUniquePtr(nullptr_t)noexcept {}
			JUniquePtr(const JUniquePtr & rhs) = delete;
			JUniquePtr& operator=(const JUniquePtr & rhs) = delete;
			JUniquePtr(T * newPtr)noexcept
			{
				UniqueConnect(newPtr);
			} 
			JUniquePtr(JUniquePtr&& rhs)noexcept
			{
				UniqueMove(std::move(rhs));
			}
			JUniquePtr& operator=(JUniquePtr&& rhs)noexcept
			{
				UniqueMove(std::move(rhs));
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUniquePtr(JUniquePtr<NewType>&& rhs)noexcept
			{
				UniqueMove(std::move(rhs));
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUniquePtr& operator=(JUniquePtr<NewType>&& rhs)noexcept
			{
				UniqueMove(std::move(rhs));
				return *this;
			}
			JUniquePtr& operator=(nullptr_t)noexcept
			{
				UniqueDisConnect();
				return *this;
			}
			~JUniquePtr()noexcept
			{
				Clear();
			}
		public: 
			bool operator==(nullptr_t) const noexcept
			{
				return !PtrBase::IsValid();
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return PtrBase::IsValid();
			} 
		public:
			void Reset(T * newPtr)noexcept
			{
				if (newPtr == nullptr)
					Clear();
				else if (PtrBase::IsValid())
					ExchangePointer(newPtr);
				else
					UniqueConnect(newPtr);
			}
			T* Release()noexcept
			{
				T* ptr = PtrBase::Get();
				if (PtrBase::IsValid())
					PtrBase::SetInvalidPointer();
				return ptr;
			}
			T* Swap(T * newPtr)
			{
				T* exist = PtrBase::Get();
				if (newPtr != nullptr)
					PtrBase::SetValidPointer(newPtr);
				return exist;
			}
			void Clear()noexcept
			{
				UniqueDisConnect();
			}
		public: 
			template<typename ParentType>
			static JUniquePtr ConvertChild(JUniquePtr<ParentType>&& unique)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!unique.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (unique->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JUniquePtr newUnique;
						newUnique.UniqueMove(std::move(unique));
						return newUnique;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (unique->IsChildOf(T::TypeGuid()))
					{
						JUniquePtr newUnique;
						newUnique.UniqueMove(std::move(unique));
						return newUnique;
					}
				}
				return nullptr;
			}
		private:
			template<typename NewType>
			void UniqueMove(JUniquePtr<NewType>&& rhs)noexcept
			{
				Clear();	 
				PtrBase::SetValidPointer(rhs);
				rhs.SetInvalidPointer();
			}
			void UniqueConnect(T * ptr)noexcept
			{ 
				PtrBase::SetValidPointer(ptr);
			}
			void UniqueDisConnect()noexcept
			{
				if (!PtrBase::IsValid())
					return;

				ExchangePointer(nullptr);
			}
			void ExchangePointer(T* newPtr = nullptr)noexcept
			{
				if (PtrBase::IsValid())
					PtrBase::DestroyPointer();

				if (newPtr != nullptr)
					PtrBase::SetValidPointer(newPtr);
			}
		};

		//Unsafe
		/*
		* Owner pointer나 UniquePointer에 Raw pointer가 변경된 경우
		* 수동으로 재할당이 필요.
		*/
		template<typename T>
		class JFastPtr final  
		{
		public:
			using ElementType = T;
		private:
			template<typename T> friend class JFastPtr;
		private: 
			using OwnerPtrBase = JOwnerPtrBase<T>;
			using UniquePtrBase = JUniquePtrBase<T>;
		private:
			T* ptr = nullptr;
		public:
			JFastPtr() = default;
			JFastPtr(nullptr_t) {}
			JFastPtr(const JFastPtr& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				//rhs.FastDisConnect();
			}
			JFastPtr(JFastPtr&& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				//rhs.FastDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr(const JOwnerPtrBase<NewType>& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr(const JUniquePtrBase<NewType>& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr& operator=(const JOwnerPtrBase<NewType>& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr& operator=(const JUniquePtrBase<NewType>& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				return *this;
			} 
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr(const JFastPtr<NewType>& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr(JFastPtr<NewType>&& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				//rhs.FastDisConnect();
			}
			JFastPtr& operator=(const JFastPtr& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				return *this;
			}
			JFastPtr& operator=(JFastPtr&& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr& operator=(const JFastPtr<NewType>& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JFastPtr& operator=(JFastPtr<NewType>&& rhs)noexcept
			{
				FastDisConnect();
				FastConnect(rhs);
				//rhs.FastDisConnect();
				return *this;
			}
			JFastPtr& operator=(nullptr_t)noexcept
			{
				FastDisConnect();
				return *this;
			}
			~JFastPtr()noexcept
			{
				Clear();
			}
		public:
			T& operator*() noexcept
			{
				return *ptr;
			}
			const T& operator*() const noexcept
			{
				return *ptr;
			}
			T* operator->() const noexcept
			{
				return ptr;
			}
			template<typename OtherType>
			bool operator==(JFastPtr<OtherType> rhs) const noexcept
			{
				return ptr == rhs.ptr;
			}
			template<typename OtherType>
			bool operator!=(JFastPtr<OtherType> rhs) const noexcept
			{
				return ptr != rhs.ptr;
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return ptr != rhs;
			}
			bool operator==(nullptr_t) const noexcept
			{
				return ptr == nullptr;
			}
			bool operator!=(nullptr_t) const noexcept
			{
				return ptr != nullptr;
			}  
		public:
			T* Get()const noexcept
			{
				return ptr;
			}
			T** GetAddress()noexcept
			{
				return &ptr;
			}
		public:
			void Clear()
			{
				FastDisConnect();
			}
			void Reset(OwnerPtrBase& rhs)
			{
				Clear();
				FastConnect(rhs);
			}
			void Reset(UniquePtrBase& rhs)
			{
				Clear();
				FastConnect(rhs);
			}
			T* Release()
			{
				T* res = ptr;
				FastDisConnect();
				return res;
			}
		public:
			template<typename ParentType>
			static JFastPtr<T> CreateChild(JOwnerPtrBase<ParentType>& fast)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!fast.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (fast->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JFastPtr<T> newUser;
						newUser.FastConnect(fast);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (fast->IsChildOf(T::TypeGuid()))
					{
						JFastPtr<T> newUser;
						newUser.FastConnect(fast);
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ParentType>
			static JFastPtr<T> CreateChild(JUniquePtrBase<ParentType>& fast)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!fast.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (fast->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JFastPtr<T> newUser;
						newUser.FastConnect(fast);
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (fast->IsChildOf(T::TypeGuid()))
					{
						JFastPtr<T> newUser;
						newUser.FastConnect(fast);
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ParentType>
			static JFastPtr<T> ConvertChild(JFastPtr<ParentType>&& fast)
			{
				if constexpr (!CanConvertChildType<T>())
					return nullptr;

				if (!fast.IsValid())
					return nullptr;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (fast->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						JFastPtr<T> newUser;
						newUser.FastConnect(fast);
						fast.Clear();
						return newUser;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (fast->IsChildOf(T::TypeGuid()))
					{
						JFastPtr<T> newUser;
						newUser.FastConnect(fast);
						fast.Clear();
						return newUser;
					}
				}
				return nullptr;
			}
			template<typename ParentType>
			bool ConnnectChild(const JFastPtr<ParentType>& fast)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ParentType> || !std::is_base_of_v<JTypeBase, T>)
					return false;

				if (!fast.IsValid())
					return false;

				if constexpr (HasTypeInfo<T>::value)
				{
					if (fast->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
					{
						FastDisConnect();
						FastConnect(fast);
						return true;
					}
				}
				else if constexpr (HasTypeList<T>::value)
				{
					if (fast->IsChildOf(T::TypeGuid()))
					{
						FastDisConnect();
						FastConnect(fast);
						return true;
					}
				}
				return false;
			}
		private:
			template<typename NewType>
			void FastConnect(const JOwnerPtrBase<NewType>& ptrBase)noexcept
			{ 
				ptr = static_cast<T*>(ptrBase.Get());
			}
			template<typename NewType>
			void FastConnect(const JUniquePtrBase<NewType>& ptrBase)noexcept
			{
				ptr = static_cast<T*>(ptrBase.Get());
			}
			template<typename NewType>
			void FastConnect(const JFastPtr<NewType>& fastPtr)noexcept
			{
				ptr = fastPtr.ptr;
			}
			template<typename NewType>
			void FastConnect(const JFastPtr<NewType>&& fastPtr)noexcept
			{
				ptr = static_cast<T*>(fastPtr.ptr);
				fastPtr.ptr = nullptr;
			}
			void FastDisConnect()noexcept
			{  
				ptr = nullptr;
			}
		};

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

		class JPtrUtil
		{
		public:
			template<typename T, typename ...Param>
			static JOwnerPtr<T> MakeOwnerPtr(Param&&... var)
			{
				return JOwnerPtr<T>(new T(std::forward<Param>(var)...));
			}
			template<typename T, typename ...Param>
			static JUniquePtr<T> MakeUniquePtr(Param&&... var)
			{
				return JUniquePtr<T>(new T(std::forward<Param>(var)...));
			}
		};
	}

	template<typename T>
	using JOwnerPtr = Core::JOwnerPtr<T>;
	template<typename T>
	using JUserPtr = Core::JUserPtr<T>;
	template<typename T>
	using JWeakPtr = Core::JWeakPtr<T>;
	template<typename T>
	using JUniquePtr = Core::JUniquePtr<T>;
	template<typename T>
	using JFastPtr = Core::JFastPtr<T>;
}
