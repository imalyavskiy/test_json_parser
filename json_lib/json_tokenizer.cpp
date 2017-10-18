#include "json_tokenizer.h"

using namespace json;
#define BIND(__CLASS_METHOD__) std::bind(&__CLASS_METHOD__, this, std::placeholders::_1, std::placeholders::_2)
#pragma region -- key_string_parser --
string_parser::string_parser()
	: parser_impl(parser_id::parser_string)
	, m_state_table
	{
		{ read_state_t::outside,{		{ symbol_t::Quote,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::ReverseSolidus,			{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::Solidus,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::BackSpace,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::FormFeed,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::LineFeed,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::CarriageReturn,			{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::HTab,					{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::Unicode,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::Other,					{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
		} },
		{ read_state_t::inside,	{		{ symbol_t::Quote,					{ read_state_t::outside,	BIND(string_parser::on_outside)	} },
										{ symbol_t::ReverseSolidus,			{ read_state_t::escape,		BIND(string_parser::on_escape)	} },
										{ symbol_t::Solidus,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::BackSpace,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::FormFeed,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::LineFeed,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::CarriageReturn,			{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::HTab,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::Unicode,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::Other,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
		} },
		{ read_state_t::escape,	{		{ symbol_t::Quote,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::ReverseSolidus,			{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::Solidus,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::BackSpace,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::FormFeed,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::LineFeed,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::CarriageReturn,			{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::HTab,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::Unicode,				{ read_state_t::inside,		BIND(string_parser::on_unicode)	} },
										{ symbol_t::Other,					{ read_state_t::_fail_,		BIND(string_parser::on_failure) } },
		} },
		{ read_state_t::unicode,{		{ symbol_t::Quote,					{ read_state_t::inside,		BIND(string_parser::on_failure)	} },
										{ symbol_t::ReverseSolidus,			{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::Solidus,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::BackSpace,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::FormFeed,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::LineFeed,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::CarriageReturn,			{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::HTab,					{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::Unicode,				{ read_state_t::_fail_,		BIND(string_parser::on_failure)	} },
										{ symbol_t::Other,					{ read_state_t::unicode,	BIND(string_parser::on_unicode) } },
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
	state::set(e_string_read_state::_fail_);
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
	: parser_impl(parser_id::parser_number)
	, m_state_table
	{
		{ read_state_t::initial,{		{ symbol_t::minus,					{ read_state_t::minus,			BIND(number_parser::on_minus)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::zero,			BIND(number_parser::on_zero)	} },
										{ symbol_t::dec_digit,				{ read_state_t::integer,		BIND(number_parser::on_initial)	} },
										{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_dot)		} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::minus,{			{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::zero,			BIND(number_parser::on_zero)	} },
										{ symbol_t::dec_digit,				{ read_state_t::integer,		BIND(number_parser::on_integer)	} },
										{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_dot)		} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::zero,{			{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::dot,			BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::dot,{			{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::fractional,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::fractional,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::integer,{		{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::fractional,{	{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::exponential,{	{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
		{ read_state_t::_failure_,{		{ symbol_t::minus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::plus,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_zero,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dec_digit,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::dot,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::exponent,				{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
										{ symbol_t::other,					{ read_state_t::_failure_,		BIND(number_parser::on_failure)	} },
		} },
	}
{
}

number_parser::~number_parser()
{
}

error 
number_parser::on_initial(const char& c, const int pos)
{
	state::set(read_state_t::initial);
	return error::ok;
}

error 
number_parser::on_minus(const char& c, const int pos)
{
	state::set(read_state_t::minus);
	return error::ok;
}

error 
number_parser::on_integer(const char& c, const int pos)
{
	state::set(read_state_t::integer);
	return error::ok;
}

error 
number_parser::on_fractional(const char& c, const int pos)
{
	state::set(read_state_t::fractional);
	return error::ok;
}

error 
number_parser::on_exponential(const char& c, const int pos)
{
	state::set(read_state_t::exponential);
	return error::ok;
}

error 
number_parser::on_failure(const char& c, const int pos)
{
	state::set(read_state_t::_failure_);
	return error::fatal;
}

error 
number_parser::on_zero(const char& c, const int pos)
{
	state::set(read_state_t::zero);
	return error::ok;
}

error 
number_parser::on_dot(const char& c, const int pos)
{
	state::set(read_state_t::dot);
	return error::ok;
}

number_parser::symbol_t
number_parser::token_type_of(const char& c) const
{
	if ((char)symbol_t::minus == c)
		return symbol_t::minus;
	if ((char)symbol_t::plus == c)
		return symbol_t::plus;
	if ((char)symbol_t::dec_zero == c)
		return symbol_t::dec_zero;
	if ((char)symbol_t::dot == c)
		return symbol_t::dot;
	if ((char)0x45 == c || (char)0x65 == c)
		return symbol_t::exponent;
	if (0x31 <= c && c <= 0x39)
		return symbol_t::dec_digit;

	return symbol_t::other;
}
#pragma endregion
#pragma region -- json_parser --
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
