#include "parser_base.h"
#include "parser_string.h"
#include "parser_number.h"
#include "parser_array.h"
#include "parser_json.h"

using namespace json;

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