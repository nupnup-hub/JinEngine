#pragma once
#include"../JCoreEssential.h"
#include<string>
#include<vector> 

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