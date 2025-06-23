#include "parser.hpp"

JsonValue::member_type	JsonParser::handle_member(const_iter& begin, const const_iter& end)
{
	if (begin->type != JsonLexer::TokenType::STRING)
		throw std::runtime_error("Invalid object: 'key' member is invalid\n");
	std::string	str = begin->value;

	if (++begin == end)
		throw std::runtime_error("Invalid object: [1] missing syntax in member\n");
	else if (begin->type != JsonLexer::TokenType::COLON)
		throw std::runtime_error("Invalid object: [2] missing syntax in member\n");
	if (++begin == end)
		throw std::runtime_error("Invalid object: [3] missing syntax in member\n");

	JsonValue	val = handle_tokens(begin, end);
	return JsonValue::member_type(str, val);
}

JsonValue	JsonParser::handle_object(const_iter& begin, const const_iter& end)
{
	JsonValue	obj(JsonValue::JsonType::TOBJECT);

	for (begin++; begin < end; begin++)
	{
		if (begin->type == JsonLexer::TokenType::RCURLY)
			break ;
		obj.insert(handle_member(begin, end));
		if (++begin == end)
			throw std::runtime_error("Invalid object: [1] missing syntax\n");
		else if (begin->type == JsonLexer::TokenType::RCURLY)
			break ;
		else if (begin->type != JsonLexer::TokenType::COMMA)
			throw std::runtime_error("Invalid object: [2] missing syntax\n");
	}
	return obj;
}

JsonValue	JsonParser::handle_array(const_iter& begin, const const_iter& end)
{
	JsonValue	arr(JsonValue::JsonType::TARRAY);

	for (begin++; begin < end; begin++)
	{
		if (begin->type == JsonLexer::TokenType::RBRAC)
			break ;
		arr.push_back(handle_tokens(begin, end));
		if (++begin == end)
			throw std::runtime_error("Invalid array: missing syntax\n");
		else if (begin->type == JsonLexer::TokenType::RBRAC)
			break ;
		else if (begin->type != JsonLexer::TokenType::COMMA)
			throw std::runtime_error("Invalid array: missing syntax\n");
	}
	return arr;
}

JsonValue	JsonParser::handle_tokens(const_iter& begin, const const_iter& end)
{
	switch (begin->type)
	{
		case JsonLexer::TokenType::LCURLY:
			return handle_object(begin, end);
		case JsonLexer::TokenType::LBRAC:
			return handle_array(begin, end);
		case JsonLexer::TokenType::STRING:
		case JsonLexer::TokenType::DECIMAL:
		case JsonLexer::TokenType::VTRUE:
		case JsonLexer::TokenType::VFALSE:
			return JsonValue(*begin);
		case JsonLexer::TokenType::VNULL:
			return JsonValue();
		default:
			break;
	}
	throw std::runtime_error("Bad token\n");
}

JsonValue	JsonParser::parse(std::ifstream& stream)
{
	JsonLexer::Tokens					tokens = JsonLexer::lex(stream);
	JsonLexer::Tokens::const_iterator	it = tokens.begin();
	try
	{
		return handle_tokens(it, tokens.end());
	}
	catch (const std::exception& e)
	{
		throw;
	}
}
