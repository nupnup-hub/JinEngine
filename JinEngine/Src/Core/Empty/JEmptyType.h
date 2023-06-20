#pragma once

namespace JinEngine
{
	namespace Core
	{
		class JEmptyType {};
		static constexpr JEmptyType empty;

		using EmptyParam = const JEmptyType&;

		inline bool operator==(const JEmptyType&, const JEmptyType&)
		{
			return true;
		}

		inline bool operator<(const JEmptyType&, const JEmptyType&)
		{
			return false;
		}

		inline bool operator>(const JEmptyType&, const JEmptyType&)
		{
			return false;
		}
	}
}
 