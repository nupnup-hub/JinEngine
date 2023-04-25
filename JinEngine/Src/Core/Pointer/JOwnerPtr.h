#pragma once 
#include "../JDataType.h"  
#include<type_traits>  

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
		class JPtrRefCount
		{
		private:
			bool isValid = false;
			uint userCount = 0;
		public:
			bool IsValid()const noexcept
			{
				return isValid;
			}
			void SetValid(bool value)noexcept
			{
				isValid = value;
			}
		public:
			uint GetUserCount()const noexcept
			{
				return userCount;
			}
			void AddUserCount()noexcept
			{
				++userCount;
			}
			void MinusUserCount()noexcept
			{
				--userCount;
			}
		};

		template<typename T> class JOwnerPtr;
		template<typename T> class JUserPtr;
		template<typename T>
		class JPtrBase
		{
		private:
			template<typename T> friend class JOwnerPtr;
			template<typename T> friend class JUserPtr;
		private:
			T* ptr = nullptr;
			JPtrRefCount* ptrRef = nullptr;
		public:
			T& operator*()
			{
				return *ptr;
			}
			T* operator->() const noexcept
			{
				return IsValid() ? ptr : nullptr;
			}
		public:
			T* Get()const noexcept
			{
				return IsValid() ? ptr : nullptr;
			}  
			int GetUserCount()const noexcept
			{
				return ptrRef->GetUserCount();
			}
			bool IsValid()const noexcept
			{
				return ptrRef != nullptr ? ptrRef->IsValid() && ptr != nullptr : false;
			}
		protected:
			void TryDestroyPtrRef()noexcept
			{
				if (!ptrRef->IsValid() && ptrRef->GetUserCount() == 0)
				{
					delete ptrRef;
					ptrRef = nullptr;
				}
			}
		};

		template<typename T>
		class JOwnerPtr final : public JPtrBase<T>
		{
		public:
			using ElementType = T;
		private:
			template<typename T> friend class JOwnerPtr;
		private:
			using PtrBase = JPtrBase<T>;
		public:
			JOwnerPtr() = default;
			JOwnerPtr(T* newPtr)
			{
				OwnerConnect(newPtr);
			}
			JOwnerPtr(const JOwnerPtr& rhs) = delete;
			JOwnerPtr& operator=(const JOwnerPtr& rhs) = delete;
			JOwnerPtr(JOwnerPtr&& rhs)
			{
				OwnerMove(rhs);
			}
			JOwnerPtr& operator=(JOwnerPtr&& rhs)
			{
				OwnerMove(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JOwnerPtr(JOwnerPtr<NewType>&& rhs)
			{
				OwnerMove(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JOwnerPtr& operator=(JOwnerPtr<NewType>&& rhs)
			{
				OwnerMove(rhs);
				return *this;
			}
			~JOwnerPtr()
			{
				Clear();
			}
			T* operator->() const noexcept
			{
				return PtrBase::IsValid() ? PtrBase::ptr : nullptr;
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
			T* Get()const noexcept
			{
				return PtrBase::IsValid() ? PtrBase::ptr : nullptr;
			}
			void Reset(T* newPtr)
			{
				if (newPtr != nullptr)
				{
					Clear();
					OwnerConnect(newPtr);
				}
			}
			template<typename ...Param>
			void Reset(Param&&... var)
			{
				Clear();
				T* newPtr = new T(std::forward<Param>(var)...);
				OwnerConnect(newPtr);
			}
			T* Release()
			{
				if (PtrBase::ptrRef != nullptr)
				{
					PtrBase::ptrRef->SetValid(false);
					PtrBase::TryDestroyPtrRef();
					PtrBase::ptrRef = nullptr;
				}
				T* ptr = PtrBase::ptr;
				PtrBase::ptr = nullptr;
				return ptr;
			}
			void Clear()
			{
				OwnerDisConnect();
			}
		public:
			//For JIdentifier
			template<typename BaseType>
			static JOwnerPtr ConvertChildUser(JOwnerPtr<BaseType>&& base)
			{
				if constexpr (!std::is_base_of_v<JIdentifier, BaseType> || !std::is_base_of_v<JIdentifier, T>)
					return false;

				if (!base.IsValid())
					return JOwnerPtr<T>{};

				if (base.Get()->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					JOwnerPtr newOwner;
					newOwner.OwnerMove(base);
					return newOwner;
				}
				else
					return JOwnerPtr{};
			}
		private:
			template<typename NewType>
			void OwnerMove(JOwnerPtr<NewType>& rhs)
			{
				Clear();
				PtrBase::ptr = static_cast<T*>(rhs.ptr);
				PtrBase::ptrRef = rhs.ptrRef;
				rhs.ptr = nullptr;
				rhs.ptrRef = nullptr;
			}
			void OwnerConnect(T* ptr)noexcept
			{
				PtrBase::ptr = ptr;
				PtrBase::ptrRef = new JPtrRefCount();
				PtrBase::ptrRef->SetValid(true);
			}
			void OwnerDisConnect()noexcept
			{
				if (PtrBase::ptr != nullptr)
				{
					if constexpr (std::is_base_of_v<JIdentifier, T>)
						delete PtrBase::ptr;
					else
						delete PtrBase::ptr;
					PtrBase::ptr = nullptr;
				}

				if (PtrBase::ptrRef != nullptr)
				{
					PtrBase::ptrRef->SetValid(false);
					PtrBase::TryDestroyPtrRef();
					PtrBase::ptrRef = nullptr;
				}
			}
		};

		template<typename T>
		class JUserPtr final : public JPtrBase<T>
		{
		public:
			using ElementType = T;
		private:
			template<typename T> friend class JUserPtr;
		private:
			using PtrBase = JPtrBase<T>;
		public:
			JUserPtr() = default;
			JUserPtr(nullptr_t){}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(const JPtrBase<NewType>& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(const JPtrBase<NewType>& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(JUserPtr<NewType>& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(JUserPtr<NewType>&& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(JUserPtr<NewType>& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(JUserPtr<NewType>&& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
				return *this;
			}
			JUserPtr& operator=(nullptr_t)
			{
				UserDisConnect();
				return *this;
			}
			~JUserPtr()
			{
				Clear();
			}
			template<typename OtherType>
			bool operator==(JUserPtr<OtherType> rhs) const noexcept
			{
				return Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(JUserPtr<OtherType> rhs) const noexcept
			{
				return !(Get() == rhs.Get());
			}
			template<typename OtherType>
			bool operator!=(OtherType* rhs) const noexcept
			{
				return !(Get() == rhs);
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
				return PtrBase::IsValid() ? PtrBase::ptr : nullptr;
			}
		public:
			T* Get()const noexcept
			{
				return PtrBase::IsValid() ? PtrBase::ptr : nullptr;
			}
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
				T* res = PtrBase::ptr;
				UserDisConnect();
				return res;
			}

			//For JIdentifier
			template<typename ChildType>
			static JUserPtr<T> CreateChildUser(JPtrBase<ChildType>& child)
			{
				if constexpr (!std::is_base_of_v<JIdentifier, ChildType> || !std::is_base_of_v<JIdentifier, T>)
					return JUserPtr<T>{};

				if (!child.IsValid())
					return JUserPtr<T>{};

				if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					JUserPtr<T> newUser;
					newUser.UserConnect(child); 
					return newUser;
				}
				else
					return JUserPtr<T>{};
			}
			//For JIdentifier
			//Connect base user ... is same downcast base to t
			template<typename ChildType>
			static JUserPtr<T> ConvertChildUser(JUserPtr<ChildType>&& child)
			{
				if constexpr (!std::is_base_of_v<JIdentifier, ChildType> || !std::is_base_of_v<JIdentifier, T>)
					return JUserPtr<T>{};

				if (!child.IsValid())
					return JUserPtr<T>{};

				if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					JUserPtr<T> newUser;
					newUser.UserConnect(child);
					child.UserDisConnect();
					return newUser;
				}
				else
					return JUserPtr<T>{};
			}

			template<typename ChildType>
			bool ConnnectChildUser(const JUserPtr<ChildType>& child)
			{
				if constexpr (!std::is_base_of_v<JIdentifier, ChildType> || !std::is_base_of_v<JIdentifier, T>)
					return false;

				if (!child.IsValid())
					return false;

				if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					UserDisConnect();
					UserConnect(child);
					return true;
				}
				else
					return false;
			}
		private:
			template<typename NewType>
			void UserConnect(const JPtrBase<NewType>& ptrBase)noexcept
			{
				PtrBase::ptr = static_cast<T*>(ptrBase.ptr);
				PtrBase::ptrRef = ptrBase.ptrRef;
				if (PtrBase::ptrRef != nullptr)
					PtrBase::ptrRef->AddUserCount();
			}
			void UserDisConnect()noexcept
			{
				if (PtrBase::ptrRef != nullptr)
				{
					PtrBase::ptrRef->MinusUserCount();
					PtrBase::TryDestroyPtrRef();
				}
				PtrBase::ptr = nullptr;
				PtrBase::ptrRef = nullptr;
			}
		};
 
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
}
