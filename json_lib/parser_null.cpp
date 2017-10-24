#include "parsers.h"

using namespace json;

null_parser::null_parser()
	: m_state_table
	{
		{ read_state_t::initial,	{	{ symbol_t::letter_n,				{ read_state_t::got_n,	BIND(null_parser::on_n)		} },
		} },
		{ read_state_t::got_n,		{	{ symbol_t::letter_u,				{ read_state_t::got_u,	BIND(null_parser::on_u)		} },
		} },
		{ read_state_t::got_u,		{	{ symbol_t::letter_l,				{ read_state_t::got_l,	BIND(null_parser::on_l)		} },
		} },
		{ read_state_t::got_l,		{	{ symbol_t::letter_l,				{ read_state_t::done,	BIND(null_parser::on_done)	} },
		} },
	}
{
	reset();
}

null_parser::~null_parser()
{
}

result 
null_parser::on_n(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
null_parser::on_u(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
null_parser::on_l(const unsigned char& c, const int pos)
{
	return result::s_need_more;
}

result 
null_parser::on_done(const unsigned char& c, const int pos)
{
	return result::s_done;
}

void 
null_parser::reset()
{
	state::set(read_state_t::initial);
}