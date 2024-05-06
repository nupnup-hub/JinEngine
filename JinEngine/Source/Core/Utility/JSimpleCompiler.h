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
		enum class J_COMMAND_KIND
		{
			UNKNOWN, END_OF_TOKEN,

			NULL_LITERAL,
			TRUE_LITERAL, FALSE_LITERAL,
			NUMBER_LITERAL, STRING_LITERAL,
			IDENTIFIER,

			FUNCTION, RETURN,
			VARIABLE,
			FOR, BREAK, CONTINUE,
			IF, ELIF, ELSE,
			PRINT, PRINT_LINE,

			LOGICAL_AND, LOGICAL_OR,
			ASSIGNMENT,
			ADD, SUBTRACT,
			MULTIPLY, DIVIDE, MODULO,
			EQUAL, NOT_EQUAL,
			LESS_THAN, GREATER_THAN,
			LESS_OR_EQUAL, GREATER_OR_EQUAL,

			COMMA, COLON, SEMICOLON,
			LEFT_PAREN, RIGHT_PAREN,
			LEFT_BRACE, RIGHT_BRACE,
			LEFT_BRAKET, RIGHT_BRAKET,
		};
		struct JCommandToken
		{
		public:
			J_COMMAND_KIND kind = J_COMMAND_KIND::UNKNOWN;
			std::string str;
		};
		class JSimpleCompiler
		{
		public:
			static std::vector<JCommandToken> Scan(std::string& command)noexcept;
		};
	}
}