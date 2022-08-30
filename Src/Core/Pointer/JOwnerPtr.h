#pragma once 
#include "../JDataType.h"
#include<type_traits>

namespace JinEngine
{
	class JObject;
	namespace Core
	{
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
			int GetUserCount()const noexcept
			{
				return ptrRef->GetUserCount();
			}
			bool IsValid()const noexcept
			{
				return ptrRef != nullptr ? ptrRef->IsValid() : false;
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
		class JOwnerPtr : public JPtrBase<T>
		{
		private:
			using PtrBase = JPtrBase<T>;
		public:
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
				return PtrBase::ptr;
			}
		public:
			T* Get()const noexcept
			{
				return PtrBase::ptr;
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
			void Clear()
			{
				OwnerDisConnect();
			}
		private:
			template<typename NewType>
			void OwnerMove(JOwnerPtr<NewType>& rhs)
			{
				Clear();
				PtrBase::ptr = rhs.ptr;
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
		class JUserPtr : public JPtrBase<T>
		{
		private:
			using PtrBase = JPtrBase<T>;
		public:
			JUserPtr() = default;
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr(const JPtrBase<NewType>& rhs)
			{
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
			JUserPtr(JUserPtr<NewType>&& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				rhs.UserDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(JUserPtr<NewType>&& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				rhs.UserDisConnect();
				return *this;
			}
			~JUserPtr()
			{
				Clear();
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
			//For JObject
			template<typename BaseType>
			bool ConnnectBaseUser(const JUserPtr<BaseType>& basePtr)
			{			  
				if constexpr (!std::is_base_of_v<JObject, BaseType> || !std::is_base_of_v<JObject, T>)
					return false;
				 
				if (T::GetTypeInfo()->IsChildOf(BaseType::GetTypeInfo()))
				{
					UserDisConnect();
					UserConnect(basePtr); 
					return true;
				}
				else
					return false;
			}
		private:
			template<typename NewType>
			void UserConnect(const JPtrBase<NewType>& ptrBase)noexcept
			{
				PtrBase::ptr = ptrBase.ptr;
				PtrBase::ptrRef = ptrBase.ptrRef;
				if(PtrBase::ptrRef != nullptr)
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