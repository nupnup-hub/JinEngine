#pragma once
#include"JGuiWidgetInfoHandleBase.h"  

namespace JinEngine
{
	namespace Core
	{
		class JPropertyInfo;
		class JMethodInfo;
		class JTypeInfo;
		class JGuiWidgetInfo;

		template<typename T>
		class JGuiWidgetInfoHandle : public JGuiWidgetInfoHandleBase
		{
		public:
			template<typename ...Widget>
			JGuiWidgetInfoHandle(Widget&&... var)
				:JGuiWidgetInfoHandleBase(std::forward<Widget>(var)...)
			{}
		public:
			virtual T Get(void* object) = 0;
			virtual T UnsafeGet(void* object) = 0;
			virtual void Set(void* object, T&& value) = 0;
			virtual void UnsafeSet(void* object, T&& value) = 0;
		public:
			virtual bool CanInvokeGet()const noexcept = 0;
			virtual bool CanInvokeSet()const noexcept = 0;
		};

		template<typename T>
		class JPropertyGuiWidgetInfoHandle : public JGuiWidgetInfoHandle<T>
		{
		private:
			JPropertyInfo* propertyInfo;
		public:
			template<typename ...Widget>
			JPropertyGuiWidgetInfoHandle(JPropertyInfo* propertyInfo, Widget&&... var)
				:JGuiWidgetInfoHandle<T>(std::forward<Widget>(var)...), propertyInfo(propertyInfo)
			{}
		public:
			std::string GetName()const noexcept
			{
				return propertyInfo->Name();
			}
			JTypeInfo* GetTypeInfo()const noexcept
			{
				return propertyInfo->GetTypeInfo();
			}
			JParameterHint GetFieldHint()const noexcept
			{
				return propertyInfo->GetHint();
			}
		public:
			T Get(void* object)final
			{
				return propertyInfo->Get<T>(object);
			}
			T UnsafeGet(void* object)final
			{
				return propertyInfo->UnsafeGet<T>(object);
			}
			void Set(void* object, T&& value)final
			{
				propertyInfo->Set<T>(object, std::forward<T>(value));
			}
			void UnsafeSet(void* object, T&& value)final
			{
				propertyInfo->UnsafeSet<T>(object, std::forward<T>(value));
			}
		public:
			bool CanInvokeGet()const noexcept final
			{
				return true;
			}
			bool CanInvokeSet()const noexcept final
			{
				return true;
			}
		};

		template<typename T>
		class JMethodReadOnlyGuiWidgetInfoHandle : public JGuiWidgetInfoHandle<T>
		{
		private:
			const std::string displayName;
		private:
			JMethodInfo* getMethodInfo = nullptr; 
		public:
			template<typename ...Widget>
			JMethodReadOnlyGuiWidgetInfoHandle(JMethodInfo* getMethodInfo,
				const std::string displayName,
				Widget&&... var)
				:JGuiWidgetInfoHandle<T>(std::forward<Widget>(var)...),
				getMethodInfo(getMethodInfo),
				displayName(displayName)
			{}
		public:
			std::string GetName()const noexcept
			{
				return displayName;
			}
			JTypeInfo* GetTypeInfo()const noexcept
			{
				return getMethodInfo->GetTypeInfo();
			}
			JParameterHint GetFieldHint()const noexcept
			{
				return getMethodInfo->GetReturnHint();
			}
		public:
			T Get(void* object)final
			{
				return getMethodInfo->Invoke<T>(object);
			}
			T UnsafeGet(void* object)final
			{
				return getMethodInfo->UnsafeInvoke<T>(object);
			}
			void Set(void* object, T&& value)final{}
			void UnsafeSet(void* object, T&& value)final{}
		public:
			bool CanInvokeGet()const noexcept final
			{
				return getMethodInfo != nullptr;
			}
			bool CanInvokeSet()const noexcept final
			{
				return false;
			}
		};

		template<typename T>
		class JMethodGuiWidgetInfoHandle : public JGuiWidgetInfoHandle<T>
		{
		private:
			const std::string displayName;
		private:
			JMethodInfo* getMethodInfo = nullptr;
			JMethodInfo* setMethodInfo = nullptr;
		public:
			template<typename ...Widget>
			JMethodGuiWidgetInfoHandle(JMethodInfo* getMethodInfo,
				JMethodInfo* setMethodInfo,
				const std::string displayName,
				Widget&&... var)
				:JGuiWidgetInfoHandle<T>(std::forward<Widget>(var)...),
				getMethodInfo(getMethodInfo),
				setMethodInfo(setMethodInfo),
				displayName(displayName)
			{}
		public:
			std::string GetName()const noexcept
			{
				return displayName;
			}
			JTypeInfo* GetTypeInfo()const noexcept
			{
				return getMethodInfo->GetTypeInfo();
			}
			JParameterHint GetFieldHint()const noexcept
			{
				return getMethodInfo->GetReturnHint();
			}
		public:
			T Get(void* object)final
			{
				return getMethodInfo->Invoke<T>(object);
			}
			T UnsafeGet(void* object)final
			{
				return getMethodInfo->UnsafeInvoke<T>(object);
			}
			void Set(void* object, T&& value)final
			{
				setMethodInfo->Invoke<void>(object, std::forward<T>(value));
			}
			void UnsafeSet(void* object, T&& value)final
			{
				setMethodInfo->UnsafeInvoke<void>(object, std::forward<T>(value));
			}
		public:
			bool CanInvokeGet()const noexcept final
			{
				return getMethodInfo != nullptr;
			}
			bool CanInvokeSet()const noexcept final
			{
				return setMethodInfo != nullptr;
			}
		};

	}
}