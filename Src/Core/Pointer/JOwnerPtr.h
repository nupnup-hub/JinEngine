#pragma once 
#include"../Func/Functor/JFunctor.h"
#include"../Func/Callable/JCallable.h"
#include<memory> 

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		class JPtrBase
		{
		protected:
			JPtrBase* prevPtr = nullptr;
			JPtrBase* nextPtr = nullptr;
			T* ptr = nullptr;
		protected:
			JPtrBase() = default;
			virtual ~JPtrBase() {}
		public:
			T* Get() noexcept
			{
				return ptr;
			}
			bool IsValid()noexcept
			{
				return ptr != nullptr;
			}
			virtual bool IsOwner()const noexcept = 0;
			virtual void Clear()noexcept = 0;
		protected:
			bool HasPrev()const noexcept
			{
				return prevPtr != nullptr;
			}
			bool HasNext()const noexcept
			{
				return nextPtr != nullptr;
			}
		protected:
			bool ConnectPtr(JPtrBase& rhs)noexcept
			{
				ptr = rhs.ptr;
				if (IsValid() && !IsOwner())
				{
					JPtrBase* lastValidPtr = rhs.GetLastValidPtr();
					lastValidPtr->nextPtr = this;
					prevPtr = lastValidPtr;
					return true;
				}
				else
					return false;
			}
			bool DisConnectPtr()noexcept
			{
				if (IsValid() && !IsOwner())
				{
					if (nextPtr != nullptr)
					{
						prevPtr->nextPtr = nextPtr;
						nextPtr->prevPtr = prevPtr;
					}
					else
						prevPtr->nextPtr = nullptr;

					prevPtr = nullptr;
					nextPtr = nullptr;
					ptr = nullptr;
					return true;
				}
				else
					return false;
			}
		private:
			JPtrBase* GetLastValidPtr()noexcept
			{
				if (nextPtr != nullptr)
				{
					JPtrBase* res = nextPtr;
					while (res->nextPtr != nullptr)
						res = res->nextPtr;
					return res;
				}
				else
					return this;
			}
		};

		template<typename T>
		class JUserPtr : public JPtrBase<T>
		{
		protected:
			using PtrBase = JPtrBase<T>;
		private:
			//notify event when ptr is become to nullptr 
			std::unique_ptr<Core::JBindHandleBase> notifyInvalidPtrBind = nullptr;
		public:
			JUserPtr() = default;
			JUserPtr(PtrBase& rhs)
			{
				PtrBase::ConnectPtr(rhs);
			}
			JUserPtr(JUserPtr& rhs)
			{
				PtrBase::ConnectPtr(rhs);
			}
			JUserPtr& operator=(JUserPtr& rhs)
			{
				PtrBase::ConnectPtr(rhs);
				return *this;
			}
			JUserPtr(JUserPtr&& rhs)
			{
				PtrBase::ConnectPtr(rhs);
				rhs.Clear();
			}
			JUserPtr& operator=(JUserPtr&& rhs)
			{
				PtrBase::ConnectPtr(rhs);
				rhs.Clear();
				return *this;
			}
			~JUserPtr()
			{
				DeRegisterFunc();
				Clear();
			}
		public:
			bool IsOwner()const noexcept final
			{
				return false;
			}
			bool HasNotifyInvalidBind()const noexcept
			{
				return notifyInvalidPtrBind != nullptr;
			}
			void Clear()noexcept final
			{
				if (PtrBase::DisConnectPtr())
				{
					if (notifyInvalidPtrBind != nullptr)
						notifyInvalidPtrBind->InvokeCompletelyBind();
				}
			}
			void Reset(PtrBase& uPtr)
			{
				Clear();
				PtrBase::ConnectPtr(uPtr);
			}
		public:
			void RegisterFunc(std::unique_ptr<Core::JBindHandleBase> bindHandle)
			{
				if (bindHandle->IsCompletelyBind())
					notifyInvalidPtrBind = std::move(bindHandle);
			}
			void DeRegisterFunc()
			{
				notifyInvalidPtrBind.reset();
			}
		};

		template<typename T>
		class JOwnerPtr : public JPtrBase<T>
		{
		private:
			using PtrBase = JPtrBase<T>;
		public:
			JOwnerPtr(const T& instance)
			{
				PtrBase::ptr = new T(instance);
			}
			JOwnerPtr(T&& instance)
			{
				PtrBase::ptr = new T(instance);
			}
			~JOwnerPtr()
			{
				Clear();
			}
			JOwnerPtr(const JOwnerPtr& rhs) = delete;
			JOwnerPtr& operator=(const JOwnerPtr& rhs) = delete;
			JOwnerPtr(JOwnerPtr&& rhs)
				:PtrBase(rhs.Get())
			{
				rhs.Clear();
			}
			JOwnerPtr& operator=(JOwnerPtr&& rhs)
			{
				PtrBase::ptr = rhs.Get();
				rhs.Clear();
			}
		public:
			bool IsOwner()const noexcept final
			{
				return true;
			}
			void Clear()noexcept final
			{
				if (PtrBase::IsValid())
				{
					if (PtrBase::nextPtr != nullptr)
					{
						PtrBase* res = PtrBase::nextPtr;
						while (res != nullptr)
						{
							res->Clear();
							res = PtrBase::nextPtr;
						}
					}
					delete PtrBase::ptr;
					PtrBase::ptr = nullptr;
				}
			}
		};
		template<typename T, typename ...Param>
		JOwnerPtr<T> MakeOwnerPtr(Param... var)
		{
			return JOwnerPtr{ T(std::forward<Param>(var)...) };
		}
	}
}