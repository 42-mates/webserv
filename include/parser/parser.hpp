// 將 lexer.cpp 產生的 token 流“翻譯”為 JsonValue 物件，構建 JSON 的層次結構（物件、陣列、字串等）。

#pragma once
# include "json.hpp"
# include "lexer.hpp"
# include <fstream>
# include <iostream>

class	JsonParser
{
	public:
		static JsonValue	parse(std::ifstream& stream);

	private:
		typedef std::pair<std::string, JsonValue*>	Member;

		typedef JsonLexer::Tokens::iterator			iter;
		typedef JsonLexer::Tokens::const_iterator	const_iter;

		static JsonValue				handle_tokens(const_iter& begin, const const_iter& end);
		static JsonValue				handle_object(const_iter& begin, const const_iter& end);
		static JsonValue				handle_array(const_iter& begin, const const_iter& end);
		static JsonValue::member_type	handle_member(const_iter& begin, const const_iter& end);
};