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
		class JPtrData
		{
		private:
			using VoidPointer = void*;
		public:
			VoidPointer ptr = nullptr;
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
			T* UnsafeGet()noexcept
			{
				return static_cast<T*>(ptrData->ptr);
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
			T* Get()const noexcept
			{
				return PtrBase::Get();
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
				PtrBase::SetValidPtrData(new JPtrData());
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
					return nullptr;

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
		};
	}

	template<typename T>
	using JOwnerPtr = Core::JOwnerPtr<T>;
	template<typename T>
	using JUserPtr = Core::JUserPtr<T>;
	template<typename T>
	using JWeakPtr = Core::JWeakPtr<T>;
}
