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
		};
	}
}