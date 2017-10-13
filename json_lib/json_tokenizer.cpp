#include "json_tokenizer.h"

using namespace json;

tokenizer::tokenizer()
	: m_on_initial_func(std::bind(&tokenizer::on_initial, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_wait_pair_func(std::bind(&tokenizer::on_wait_pair, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_off_pair_func(std::bind(&tokenizer::on_off_pair, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_reading_key_func(std::bind(&tokenizer::on_reading_key, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_wait_colon_func(std::bind(&tokenizer::on_wait_colon, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_wait_value_func(std::bind(&tokenizer::on_wait_value, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_reading_value_func(std::bind(&tokenizer::on_reading_value, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_off_object_func(std::bind(&tokenizer::on_off_object, this, std::placeholders::_1, std::placeholders::_2))
	, m_on_failure_func(std::bind(&tokenizer::on_failure, this, std::placeholders::_1, std::placeholders::_2))
	, m_state_table
	{											// got								// transit to				// action
		{ e_read_state::initial, {		/*}*/ { e_token::left_curly_brace,		{ e_read_state::wait_pair,		m_on_wait_pair_func		} },
										/* */ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::comma,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::quote,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::__fail__,		m_on_failure_func		} },
		} },									// got								// transit to				// action
		{ e_read_state::wait_pair, {	/*}*/ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*"*/ { e_token::quote,					{ e_read_state::reading_key,	m_on_reading_key_func	} },
										/* */ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::quote,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::__fail__,		m_on_failure_func		} },
		} },									// got								// transit to				// action
		{ e_read_state::reading_key, {	/*{*/ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*}*/ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*[*/ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*]*/ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*,*/ { e_token::comma,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*:*/ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*"*/ { e_token::quote,					{ e_read_state::wait_colon,		m_on_wait_colon_func	} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::reading_key,	m_on_reading_key_func	} },
		} },									// got								// transit to				// action
		{ e_read_state::wait_colon, {	/*{*/ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*}*/ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*[*/ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*]*/ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*,*/ { e_token::comma,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*:*/ { e_token::colon,					{ e_read_state::wait_value,		m_on_wait_value_func	} },
										/*"*/ { e_token::quote,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::__fail__,		m_on_failure_func		} },
		} },									// got								// transit to				// action
		{ e_read_state::wait_value, {	/*{*/ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*}*/ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*[*/ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*]*/ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*,*/ { e_token::comma,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*:*/ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*"*/ { e_token::quote,					{ e_read_state::reading_value,	m_on_reading_value_func	} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::__fail__,		m_on_failure_func		} },
		} },									// got								// transit to				// action
		{ e_read_state::reading_value, {/*{*/ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*}*/ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*[*/ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*]*/ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*,*/ { e_token::comma,					{ e_read_state::wait_pair,		m_on_wait_pair_func		} },
										/*:*/ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*"*/ { e_token::quote,					{ e_read_state::off_pair,		m_on_off_pair_func		} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::reading_value,	m_on_reading_value_func } },
		} },									// got								// transit to				// action
		{ e_read_state::off_pair, {		/* */ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*}*/ { e_token::right_curly_brace,		{ e_read_state::off_object,		m_on_off_object_func	} },
										/* */ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*,*/ { e_token::comma,					{ e_read_state::wait_pair,		m_on_wait_pair_func		} },
										/* */ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::quote,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::__fail__,		m_on_failure_func		} },
		} },									// got								// transit to				// action
		{ e_read_state::off_object, {	/*{*/ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*}*/ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*[*/ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*]*/ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*,*/ { e_token::comma,					{ e_read_state::wait_pair,		m_on_wait_pair_func		} },
										/*:*/ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*"*/ { e_token::quote,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::eos,					{ e_read_state::initial,		m_on_initial_func		} },
										/* */ { e_token::other,					{ e_read_state::__fail__,		m_on_failure_func		} },
		} },									// got								// transit to				// action
		{ e_read_state::__fail__, {		/*{*/ { e_token::left_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*}*/ { e_token::right_curly_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*[*/ { e_token::left_square_brace,		{ e_read_state::__fail__,		m_on_failure_func		} },
										/*]*/ { e_token::right_square_brace,	{ e_read_state::__fail__,		m_on_failure_func		} },
										/*,*/ { e_token::comma,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*:*/ { e_token::colon,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/*"*/ { e_token::quote,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::eos,					{ e_read_state::__fail__,		m_on_failure_func		} },
										/* */ { e_token::other,					{ e_read_state::__fail__,		m_on_failure_func		} },
		} },									// got								// transit to				// action
	}
{
	// the state table
}

tokenizer::~tokenizer()
{

}

error 
tokenizer::process(const std::string& input)
{
	std::stringstream sstr;
	sstr.str(input);
	return process(sstr);
}

// char to token name
e_token
tokenizer::token_type_of(char& c)
{
	if ('{' == c)
		return left_curly_brace;
	if ('}' == c)
		return right_curly_brace;
	if ('[' == c)
		return left_square_brace;
	if (']' == c)
		return right_square_brace;
	if (',' == c)
		return comma;
	if (':' == c)
		return colon;
	if ('"' == c)
		return quote;

	return other;
};

// The step of the automata
error
tokenizer::step(char& c, const int pos)
{
	e_token symbol = token_type_of(c);

	auto transition_group = m_state_table.at(state::get());
	auto transition = transition_group.at(symbol);
	
	auto state = transition.first;

	if (/*e_read_state::__fail__ != transition.first && */transition.second)
		transition.second(c, pos);

	return error::ok;
};

error 
tokenizer::process(/*const */std::istream& input)
{
	char c = 0;

	while (input >> std::noskipws >> c && state::get() != e_read_state::__fail__)
		step(c, (int)input.tellg() - 1);

	std::cout << std::endl;

	return error::ok;
}

void
tokenizer::on_initial(const char& c, const int pos)
{
	state::set(e_read_state::initial);
}

void
tokenizer::on_wait_pair(const char& c, const int pos)
{
	state::set(e_read_state::wait_pair);
}

void
tokenizer::on_off_pair(const char& c, const int pos)
{
	state::set(e_read_state::off_pair);
}

void
tokenizer::on_reading_key(const char& c, const int pos)
{
	state::set(e_read_state::reading_key);
}

void
tokenizer::on_wait_colon(const char& c, const int pos)
{
	state::set(e_read_state::wait_colon);
}

void
tokenizer::on_wait_value(const char& c, const int pos)
{
	state::set(e_read_state::wait_value);
}

void
tokenizer::on_reading_value(const char& c, const int pos)
{
	state::set(e_read_state::reading_value);
}

void
tokenizer::on_off_object(const char& c, const int pos)
{
	state::set(e_read_state::off_object);
}

void
tokenizer::on_failure(const char& c, const int pos)
{
	state::set(e_read_state::__fail__);
}
