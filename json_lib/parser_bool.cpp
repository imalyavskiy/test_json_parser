#include "parsers.h"

using namespace json;

bool_parser::bool_parser()
	: m_event_2_state_table
{	// true
	{ state_t::initial,	{	{ event_t::letter_t,{ state_t::got_t,	BIND(bool_parser::on_t) } },
							{ event_t::letter_f,{ state_t::got_f,	BIND(bool_parser::on_f) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	{ state_t::got_t,	{	{ event_t::letter_r,{ state_t::got_r,	BIND(bool_parser::on_r) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	{ state_t::got_r,	{	{ event_t::letter_u,{ state_t::got_u,	BIND(bool_parser::on_u) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	{ state_t::got_u,	{	{ event_t::letter_e,{ state_t::done,	BIND(bool_parser::on_done) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	// false
	{ state_t::got_f,	{	{ event_t::letter_a,{ state_t::got_a,	BIND(bool_parser::on_a) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	{ state_t::got_a,	{	{ event_t::letter_l,{ state_t::got_l,	BIND(bool_parser::on_l) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	{ state_t::got_l,	{	{ event_t::letter_s,{ state_t::got_s,	BIND(bool_parser::on_s) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	{ state_t::got_s,	{	{ event_t::letter_e,{ state_t::done,	BIND(bool_parser::on_done) } },
							{ event_t::other,	{ state_t::failure,	BIND(bool_parser::on_fail) } },
	} },
	{ state_t::done,	{	{ event_t::other,	{ state_t::failure, BIND(bool_parser::on_fail) } },
	} },
	{ state_t::failure,	{	{ event_t::other,	{ state_t::failure, BIND(bool_parser::on_fail) } },
	} },
}
{
}

bool_parser::~bool_parser()
{
}

result 
bool_parser::step(const char& c, const int pos)
{
	return parser_impl::step(c, pos);
}

result 
bool_parser::on_t(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
bool_parser::on_r(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
bool_parser::on_u(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
bool_parser::on_f(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
bool_parser::on_a(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
bool_parser::on_l(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
bool_parser::on_s(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result
bool_parser::on_done(const unsigned char& c, const int pos)
{
	return result::s_done;
}

result 
bool_parser::on_fail(const unsigned char& c, const int pos)
{
	return result::e_unexpected;
}

void
bool_parser::reset()
{
	state::set(state_t::initial);
}

bool_parser::event_t 
bool_parser::to_event(const char& c) const
{
	switch (c)
	{
	case 0x74:
		return event_t::letter_t;
	case 0x72:
		return event_t::letter_r;
	case 0x75:
		return event_t::letter_u;
	case 0x65:
		return event_t::letter_e;
	case 0x66:
		return event_t::letter_f;
	case 0x61:
		return event_t::letter_a;
	case 0x6c:
		return event_t::letter_l;
	case 0x73:
		return event_t::letter_s;
	}
	return event_t::other;
}
