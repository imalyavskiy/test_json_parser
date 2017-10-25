#include "parsers.h"

using namespace json;

string_parser::string_parser()
	: m_state_table
	{
		{ read_state_t::initial,{		{ symbol_t::quote,					{ read_state_t::inside,		BIND(string_parser::on_initial)	} },
		} },
		{ read_state_t::inside,	{		{ symbol_t::quote,					{ read_state_t::done,	BIND(string_parser::on_done)	} },
										{ symbol_t::back_slash,				{ read_state_t::escape,		BIND(string_parser::on_escape)	} },
										{ symbol_t::other,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
		} },
		{ read_state_t::escape,	{		{ symbol_t::quote,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::back_slash,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::slash,					{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::alpha_b,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::alpha_f,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::alpha_n,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::alpha_r,				{ read_state_t::cr,			BIND(string_parser::on_escape)	} },
										{ symbol_t::alpha_t,				{ read_state_t::inside,		BIND(string_parser::on_inside)	} },
										{ symbol_t::alpha_u,				{ read_state_t::unicode_1,	BIND(string_parser::on_unicode)	} },
		} },
		{ read_state_t::cr,		  {		{ symbol_t::back_slash,				{ read_state_t::lf,			BIND(string_parser::on_escape) } },
		} },
		{ read_state_t::lf,		  {		{ symbol_t::alpha_n,				{ read_state_t::inside,		BIND(string_parser::on_inside) } },
		} },
		{ read_state_t::unicode_1,{		{ symbol_t::hex_digit,				{ read_state_t::unicode_2,	BIND(string_parser::on_unicode) } },
		} },
		{ read_state_t::unicode_2,{		{ symbol_t::hex_digit,				{ read_state_t::unicode_3,	BIND(string_parser::on_unicode) } },
		} },
		{ read_state_t::unicode_3,{		{ symbol_t::hex_digit,				{ read_state_t::unicode_4,	BIND(string_parser::on_unicode) } },
		} },
		{ read_state_t::unicode_4,{		{ symbol_t::hex_digit,				{ read_state_t::inside,		BIND(string_parser::on_inside) } },
		} },
	}
{

}

string_parser::~string_parser()
{

}

result
string_parser::on_initial(const char&c, const int pos)
{
	return result::s_need_more;
}

result
string_parser::on_inside(const char&c, const int pos)
{
	return result::s_need_more;
}

result
string_parser::on_escape(const char&c, const int pos)
{
	return result::s_need_more;
}

result
string_parser::on_unicode(const char&c, const int pos)
{
	return result::s_need_more;
}

result
string_parser::on_done(const char&c, const int pos)
{
	return result::s_done;
}


e_string_special_symbols
string_parser::token_type_of(const char& c) const
{
	symbol_t smb = symbol_t::other;
	switch (state::get())
	{
	case read_state_t::initial:
		if ((uint8_t)symbol_t::quote == c)
			smb = symbol_t::quote;
		break;
	case read_state_t::inside:
		if ((uint8_t)symbol_t::back_slash == c)
			smb = symbol_t::back_slash;
		if ((uint8_t)symbol_t::quote == c)
			smb = symbol_t::quote;
		break;
	case read_state_t::escape:
		switch (c)
		{
		case (uint8_t)symbol_t::quote:		smb = symbol_t::quote;		break;
		case (uint8_t)symbol_t::back_slash:	smb = symbol_t::back_slash;	break;
		case (uint8_t)symbol_t::slash:		smb = symbol_t::slash;		break;
		case (uint8_t)symbol_t::alpha_b:	smb = symbol_t::alpha_b;	break;
		case (uint8_t)symbol_t::alpha_f:	smb = symbol_t::alpha_f;	break;
		case (uint8_t)symbol_t::alpha_n:	smb = symbol_t::alpha_n;	break;
		case (uint8_t)symbol_t::alpha_r:	smb = symbol_t::alpha_r;	break;
		case (uint8_t)symbol_t::alpha_t:	smb = symbol_t::alpha_t;	break;
		case (uint8_t)symbol_t::alpha_u:	smb = symbol_t::alpha_u;	break;
		}
		break;
	case read_state_t::cr:
		if ((uint8_t)symbol_t::back_slash == c)
			return symbol_t::back_slash;
		break;
	case read_state_t::lf:
		if ((uint8_t)symbol_t::alpha_n == c)
			return symbol_t::alpha_n;
		break;
	case read_state_t::unicode_1:
	case read_state_t::unicode_2:
	case read_state_t::unicode_3:
	case read_state_t::unicode_4:
		if ((uint8_t)symbol_t::numeric_0 <= c && c <= (uint8_t)symbol_t::numeric_9)
			smb = symbol_t::hex_digit;
		if ((uint8_t)symbol_t::alpha_A <= c && c <= (uint8_t)symbol_t::alpha_F)
			smb = symbol_t::hex_digit;
		if ((uint8_t)symbol_t::alpha_a <= c && c <= (uint8_t)symbol_t::alpha_f)
			smb = symbol_t::hex_digit;
		break;
	case read_state_t::done:
		assert(0);
		break;
	}

	return smb;
}

void 
string_parser::reset()
{
	state::set(read_state_t::initial);
}
