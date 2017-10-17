#include "json_tokenizer.h"

using namespace json;
#define BIND(__CLASS_METHOD__) std::bind(&__CLASS_METHOD__, this, std::placeholders::_1, std::placeholders::_2)
#pragma region -- key_string_parser --
string_parser::string_parser()
	: m_state_table
	{
		{ e_string_read_state::outside,	{		{ e_string_special_symbols::Quote,					{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::ReverseSolidus,			{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::Solidus,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::BackSpace,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::FormFeed,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::LineFeed,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::CarriageReturn,			{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::HTab,					{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::Unicode,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::Other,					{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
		} },
		{ e_string_read_state::inside,	{		{ e_string_special_symbols::Quote,					{ e_string_read_state::outside,	BIND(string_parser::on_outside)	} },
												{ e_string_special_symbols::ReverseSolidus,			{ e_string_read_state::escape,	BIND(string_parser::on_escape)	} },
												{ e_string_special_symbols::Solidus,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::BackSpace,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::FormFeed,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::LineFeed,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::CarriageReturn,			{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::HTab,					{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::Unicode,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::Other,					{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
		} },
		{ e_string_read_state::escape,	{		{ e_string_special_symbols::Quote,					{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::ReverseSolidus,			{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::Solidus,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::BackSpace,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::FormFeed,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::LineFeed,				{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::CarriageReturn,			{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::HTab,					{ e_string_read_state::inside,	BIND(string_parser::on_inside)	} },
												{ e_string_special_symbols::Unicode,				{ e_string_read_state::inside,	BIND(string_parser::on_unicode)	} },
												{ e_string_special_symbols::Other,					{ e_string_read_state::failure,	BIND(string_parser::on_failure) } },
		} },
		{ e_string_read_state::unicode,	{		{ e_string_special_symbols::Quote,					{ e_string_read_state::inside,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::ReverseSolidus,			{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::Solidus,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::BackSpace,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::FormFeed,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::LineFeed,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::CarriageReturn,			{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::HTab,					{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::Unicode,				{ e_string_read_state::failure,	BIND(string_parser::on_failure)	} },
												{ e_string_special_symbols::Other,					{ e_string_read_state::unicode,	BIND(string_parser::on_unicode) } },
		} },
	}
{

}

string_parser::~string_parser()
{

}

error 
string_parser::on_outside(const char&c, const int pos)
{
	state::set(e_string_read_state::outside);
	return error::done;
}

error 
string_parser::on_inside(const char&c, const int pos)
{
	state::set(e_string_read_state::inside);
	return error::ok;
}

error 
string_parser::on_escape(const char&c, const int pos)
{
	state::set(e_string_read_state::escape);
	return error::ok;
}

error 
string_parser::on_unicode(const char&c, const int pos)
{
	state::set(e_string_read_state::unicode);
	return error::ok;
}

error 
string_parser::on_failure(const char&c, const int pos)
{
	state::set(e_string_read_state::failure);
	return error::fatal;
}

e_string_special_symbols
string_parser::token_type_of(const char& c) const
{
	const read_state_t state = state::get();
	if ((char)symbol_t::Quote == c)			   // string start/end		an ordinary symbol
		return (state != read_state_t::escape) ? symbol_t::Quote		:	symbol_t::Other;
	
	if ((char)symbol_t::ReverseSolidus == c)		// escape start			an ordinary symbol
		return (state != read_state_t::escape) ? symbol_t::ReverseSolidus : symbol_t::Other;
	
 	if ((char)symbol_t::Solidus == c)
 		return symbol_t::Other/*Solidus*/;
	
	if ((char)symbol_t::BackSpace == c)			//    b				\b
		return (state != read_state_t::escape) ? symbol_t::Other : symbol_t::BackSpace;
	
	if ((char)symbol_t::FormFeed == c)			//    f				\f
		return (state != read_state_t::escape) ? symbol_t::Other : symbol_t::FormFeed;
	
	if ((char)symbol_t::LineFeed == c)			//    n				\n
		return (state != read_state_t::escape) ? symbol_t::Other : symbol_t::LineFeed;
	
	if ((char)symbol_t::CarriageReturn == c)		//    r				\r 
		return (state != read_state_t::escape) ? symbol_t::Other : symbol_t::CarriageReturn;
	
	if ((char)symbol_t::HTab == c)				//    t				\t
		return (state != read_state_t::escape) ? symbol_t::Other : symbol_t::HTab;
	
	if((char)symbol_t::Unicode == c)				//    u				\u
		return (state != read_state_t::escape) ? symbol_t::Other: symbol_t::Unicode;
	
	return symbol_t::Other;
}
#pragma endregion
#pragma region -- number_praser --
number_parser::number_parser()
{
}

number_parser::~number_parser()
{
}

error 
number_parser::on_initial(const char& c, const int pos)
{
	return error::ok;
}

error 
number_parser::on_minus(const char& c, const int pos)
{
	return error::ok;
}

error 
number_parser::on_integer(const char& c, const int pos)
{
	return error::ok;
}

error 
number_parser::on_fractional(const char& c, const int pos)
{
	return error::ok;
}

error 
number_parser::on_exponential(const char& c, const int pos)
{
	return error::ok;
}
#pragma endregion
#pragma region -- json_parser --
json_parser::json_parser()
	: m_str_parser(new string_parser())
	, m_json_parser(new json_parser())
	, m_num_parser(nullptr)
	, m_bool_parser(nullptr)
	, m_null_parser(nullptr)
	, m_array_parser(nullptr)
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
	return error::ok;
}

error
json_parser::on_reading_value(const char& c, const int pos)
{
	state::set(e_json_read_state::rd_value);
	
	#error

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
	return error::ok;
}
#pragma endregion


error
json::process(const std::string& input)
{
	std::stringstream sstr;
	sstr.str(input);
	return process(sstr);
}

error
json::process(/*const */std::istream& input)
{
	char c = 0;
	json_parser p;
	
	while (input >> std::noskipws >> c)
	{
		std::cout << c;

		if (error::ok != p.step(c, (int)input.tellg() - 1))
		{
			std::cout << "Error while parsing" << std::endl;
			break;
		}
	}

	std::cout << std::endl;

	return error::ok;
}
