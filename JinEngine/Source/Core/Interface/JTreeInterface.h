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