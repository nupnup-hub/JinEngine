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