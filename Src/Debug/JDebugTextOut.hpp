#include"JDebugTextOut.h"

namespace JinEngine
{
	namespace Core
	{
		template<typename T, typename C >
		void JDebugTextOut::PrintIntData(const T data, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;
			stream << guide << '\n';
			stream << data << '\n';
		}
		template<typename T, typename C >
		void JDebugTextOut::PrintFloatData(const T data, const std::wstring& guide)noexcept
		{
			if (!stream.is_open())
				return;
			stream << guide << '\n';
			stream << data << '\n';
		}
	}
}