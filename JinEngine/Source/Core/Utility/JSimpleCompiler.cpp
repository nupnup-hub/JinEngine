#include"JSimpleCompiler.h"
#include<map> 
namespace JinEngine::Core
{
	namespace Private
	{ 
		//reference crafting-compiler-master
		enum class CHAR_TYPE
		{
			UNKNOWN,
			WHITE_SPACE,
			NUMBER_LITERAL,
			STRING_LITERAL,
			IDENTIFIER_AND_KEYWORD,
			OPERATOR_AND_PUNCTUATOR,
		};

		static std::map<std::string, J_COMMAND_KIND> stringToKind =
		{
		  {"#unknown",    J_COMMAND_KIND::UNKNOWN},
		  {"#EndOfJCommandToken", J_COMMAND_KIND::END_OF_TOKEN},

		  {"null",        J_COMMAND_KIND::NULL_LITERAL},
		  {"true",        J_COMMAND_KIND::TRUE_LITERAL},
		  {"false",       J_COMMAND_KIND::FALSE_LITERAL},
		  {"#Number",     J_COMMAND_KIND::NUMBER_LITERAL},
		  {"#String",     J_COMMAND_KIND::STRING_LITERAL},
		  {"#identifier", J_COMMAND_KIND::IDENTIFIER},

		  {"function",    J_COMMAND_KIND::FUNCTION},
		  {"return",      J_COMMAND_KIND::RETURN},
		  {"var",         J_COMMAND_KIND::VARIABLE},
		  {"for",         J_COMMAND_KIND::FOR},
		  {"break",       J_COMMAND_KIND::BREAK},
		  {"continue",    J_COMMAND_KIND::CONTINUE},
		  {"if",          J_COMMAND_KIND::IF},
		  {"elif",        J_COMMAND_KIND::ELIF},
		  {"else",        J_COMMAND_KIND::ELSE},
		  {"print",       J_COMMAND_KIND::PRINT},
		  {"printLine",   J_COMMAND_KIND::PRINT_LINE},

		  {"and",         J_COMMAND_KIND::LOGICAL_AND},
		  {"or",          J_COMMAND_KIND::LOGICAL_OR},

		  {"=",           J_COMMAND_KIND::ASSIGNMENT},

		  {"+",           J_COMMAND_KIND::ADD},
		  {"-",           J_COMMAND_KIND::SUBTRACT},
		  {"*",           J_COMMAND_KIND::MULTIPLY},
		  {"/",           J_COMMAND_KIND::DIVIDE},
		  {"%",           J_COMMAND_KIND::MODULO},

		  {"==",          J_COMMAND_KIND::EQUAL},
		  {"!=",          J_COMMAND_KIND::NOT_EQUAL},
		  {"<",           J_COMMAND_KIND::LESS_THAN},
		  {">",           J_COMMAND_KIND::GREATER_THAN},
		  {"<=",          J_COMMAND_KIND::LESS_OR_EQUAL},
		  {">=",          J_COMMAND_KIND::GREATER_OR_EQUAL},

		  {",",           J_COMMAND_KIND::COMMA},
		  {":",           J_COMMAND_KIND::COLON},
		  {";",           J_COMMAND_KIND::SEMICOLON},
		  {"(",           J_COMMAND_KIND::LEFT_PAREN},
		  {")",           J_COMMAND_KIND::RIGHT_PAREN},
		  {"{",           J_COMMAND_KIND::LEFT_BRACE},
		  {"}",           J_COMMAND_KIND::RIGHT_BRACE},
		  {"[",           J_COMMAND_KIND::LEFT_BRAKET},
		  {"]",           J_COMMAND_KIND::RIGHT_BRAKET},
		};
		static auto KindToString = [] 
		{
			std::map<J_COMMAND_KIND, std::string> result;
			for (auto& [key, value] : stringToKind)
				result[value] = key;
			return result;
		}();

		static J_COMMAND_KIND ToKind(const std::string& string)
		{
			if (stringToKind.count(string))
				return stringToKind.at(string);
			return J_COMMAND_KIND::UNKNOWN;
		}

		static std::string ToString(const J_COMMAND_KIND type)
		{
			if (KindToString.count(type))
				return KindToString.at(type);
			return "";
		}
	}
#pragma region Scan
	namespace Private
	{
		static CHAR_TYPE GetCharType(char c)noexcept
		{
			if (' ' == c || '\t' == c || '\r' == c || '\n' == c)
				return CHAR_TYPE::WHITE_SPACE;
			if ('0' <= c && c <= '9')
				return CHAR_TYPE::NUMBER_LITERAL;
			if (c == '\'')
				return CHAR_TYPE::STRING_LITERAL;
			if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z')
				return CHAR_TYPE::IDENTIFIER_AND_KEYWORD;
			if (33 <= c && c <= 47 && c != '\'' ||
				58 <= c && c <= 64 ||
				91 <= c && c <= 96 ||
				123 <= c && c <= 126)
				return CHAR_TYPE::OPERATOR_AND_PUNCTUATOR;
			return CHAR_TYPE::UNKNOWN;
		}
		static bool IsCharType(char c, CHAR_TYPE type)noexcept
		{
			switch (type) 
			{
			case CHAR_TYPE::NUMBER_LITERAL:
			{
				return '0' <= c && c <= '9';
			}
			case CHAR_TYPE::STRING_LITERAL: 
			{
				return 32 <= c && c <= 126 && c != '\'';
			}
			case CHAR_TYPE::IDENTIFIER_AND_KEYWORD: 
			{
				return '0' <= c && c <= '9' ||
					'a' <= c && c <= 'z' ||
					'A' <= c && c <= 'Z';
			}
			case CHAR_TYPE::OPERATOR_AND_PUNCTUATOR:
			{
				return 33 <= c && c <= 47 ||
					58 <= c && c <= 64 ||
					91 <= c && c <= 96 ||
					123 <= c && c <= 126;
			}
			default:  
				return false;
			}
		}
		static JCommandToken ScanNumberLiteral(std::string::iterator& current)noexcept
		{
			std::string string;
			while (IsCharType(*current, CHAR_TYPE::NUMBER_LITERAL))
				string += *current++;
			if (*current == '.') 
			{
				string += *current++;
				while (IsCharType(*current, CHAR_TYPE::NUMBER_LITERAL))
					string += *current++;
			}
			return JCommandToken{ J_COMMAND_KIND::NUMBER_LITERAL, string };
		}
		static JCommandToken ScanStringLiteral(std::string::iterator& current)noexcept
		{
			std::string string;
			current++;
			while (IsCharType(*current, CHAR_TYPE::STRING_LITERAL))
				string += *current++;
			current++;
			if (*current != '\'')
				return JCommandToken{};
			else
				return JCommandToken{ J_COMMAND_KIND::STRING_LITERAL, string };
		}
		static JCommandToken ScanIdentifierAndKeyword(std::string::iterator& current)noexcept
		{
			std::string string;
			while (IsCharType(*current, CHAR_TYPE::IDENTIFIER_AND_KEYWORD))
				string += *current++;
			auto kind = ToKind(string);
			if (kind == J_COMMAND_KIND::UNKNOWN)
				kind = J_COMMAND_KIND::IDENTIFIER;
			return JCommandToken{ kind, string };
		}
		static JCommandToken ScanOperatorAndPunctuator(std::string::iterator& current)noexcept
		{
			std::string string;
			while (IsCharType(*current, CHAR_TYPE::OPERATOR_AND_PUNCTUATOR))
				string += *current++;
			while (string.empty() == false && ToKind(string) == J_COMMAND_KIND::UNKNOWN)
			{
				string.pop_back();
				current--;
			}
			if (string.empty())
			{
				current++;
				return JCommandToken{};
			}
			else
				return JCommandToken{ ToKind(string), string };
		}
		static std::vector<JCommandToken> Scan(std::string& command)
		{
			std::vector<JCommandToken> result;
			std::string::iterator current = command.begin();
			while (*current != '\0')
			{
				switch (Private::GetCharType(*current))
				{
				case JinEngine::Core::Private::CHAR_TYPE::WHITE_SPACE:
				{
					current += 1;
					break;
				}
				case JinEngine::Core::Private::CHAR_TYPE::NUMBER_LITERAL:
				{ 
					result.emplace_back(ScanNumberLiteral(current));
					break;
				}
				case JinEngine::Core::Private::CHAR_TYPE::STRING_LITERAL:
				{
					JCommandToken t = ScanStringLiteral(current);
					if (t.kind != J_COMMAND_KIND::UNKNOWN)
						result.emplace_back(std::move(t));
					else
						return std::vector<JCommandToken>{};	//err
					break;
				}
				case JinEngine::Core::Private::CHAR_TYPE::IDENTIFIER_AND_KEYWORD:
				{
					result.push_back(ScanIdentifierAndKeyword(current));
					break;
				}
				case JinEngine::Core::Private::CHAR_TYPE::OPERATOR_AND_PUNCTUATOR:
				{
					JCommandToken t = ScanOperatorAndPunctuator(current);
					if (t.kind != J_COMMAND_KIND::UNKNOWN)
						result.emplace_back(std::move(t));
					else
						return std::vector<JCommandToken>{};	//err
					break;
				}
				default:
				{
					++current;
					break;
				}
				}
			}
			return result;
		}
	}
#pragma endregion
	std::vector<JCommandToken> JSimpleCompiler::Scan(std::string& command)noexcept
	{
		return Private::Scan(command);
	}
}