#pragma once
#include"../Reflection/JReflection.h"

namespace JinEngine
{
	namespace Core
	{
		class JTypeBase
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JTypeBase)
		private:
			const size_t guid;	//used by classify same type instance
		public:
			size_t GetGuid()const noexcept; 
		protected:
			JTypeBase(const size_t guid);
			virtual ~JTypeBase() = default;
		};
	}
	using JTypeBase = Core::JTypeBase;
}