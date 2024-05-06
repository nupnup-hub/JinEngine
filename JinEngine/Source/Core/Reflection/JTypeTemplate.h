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
		template<typename To, typename From>
		To* UnSafeFastCast(From* p)
		{
			return IsParent<To, From>() ? static_cast<To*>(p) : nullptr;
		}

		template<typename To, typename From>
		To* Cast(From* p)
		{
			if (IsInvalidType<From, To>())
				return nullptr;

			if constexpr (!std::is_base_of_v< From, To> && !std::is_base_of_v< From, To>)
				return nullptr;

			if constexpr (std::is_same_v<From, To>)
				return p;

			return p && p->GetTypeInfo().IsChildOf<To>() ? static_cast<To*>(p) : nullptr;
		}

		//Caution!
		//A templateBase and Template<T, ...> relationship is unsafe
		//Non template class is safe
		template<typename To, typename From>
		To* FastCast(From* p)
		{
			if (IsInvalidType<From, To>())
				return nullptr;

			if constexpr (!std::is_base_of_v< From, To> && !std::is_base_of_v< From, To>)
				return nullptr;

			if constexpr (std::is_same_v<From, To>)
				return p;

			if (To::TypeDepth::value > p->GetTypeDepth())
				return nullptr;
			else
				return IsParent<To, From>() ? static_cast<To*>(p) : nullptr;
		}
	}
}