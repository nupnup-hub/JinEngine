#pragma once 
#include "../JDataType.h"  
#include<type_traits>  

namespace JinEngine
{
	namespace Core
	{
		class JTypeBase;
		class JTypeInfo;

		class JPtrData
		{
		private:
			using VoidPointer = void*; 
		public:
			VoidPointer ptr = nullptr;
			uint userCount = 0;
			uint weakCount = 0;
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
			JPtrData* ptrData = nullptr;
		public:
			T& operator*()
			{ 
				return *Get();
			}
			T* operator->() const noexcept
			{
				return IsValid() ? static_cast<T*>(ptrData->ptr) : nullptr;
			}
		public:
			T* Get()const noexcept
			{
				return IsValid() ? static_cast<T*>(ptrData->ptr) : nullptr;
			}
			T* UnsafeGet()const noexcept
			{
				return static_cast<T*>(ptrData->ptr);
			}
			uint GetUserCount()const noexcept
			{
				return ptrData != nullptr ? ptrData->userCount : 0;
			}
			uint GetWeakCount()const noexcept
			{
				return ptrData != nullptr ? ptrData->weakCount : 0;
			}
		protected:
			template<typename U>
			void SetValidPtrData(const JPtrBase<U>& otherPtrBase)
			{
				ptrData = otherPtrBase.ptrData;
			}
			void SetValidPtrData(JPtrData* newPtrData)
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
		private:
			using PtrBase = JPtrBase<T>;
		protected:
			void SetValidPointer(T* ptr)
			{
				PtrBase::ptrData->ptr = ptr;
			}
			void SetInValidPointer()noexcept
			{
				PtrBase::ptrData->ptr = nullptr;
			}
		};
		template<typename T>
		class JUserPtrInterface : public JPtrBase<T>
		{
		private:
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
		private:
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
		private:
			template<typename T> friend class JOwnerPtr;
		private:
			using Owner = JOwnerPtrInterface<T>;
			using PtrBase = typename Owner::PtrBase;
		public:
			JOwnerPtr() = default;
			JOwnerPtr(nullptr_t) {}
			JOwnerPtr(const JOwnerPtr& rhs) = delete;
			JOwnerPtr& operator=(const JOwnerPtr& rhs) = delete;
			JOwnerPtr(T* newPtr)
			{  
				OwnerConnect(newPtr);
			}
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
			JOwnerPtr& operator=(nullptr_t)
			{
				OwnerDisConnect();
				return *this;
			}
			~JOwnerPtr()
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
			T* Get()const noexcept
			{
				return PtrBase::Get();
			}
			void Reset(T* newPtr)
			{
				Clear();
				OwnerConnect(newPtr);
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
				T* ptr = PtrBase::Get();
				if (PtrBase::HasPtrData())
				{
					Owner::SetInValidPointer();
					//PtrBase::TryDestroyPtrData();
				}
				return ptr;
			}
			void Clear()
			{
				OwnerDisConnect();
			}
		public:
			//For JTypeBase
			template<typename BaseType>
			static JOwnerPtr ConvertChild(JOwnerPtr<BaseType>&& base)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, BaseType> || !std::is_base_of_v<JTypeBase, T>)
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
				PtrBase::SetValidPtrData(rhs);
				rhs.SetInValidPtrData(); 
			}
			void OwnerConnect(T* ptr)noexcept
			{
				PtrBase::SetValidPtrData(new JPtrData());
				Owner::SetValidPointer(ptr);
			}
			void OwnerDisConnect()noexcept
			{
				if (!PtrBase::HasPtrData())
					return;

				T* ptr = PtrBase::UnsafeGet();
				if (ptr != nullptr)
					delete ptr;

				Owner::SetInValidPointer();
				PtrBase::TryDestroyPtrData();
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
		private:
			template<typename T, typename = void>
			struct HasTypeInfo : std::false_type
			{};
			template<typename T>
			struct HasTypeInfo<T, std::void_t<decltype(&T::StaticTypeInfo)>> : std::true_type
			{};
		public:
			JUserPtr() = default;
			JUserPtr(nullptr_t) {}
			JUserPtr(const JUserPtr& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
			JUserPtr(JUserPtr&& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				//rhs.UserDisConnect();
			}
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
			JUserPtr(const JUserPtr<NewType>& rhs)
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
			JUserPtr& operator=(const JUserPtr& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			JUserPtr& operator=(JUserPtr&& rhs)
			{
				UserDisConnect();
				UserConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JUserPtr& operator=(const JUserPtr<NewType>& rhs)
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
		public:
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
				return PtrBase::Get();
			}
		public:
			T* Get()const noexcept
			{
				return PtrBase::Get();
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
				T* res = PtrBase::Get();
				UserDisConnect();
				return res;
			}
		public:
			//For JTypeBase
			template<typename ChildType>
			static JUserPtr<T> CreateChild(JPtrBase<ChildType>& child)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
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
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ChildType>
			static JUserPtr<T> ConvertChild(JUserPtr<ChildType>&& child)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
					return JUserPtr<T>{};

				if (!child.IsValid())
					return JUserPtr<T>{};

				if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					JUserPtr<T> newUser;
					newUser.UserConnect(child);
					child.Clear();
					return newUser;
				}
				else
					return JUserPtr<T>{};
			}
			template<typename ChildType>
			bool ConnnectChild(const JUserPtr<ChildType>& child)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
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
				PtrBase::SetValidPtrData(ptrBase); 
				if (PtrBase::HasPtrData())
				{
					User::AddUserCount();
					if constexpr (HasTypeInfo<T>::value && std::is_convertible_v<T*, JTypeBase*>)
					{
						T* ptr = PtrBase::UnsafeGet();
						if (ptr != nullptr && PtrBase::GetUserCount() == 1)
							ptr->GetTypeInfo().TryCancelLazyDestruction(ptr);
					}
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
			JWeakPtr(const JWeakPtr& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			JWeakPtr(JWeakPtr&& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(const JPtrBase<NewType>& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(const JPtrBase<NewType>& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(const JWeakPtr<NewType>& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr(JWeakPtr<NewType>&& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
			}
			JWeakPtr& operator=(const JWeakPtr& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			JWeakPtr& operator=(JWeakPtr&& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(const JWeakPtr<NewType>& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				return *this;
			}
			template<typename NewType, std::enable_if_t<std::is_convertible_v<NewType*, T*>, int> = 0>
			JWeakPtr& operator=(JWeakPtr<NewType>&& rhs)
			{
				WeakDisConnect();
				WeakConnect(rhs);
				//rhs.WeakDisConnect();
				return *this;
			}
			JWeakPtr& operator=(nullptr_t)
			{
				WeakDisConnect();
				return *this;
			}
			~JWeakPtr()
			{
				Clear();
			}
			template<typename OtherType>
			bool operator==(JWeakPtr<OtherType> rhs) const noexcept
			{
				return Get() != rhs.Get();
			}
			template<typename OtherType>
			bool operator!=(JWeakPtr<OtherType> rhs) const noexcept
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
				return PtrBase::Get();
			}
		public:
			T* Get()const noexcept
			{
				return PtrBase::Get();
			}
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
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
					return JWeakPtr<T>{};

				if (!child.IsValid())
					return JWeakPtr<T>{};

				if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					JWeakPtr<T> newUser;
					newUser.WeakConnect(child);
					return newUser;
				}
				else
					return JWeakPtr<T>{};
			}
			//For JTypeBase
			//Connect base user ... is same downcast base to t
			template<typename ChildType>
			static JWeakPtr<T> ConvertChild(JWeakPtr<ChildType>&& child)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
					return JWeakPtr<T>{};

				if (!child.IsValid())
					return JWeakPtr<T>{};

				if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					JWeakPtr<T> newUser;
					newUser.WeakConnect(child);
					child.Clear();
					return newUser;
				}
				else
					return JWeakPtr<T>{};
			}
			template<typename ChildType>
			bool ConnnectChild(const JWeakPtr<ChildType>& child)
			{
				if constexpr (!std::is_base_of_v<JTypeBase, ChildType> || !std::is_base_of_v<JTypeBase, T>)
					return false;

				if (!child.IsValid())
					return false;

				if (child->GetTypeInfo().IsChildOf(T::StaticTypeInfo()))
				{
					WeakDisConnect();
					WeakConnect(child);
					return true;
				}
				else
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
