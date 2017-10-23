#include "parser_base.h"
#include "parser_string.h"
#include "parser_number.h"
#include "parser_array.h"
#include "parser_json.h"

using namespace json;

json_parser::json_parser()
	: parser_impl(parser_id::parser_json)
	, m_state_table
	{											// got													// transit to						// action
		{ e_json_read_state::initial,	{	/*}*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::rd_key_str,	BIND(json_parser::on_in_object)	} },
											/* */ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::initial,		BIND(json_parser::on_initial)		} },
											/* */ { e_json_special_symbols::other,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
		} },									// got													// transit to						// action
		{ e_json_read_state::rd_key_str, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::rd_key_str,	BIND(json_parser::on_reading_key)	} },
											/* */ { e_json_special_symbols::other,					{ e_json_read_state::rd_key_str,	BIND(json_parser::on_reading_key)	} },
		} },									// got													// transit to						// action
		{ e_json_read_state::wt_colon, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::wt_value,		BIND(json_parser::on_wait_value)	} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::wt_colon,		BIND(json_parser::on_wait_colon)	} },
		} },									// got													// transit to						// action
		{ e_json_read_state::wt_value, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::wt_value,		BIND(json_parser::on_wait_value)	} },
											/* */ { e_json_special_symbols::other,					{ e_json_read_state::rd_value,		BIND(json_parser::on_reading_value)	} },
		} },									// got													// transit to						// action
		{ e_json_read_state::rd_value, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::rd_key_str,	BIND(json_parser::on_reading_key)	} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::other,					{ e_json_read_state::rd_value,		BIND(json_parser::on_reading_value)	} },
		} },									// got													// transit to						// action
		{ e_json_read_state::in_object, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::out_object,	BIND(json_parser::on_out_object)	} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::in_object,		BIND(json_parser::on_in_object)		} },
											/* */ { e_json_special_symbols::other,					{ e_json_read_state::rd_key_str,	BIND(json_parser::on_reading_key)	} },
		} },									// got													// transit to						// action
		{ e_json_read_state::out_object, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::rd_key_str,	BIND(json_parser::on_reading_key)	} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
		} },									// got													// transit to						// action
		{ e_json_read_state::in_array, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)	} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
		} },									// got													// transit to						// action
		{ e_json_read_state::out_array, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)	} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
		} },									// got													// transit to						// action
		{ e_json_read_state::__fail__, {	/*{*/ { e_json_special_symbols::left_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*}*/ { e_json_special_symbols::right_curly_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*[*/ { e_json_special_symbols::left_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*]*/ { e_json_special_symbols::right_square_brace,		{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*,*/ { e_json_special_symbols::comma,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/*:*/ { e_json_special_symbols::colon,					{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
											/* */ { e_json_special_symbols::WS,						{ e_json_read_state::__fail__,		BIND(json_parser::on_failure)		} },
		} },									// got													// transit to						// action
	}
{
	;
}

json_parser::~json_parser()
{

}

// char to token name
json_parser::symbol_t
json_parser::token_type_of(const char& c) const
{
	if ((char)symbol_t::left_square_brace == c)		// [
		return symbol_t::left_square_brace;
	if ((char)symbol_t::right_square_brace == c)	// ]
		return symbol_t::right_square_brace;
	if ((char)symbol_t::left_curly_brace == c)		// {
		return symbol_t::left_curly_brace;
	if ((char)symbol_t::right_curly_brace == c)		// }
		return symbol_t::right_curly_brace;
	if ((char)symbol_t::comma == c)					// ,
		return symbol_t::comma;
	if ((char)symbol_t::colon == c)					// :
		return symbol_t::colon;
	if ((char)symbol_t::space == c)					// ' '
		return symbol_t::WS;
	if ((char)symbol_t::htab == c)					// \t
		return symbol_t::WS;
	if ((char)symbol_t::LF == c)					// \n
		return symbol_t::WS;
	if ((char)symbol_t::CR == c)					// \r
		return symbol_t::WS;

	return symbol_t::other;
};

error
json_parser::on_initial(const char& c, const int pos)
{
	state::set(e_json_read_state::initial);
	return error::ok;
}

error
json_parser::on_reading_key(const char& c, const int pos)
{
	if (!m_str_parser)
		m_str_parser.reset(new string_parser);

	state::set(e_json_read_state::rd_key_str);
	if (error::done == m_str_parser->step(c, pos))
	{
		state::set(e_json_read_state::wt_colon);
		m_current_processing_func = nullptr;
		
		return error::ok;
	}

	if (m_current_processing_func == nullptr)
		m_current_processing_func = BIND(json_parser::on_reading_key);

	return error::ok;
}

error
json_parser::on_wait_colon(const char& c, const int pos)
{
	state::set(e_json_read_state::wt_colon);
	return error::ok;
}

error
json_parser::on_wait_value(const char& c, const int pos)
{
	state::set(e_json_read_state::wt_value);
	
	m_active_parsers.clear();

	if (!m_json_parser) m_json_parser.reset(new json_parser());
	if (!m_str_parser) m_str_parser.reset(new string_parser());
	if (!m_num_parser) m_num_parser.reset(new number_parser());
	if (!m_bool_parser) m_bool_parser.reset();
	if (!m_null_parser) m_null_parser.reset();
	if (!m_array_parser) m_array_parser.reset();

	m_active_parsers.push_back(m_json_parser);
	m_active_parsers.push_back(m_str_parser);
	m_active_parsers.push_back(m_num_parser);
	m_active_parsers.push_back(m_bool_parser);
	m_active_parsers.push_back(m_null_parser);
	m_active_parsers.push_back(m_array_parser);

	return error::ok;
}

error
json_parser::on_reading_value(const char& c, const int pos)
{
	state::set(e_json_read_state::rd_value);
	
	auto it = m_active_parsers.begin();
	while (it != m_active_parsers.end())
	{
		if (nullptr == *it)
		{
			m_active_parsers.erase(it);
			it = m_active_parsers.begin();
			continue;
		}
		it++;
	}

	if (m_active_parsers.empty())
		return error::fatal;

	for (parser::ptr& p : m_active_parsers)
	{
		error result = error::ok;
		if (p)
			result = p->step(c, pos);
		if (result == error::fatal)
			p.reset();
	}

	return error::ok;
}

error 
json_parser::on_in_object(const char& c, const int pos)
{
	state::set(e_json_read_state::in_object);
	return error::ok;
}

error
json_parser::on_out_object(const char& c, const int pos)
{
	state::set(e_json_read_state::out_object);
	return error::ok;
}

error
json_parser::on_failure(const char& c, const int pos)
{
	state::set(e_json_read_state::__fail__);
	std::cout << "\t" << "Failed at: pos=" << pos << "(0x" << std::hex << std::setw(2) << std::setfill('0') << (int)c << ")" << std::resetiosflags(std::ios_base::basefield) << std::endl;
	return error::fatal;
}