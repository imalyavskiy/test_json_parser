#include "parsers.h"

using namespace json;

string_parser::string_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::quote,					{ state_t::inside,		BIND(string_parser::on_initial)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::inside,		{	{ event_t::quote,					{ state_t::done,		BIND(string_parser::on_done)	} },
									{ event_t::back_slash,				{ state_t::escape,		BIND(string_parser::on_escape)	} },
									{ event_t::symbol,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
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
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::cr,			{	{ event_t::back_slash,				{ state_t::lf,			BIND(string_parser::on_escape)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::lf,			{	{ event_t::alpha_n,					{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_1,	{	{ event_t::hex_digit,				{ state_t::unicode_2,	BIND(string_parser::on_unicode)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_2,	{	{ event_t::hex_digit,				{ state_t::unicode_3,	BIND(string_parser::on_unicode)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_3,	{	{ event_t::hex_digit,				{ state_t::unicode_4,	BIND(string_parser::on_unicode) } },
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::unicode_4,	{	{ event_t::hex_digit,				{ state_t::inside,		BIND(string_parser::on_inside)	} },
									{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::done,		{	{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
		{ state_t::failure,		{	{ event_t::symbol,					{ state_t::failure,		BIND(string_parser::on_fail)	} },
		} },
	}
{
}

string_parser::~string_parser()
{
}

void 
string_parser::reset()
{
	state::set(state_t::initial);
	m_value.reset();
}

result_t
string_parser::putchar(const char& c, const int pos)
{
	return parser_impl::step(to_event(c), c, pos);
}

value 
string_parser::get() const
{
	if (m_value.has_value())
		return *m_value;

	assert(0); // TODO: throw an exception
	return value();
}

e_string_events
string_parser::to_event(const char& c) const
{
	event_t smb = event_t::symbol;
	switch (state::get())
	{
	case state_t::initial:
		if (0x22 == c)
			smb = event_t::quote;
		break;
	case state_t::inside:
		if (0x5C == c)
			smb = event_t::back_slash;
		if (0x22 == c)
			smb = event_t::quote;
		break;
	case state_t::escape:
		switch (c)
		{
		case 0x22:	smb = event_t::quote;		break;
		case 0x5C:	smb = event_t::back_slash;	break;
		case 0x2F:	smb = event_t::slash;		break;
		case 0x62:	smb = event_t::alpha_b;	break;
		case 0x66:	smb = event_t::alpha_f;	break;
		case 0x6E:	smb = event_t::alpha_n;	break;
		case 0x72:	smb = event_t::alpha_r;	break;
		case 0x74:	smb = event_t::alpha_t;	break;
		case 0x75:	smb = event_t::alpha_u;	break;
		}
		break;
	case state_t::cr:
		if (0x5C == c)
			return event_t::back_slash;
		break;
	case state_t::lf:
		if (0x6E == c)
			return event_t::alpha_n;
		break;
	case state_t::unicode_1:
	case state_t::unicode_2:
	case state_t::unicode_3:
	case state_t::unicode_4:
		if (0x30 <= c && c <= 0x39)
			smb = event_t::hex_digit;
		if (0x41 <= c && c <= 0x46)
			smb = event_t::hex_digit;
		if (0x61 <= c && c <= 0x66)
			smb = event_t::hex_digit;
		break;
	case state_t::done:
		assert(0);
		break;
	}

	return smb;
}

string_parser::event_t
string_parser::to_event(const result_t& c) const
{
	return event_t::symbol;
}

result_t
string_parser::on_initial(const char&c, const int pos)
{
	return result_t::s_need_more;
}

result_t
string_parser::on_inside(const char&c, const int pos)
{
	if (!m_value.has_value())
		m_value.emplace();

	(*m_value) += c;

	return result_t::s_need_more;
}

result_t
string_parser::on_escape(const char&c, const int pos)
{
	assert(m_value.has_value());
	(*m_value) += c;
	return result_t::s_need_more;
}

result_t
string_parser::on_unicode(const char&c, const int pos)
{
	assert(m_value.has_value());
	(*m_value) += c;
	return result_t::s_need_more;
}

result_t
string_parser::on_done(const char&c, const int pos)
{
	return result_t::s_done;
}

result_t 
string_parser::on_fail(const char&c, const int pos)
{
	return result_t::e_unexpected;
}
