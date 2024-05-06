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
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		class JTreeInterface
		{
		public:
			virtual uint GetChildrenCount()const noexcept = 0;
			virtual JTreeInterface* GetChild(const uint index)const noexcept = 0;
		public:
			virtual bool IsValidIndex(const uint index)const noexcept = 0;
		public:
			template<typename ...Param>
			static void Traversal(Core::JTreeInterface* node, void(*func)(Core::JTreeInterface*, Param...), Param&&... var)
			{
				if (node == nullptr)
					return;

				if (func != nullptr)
					func(node, std::forward<Param>(var)...);

				const uint count = node->GetChildrenCount();
				for (uint i = 0; i < count; ++i)
					Traversal(node->GetChild(i), func, std::forward<Param>(var)...);
			}
			template<typename EntryBind, typename ExitBind>
			static void Traversal(Core::JTreeInterface* node, EntryBind* entryB, ExitBind* exitB)
			{
				if (node == nullptr)
					return;

				if (entryB != nullptr)
					(*entryB)(node);

				const uint count = node->GetChildrenCount();
				for (uint i = 0; i < count; ++i)
					Traversal(node->GetChild(i), entryB, exitB);

				if (exitB != nullptr)
					(*exitB)(node);
			}
		};
	}
}