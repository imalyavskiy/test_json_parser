#include "parsers.h"

using namespace json;

bool_parser::bool_parser()
	: m_state_table
{	// true
	{ read_state_t::initial,{ { symbol_t::letter_t,{ read_state_t::got_t,	BIND(bool_parser::on_t) } },
	} },
	{ read_state_t::got_t,	{ { symbol_t::letter_r,{ read_state_t::got_r,	BIND(bool_parser::on_r) } },
	} },
	{ read_state_t::got_r,	{ { symbol_t::letter_u,{ read_state_t::got_u,	BIND(bool_parser::on_u) } },
	} },
	{ read_state_t::got_u,	{ { symbol_t::letter_e,{ read_state_t::done,	BIND(bool_parser::on_done) } },
	} },
	// false
	{ read_state_t::initial,{ { symbol_t::letter_f,{ read_state_t::got_f,	BIND(bool_parser::on_f) } },
	} },
	{ read_state_t::got_f,	{ { symbol_t::letter_a,{ read_state_t::got_a,	BIND(bool_parser::on_a) } },
	} },
	{ read_state_t::got_a,	{ { symbol_t::letter_l,{ read_state_t::got_l,	BIND(bool_parser::on_l) } },
	} },
	{ read_state_t::got_l,	{ { symbol_t::letter_s,{ read_state_t::got_s,	BIND(bool_parser::on_s) } },
	} },
	{ read_state_t::got_s,	{ { symbol_t::letter_e,{ read_state_t::done,	BIND(bool_parser::on_done) } },
	} },
}
{
	reset();
}

bool_parser::~bool_parser()
{
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

void
bool_parser::reset()
{
	state::set(read_state_t::initial);
}