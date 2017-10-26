#include "parsers.h"

using namespace json;

string_parser::string_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::quote,					{ state_t::inside,		BIND(string_parser::on_initial)	} },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::inside,		{	{ event_t::quote,					{ state_t::done,		BIND(string_parser::on_done)	} },
									{ event_t::back_slash,				{ state_t::escape,		BIND(string_parser::on_escape)	} },
									{ event_t::other,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
		} },
		{ state_t::escape,		{	{ event_t::quote,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::back_slash,				{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::slash,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::alpha_b,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::alpha_f,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::alpha_n,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::alpha_r,					{ state_t::cr,			BIND(string_parser::on_escape)	} },
									{ event_t::alpha_t,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::alpha_u,					{ state_t::unicode_1,	BIND(string_parser::on_unicode)	} },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::cr,			{	{ event_t::back_slash,				{ state_t::lf,			BIND(string_parser::on_escape)	} },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::lf,			{	{ event_t::alpha_n,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_1,	{	{ event_t::hex_digit,				{ state_t::unicode_2,	BIND(string_parser::on_unicode)	} },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_2,	{	{ event_t::hex_digit,				{ state_t::unicode_3,	BIND(string_parser::on_unicode)	} },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_3,	{	{ event_t::hex_digit,				{ state_t::unicode_4,	BIND(string_parser::on_unicode) } },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_4,	{	{ event_t::hex_digit,				{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::done,		{	{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::failure,		{	{ event_t::other,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
	}
{
}

string_parser::~string_parser()
{
}

result
string_parser::step(const char& c, const int pos)
{
	return parser_impl::step(c, pos);
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

result 
string_parser::on_fail(const char&c, const int pos)
{
	return result::e_unexpected;
}


e_string_events
string_parser::to_event(const char& c) const
{
	event_t smb = event_t::other;
	switch (state::get())
	{
	case state_t::initial:
		if ((uint8_t)event_t::quote == c)
			smb = event_t::quote;
		break;
	case state_t::inside:
		if ((uint8_t)event_t::back_slash == c)
			smb = event_t::back_slash;
		if ((uint8_t)event_t::quote == c)
			smb = event_t::quote;
		break;
	case state_t::escape:
		switch (c)
		{
		case (uint8_t)event_t::quote:		smb = event_t::quote;		break;
		case (uint8_t)event_t::back_slash:	smb = event_t::back_slash;	break;
		case (uint8_t)event_t::slash:		smb = event_t::slash;		break;
		case (uint8_t)event_t::alpha_b:	smb = event_t::alpha_b;	break;
		case (uint8_t)event_t::alpha_f:	smb = event_t::alpha_f;	break;
		case (uint8_t)event_t::alpha_n:	smb = event_t::alpha_n;	break;
		case (uint8_t)event_t::alpha_r:	smb = event_t::alpha_r;	break;
		case (uint8_t)event_t::alpha_t:	smb = event_t::alpha_t;	break;
		case (uint8_t)event_t::alpha_u:	smb = event_t::alpha_u;	break;
		}
		break;
	case state_t::cr:
		if ((uint8_t)event_t::back_slash == c)
			return event_t::back_slash;
		break;
	case state_t::lf:
		if ((uint8_t)event_t::alpha_n == c)
			return event_t::alpha_n;
		break;
	case state_t::unicode_1:
	case state_t::unicode_2:
	case state_t::unicode_3:
	case state_t::unicode_4:
		if ((uint8_t)event_t::numeric_0 <= c && c <= (uint8_t)event_t::numeric_9)
			smb = event_t::hex_digit;
		if ((uint8_t)event_t::alpha_A <= c && c <= (uint8_t)event_t::alpha_F)
			smb = event_t::hex_digit;
		if ((uint8_t)event_t::alpha_a <= c && c <= (uint8_t)event_t::alpha_f)
			smb = event_t::hex_digit;
		break;
	case state_t::done:
		assert(0);
		break;
	}

	return smb;
}

void 
string_parser::reset()
{
	state::set(state_t::initial);
}
