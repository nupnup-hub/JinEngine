#pragma once
#include"../Reflection/JReflection.h"

namespace JinEngine
{
	namespace Core
	{
		class JTypeBase
		{
			REGISTER_CLASS_USE_ALLOCATOR(JTypeBase)
		private:
			const size_t guid;	//used by classify same type instance
		public:
			size_t GetGuid()const noexcept; 
		public:
			JTypeBase(const size_t guid);
			virtual ~JTypeBase();
		};
	}
	using JTypeBase = Core::JTypeBase;
}