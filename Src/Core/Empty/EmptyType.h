#pragma once

namespace JinEngine
{
	namespace Core
	{
		class EmptyType {};
		static constexpr EmptyType empty;

		inline bool operator==(const EmptyType&, const EmptyType&)
		{
			return true;
		}

		inline bool operator<(const EmptyType&, const EmptyType&)
		{
			return false;
		}

		inline bool operator>(const EmptyType&, const EmptyType&)
		{
			return false;
		}
	}
}
 