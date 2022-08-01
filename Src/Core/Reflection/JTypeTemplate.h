#pragma once

namespace JinEngine
{
	namespace Core
	{
		template<typename Child, typename Parent>
		bool IsParent()
		{
			if constexpr (std::is_void_v<Child> || std::is_void_v<Parent>)
				return false;

			if constexpr (std::is_same<Child, Parent>::value)
				return true;
			else
			{
				if constexpr (std::is_void_v<Child::ParentType>)
					return false;
				else
					return IsParent<Child::ParentType, Parent>();
			}
		}

		template<typename From, typename To>
		bool IsInvalidType()
		{
			if constexpr (std::is_void_v<From> || std::is_void_v<To> ||
				std::is_null_pointer_v<From> || std::is_null_pointer_v<To>)
				return true;
			else
				return false;
		}

		//It is only valid cast when From class depth level is over the To depth level
		template<typename From, typename To>
		To* UnSafeFastCast(From* p)
		{
			return IsParent<To, From>() ? static_cast<To*>(p) : nullptr;
		}

		template<typename From, typename To>
		To* Cast(From* p)
		{
			if (IsInvalidType<From, To>())
				return nullptr;

			if constexpr (!std::is_base_of_v< From, To> && !std::is_base_of_v< From, To>)
				return nullptr;

			if constexpr (std::is_same<From, To>::value)
				return p;

			return p && p->GetTypeInfo().IsChildOf<To>() ? static_cast<To*>(p) : nullptr;
		}

		//Caution!
		//A templateBase and Template<T, ...> relationship is unsafe
		//Non template class is safe
		template<typename From, typename To>
		To* FastCast(From* p)
		{
			if (IsInvalidType<From, To>())
				return nullptr;

			if constexpr (!std::is_base_of_v< From, To> && !std::is_base_of_v< From, To>)
				return nullptr;

			if constexpr (std::is_same<From, To>::value)
				return p;

			if (To::TypeDepth::value > p->GetTypeDepth())
				return nullptr;
			else
				return IsParent<To, From>() ? static_cast<To*>(p) : nullptr;
		}
	}
}