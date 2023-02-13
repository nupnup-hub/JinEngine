#pragma once

namespace JinEngine
{
	namespace Core
	{
		template<typename T>
		class JTypeCashInterface
		{
			friend T;
		public:
			virtual ~JTypeCashInterface() = default;
		private:
			virtual bool AddType(T* newType)noexcept = 0;
			virtual bool RemoveType(T* type)noexcept = 0;
		};
	}
}