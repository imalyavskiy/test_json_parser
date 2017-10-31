#include "parsers.h"

using namespace json;

bool_parser::bool_parser()
	: m_event_2_state_table
{
	{ state_t::initial,	{	{ event_t::letter_t,{ state_t::got_t,	BIND(bool_parser::on_t)		} },
							{ event_t::letter_f,{ state_t::got_f,	BIND(bool_parser::on_f)		} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::got_t,	{	{ event_t::letter_r,{ state_t::got_r,	BIND(bool_parser::on_r)		} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::got_r,	{	{ event_t::letter_u,{ state_t::got_u,	BIND(bool_parser::on_u)		} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::got_u,	{	{ event_t::letter_e,{ state_t::done,	BIND(bool_parser::on_done)	} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::got_f,	{	{ event_t::letter_a,{ state_t::got_a,	BIND(bool_parser::on_a)		} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::got_a,	{	{ event_t::letter_l,{ state_t::got_l,	BIND(bool_parser::on_l)		} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::got_l,	{	{ event_t::letter_s,{ state_t::got_s,	BIND(bool_parser::on_s)		} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::got_s,	{	{ event_t::letter_e,{ state_t::done,	BIND(bool_parser::on_done)	} },
							{ event_t::symbol,	{ state_t::failure,	BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::done,	{	{ event_t::symbol,	{ state_t::failure, BIND(bool_parser::on_fail)	} },
	} },
	{ state_t::failure,	{	{ event_t::symbol,	{ state_t::failure, BIND(bool_parser::on_fail)	} },
	} },
}
{
}

bool_parser::~bool_parser()
{
}

void
bool_parser::reset()
{
	state::set(state_t::initial);
	m_str.clear();
	m_value.reset();
}

result_t 
bool_parser::putchar(const char& c, const int pos)
{
	return parser_impl::step(to_event(c), c, pos);
}

value
bool_parser::get() const
{
	if (m_value.has_value())
		return *m_value;

	assert(0); // TODO: throw an exception
	return value();
}

bool_parser::event_t 
bool_parser::to_event(const char& c) const
{
	switch (c)
	{
	case 0x61:
		return event_t::letter_a;
	case 0x65:
		return event_t::letter_e;
	case 0x66:
		return event_t::letter_f;
	case 0x6c:
		return event_t::letter_l;
	case 0x72:
		return event_t::letter_r;
	case 0x73:
		return event_t::letter_s;
	case 0x74:
		return event_t::letter_t;
	case 0x75:
		return event_t::letter_u;
	}
	return event_t::symbol;
}

bool_parser::event_t
bool_parser::to_event(const result_t& c) const
{
	return event_t::symbol;
}

result_t 
bool_parser::on_t(const unsigned char& c, const int pos)
{
	m_str += c;
	return result_t::s_need_more;
}

result_t 
bool_parser::on_r(const unsigned char& c, const int pos)
{
	m_str += c;
	return result_t::s_need_more;
}

result_t 
bool_parser::on_u(const unsigned char& c, const int pos)
{
	m_str += c;
	return result_t::s_need_more;
}

result_t 
bool_parser::on_f(const unsigned char& c, const int pos)
{
	m_str += c;
	return result_t::s_need_more;
}

result_t 
bool_parser::on_a(const unsigned char& c, const int pos)
{
	m_str += c;
	return result_t::s_need_more;
}

result_t 
bool_parser::on_l(const unsigned char& c, const int pos)
{
	m_str += c;
	return result_t::s_need_more;
}

result_t 
bool_parser::on_s(const unsigned char& c, const int pos)
{
	m_str += c;
	return result_t::s_need_more;
}

result_t
bool_parser::on_done(const unsigned char& c, const int pos)
{
	m_str += c;

	auto update = [this](const bool val)->result_t
	{
		if (!m_value.has_value())
			m_value.emplace();
		
		(*m_value) = val;
	
		return result_t::s_done;
	};

	if (m_str == "true")
		return update(true);
	if (m_str == "false")
		return update(false);

	assert(0);

	return result_t::e_unexpected;
}

result_t 
bool_parser::on_fail(const unsigned char& c, const int pos)
{
	return result_t::e_unexpected;
}
