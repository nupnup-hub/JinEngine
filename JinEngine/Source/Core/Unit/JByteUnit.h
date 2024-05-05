#pragma once 
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		enum class J_BYTE_UNIT
		{
			BYTE,
			KB,
			MB,
			GB
		};

		class JByteUnit
		{
		public:
			static constexpr size_t KB() { return 1 << 10; }
			static constexpr size_t MB() { return 1 << 20; }
			static constexpr size_t GB() { return 1 << 30; } 
		public:
			static size_t ConvertUnit(const size_t byte, const J_BYTE_UNIT type)
			{ ;
				if (type == J_BYTE_UNIT::KB)
					return byte / KB();
				else if (type == J_BYTE_UNIT::MB)
					return byte / MB();
				else if (type == J_BYTE_UNIT::GB)
					return byte / GB();
				else
					return byte;
			}
		public:
			static std::wstring ByteToWString(const size_t byte, const bool addRestNumber = false)
			{
				J_BYTE_UNIT type = J_BYTE_UNIT::BYTE;
				if (byte >= GB())
					type = J_BYTE_UNIT::GB;
				else if (byte >= MB())
					type = J_BYTE_UNIT::MB;
				else if (byte >= KB())
					type = J_BYTE_UNIT::KB;

				if (addRestNumber)
				{
					if (type == J_BYTE_UNIT::KB)
						return std::to_wstring(byte / KB()) + L"." + std::to_wstring(byte % KB()) + L"kb";
					else if (type == J_BYTE_UNIT::MB)
						return std::to_wstring(byte / MB()) + L"." + std::to_wstring(byte % MB()) + L"mb";
					else if (type == J_BYTE_UNIT::GB)
						return std::to_wstring(byte / GB()) + L"." + std::to_wstring(byte % GB()) + L"gb";
					else
						return std::to_wstring(byte);
				}
				else
				{
					if (type == J_BYTE_UNIT::KB)
						return std::to_wstring(byte / KB()) + L"kb";
					else if (type == J_BYTE_UNIT::MB)
						return std::to_wstring(byte / MB()) + L"mb";
					else if (type == J_BYTE_UNIT::GB)
						return std::to_wstring(byte / GB()) + L"gb";
					else
						return std::to_wstring(byte) + L"byte";
				}
			}
			static std::string ByteToString(const size_t byte, const bool addRestNumber = false)
			{
				J_BYTE_UNIT type = J_BYTE_UNIT::BYTE;
				if (byte >= GB())
					type = J_BYTE_UNIT::GB;
				else if (byte >= MB())
					type = J_BYTE_UNIT::MB;
				else if (byte >= KB())
					type = J_BYTE_UNIT::KB;

				if (addRestNumber)
				{
					if (type == J_BYTE_UNIT::KB)
						return std::to_string(byte / KB()) + "." + std::to_string(byte % KB()) + "kb";
					else if (type == J_BYTE_UNIT::MB)
						return std::to_string(byte / MB()) + "." + std::to_string(byte % MB()) + "mb";
					else if (type == J_BYTE_UNIT::GB)
						return std::to_string(byte / GB()) + "." + std::to_string(byte % GB()) + "gb";
					else
						return std::to_string(byte);
				}
				else
				{
					if (type == J_BYTE_UNIT::KB)
						return std::to_string(byte / KB()) + "kb";
					else if (type == J_BYTE_UNIT::MB)
						return std::to_string(byte / MB()) + "mb";
					else if (type == J_BYTE_UNIT::GB)
						return std::to_string(byte / GB()) + "gb";
					else
						return std::to_string(byte) + "byte";
				}
			}
		};
	}
}