#include "parsers.h"

using namespace json;

null_parser::null_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::letter_n,				{ state_t::got_n,	BIND(null_parser::on_n)		} },
									{ event_t::other,					{ state_t::failure,	BIND(null_parser::on_fail)	} },
		} },
		{ state_t::got_n,		{	{ event_t::letter_u,				{ state_t::got_u,	BIND(null_parser::on_u)		} },
									{ event_t::other,					{ state_t::failure,	BIND(null_parser::on_fail)	} },
		} },
		{ state_t::got_u,		{	{ event_t::letter_l,				{ state_t::got_l,	BIND(null_parser::on_l)		} },
									{ event_t::other,					{ state_t::failure,	BIND(null_parser::on_fail)	} },
		} },
		{ state_t::got_l,		{	{ event_t::letter_l,				{ state_t::done,	BIND(null_parser::on_done)	} },
									{ event_t::other,					{ state_t::failure,	BIND(null_parser::on_fail)	} },
		} },
		{ state_t::done,		{	{ event_t::other,					{ state_t::failure,	BIND(null_parser::on_fail)	} },
		} },
		{ state_t::failure,		{	{ event_t::other,					{ state_t::failure,	BIND(null_parser::on_fail)	} },
		} },
	}
{
}

null_parser::~null_parser()
{
}

result_t
null_parser::putchar(const char& c, const int pos)
{
	return parser_impl::step(to_event(c), c, pos);
}


result_t 
null_parser::on_n(const unsigned char& c, const int pos)
{
	return result_t::s_need_more;
}

result_t 
null_parser::on_u(const unsigned char& c, const int pos)
{
	return result_t::s_need_more;
}

result_t 
null_parser::on_l(const unsigned char& c, const int pos)
{
	return result_t::s_need_more;
}

result_t 
null_parser::on_done(const unsigned char& c, const int pos)
{
	return result_t::s_done;
}

result_t 
null_parser::on_fail(const unsigned char& c, const int pos)
{
	return result_t::e_unexpected;
}

void 
null_parser::reset()
{
	state::set(state_t::initial);
}

null_parser::event_t
null_parser::to_event(const char& c) const
{
	switch (c)
	{
	case 0x6e:
		return event_t::letter_n;
	case 0x75:
		return event_t::letter_u;
	case 0x6c:
		return event_t::letter_l;
	}

	return event_t::other;
}

null_parser::event_t
null_parser::to_event(const result_t& c) const
{
	return event_t::other;
}
