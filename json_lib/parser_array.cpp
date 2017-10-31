#include "parsers.h"

using namespace json;

array_parser::array_parser()
	: m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::arr_begin,	{ state_t::val_before,	BIND(array_parser::on_new)	} },
									{ event_t::skip,		{ state_t::val_before,	BIND(array_parser::on_more)	} },
									{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
		} },
 		{ state_t::val_before,	{	{ event_t::symbol,		{ state_t::val_inside,	BIND(array_parser::on_val)	} },
									{ event_t::skip,		{ state_t::val_before,	BIND(array_parser::on_more)	} },
									{ event_t::arr_end,		{ state_t::done,		BIND(array_parser::on_done)	} },
 		} },
		{ state_t::val_after,	{	{ event_t::arr_end,		{ state_t::done,		BIND(array_parser::on_done)	} },
									{ event_t::comma,		{ state_t::val_before,	BIND(array_parser::on_new)	} },
									{ event_t::skip,		{ state_t::val_before,	BIND(array_parser::on_more)	} },
									{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
		} },
 		{ state_t::val_inside,	{	{ event_t::symbol,		{ state_t::val_inside,	BIND(array_parser::on_val)	} },
									{ event_t::val_done,	{ state_t::val_after,	BIND(array_parser::on_more) } },
									{ event_t::val_error,	{ state_t::failure,		BIND(array_parser::on_fail) } },
 		} },
		{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
		} },
		{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
		} },
	}
	, m_value_parser(create_value_parser())
{
}

array_parser::~array_parser()
{
}

void 
array_parser::reset()
{
#ifdef _DEBUG
	std::cout << ">>> begin reset" << std::endl;
#endif // _DEBUG
	
	state::set(state_t::initial);
	
	m_value_parser->reset();

	m_value.reset();
	
#ifdef _DEBUG
	std::cout << ">>> end reset" << std::endl;
#endif // _DEBUG
}

value
array_parser::get() const 
{
	if (m_value.has_value())
		return *m_value;

	assert(0); // TODO: throw an exception
	return value();
}

array_parser::event_t
array_parser::to_event(const char& c) const
{
	auto is_space = [](const char& c)->bool
	{
		// space, tab, cr, lf
		return (0x20 == c || 0x09 == c || 0x0A == c || 0x0D == c);
	};

	switch (state::get())
	{
	case state_t::initial:
		if (0x5B == c) //[
			return event_t::arr_begin;
		if (is_space(c))
			return event_t::skip;
		break;
	case state_t::val_before:
		if (0x5D == c) //]
			return event_t::arr_end;
		if (is_space(c))
			return event_t::skip;
		break;
	case state_t::val_after:
		if (0x5D == c) //]
			return event_t::arr_end;
		if (0x2C == c) //,
			return event_t::comma;
		if(is_space(c))
			return event_t::skip;
		break;
	}

	return event_t::symbol;
}

array_parser::event_t
array_parser::to_event(const result_t& r) const
{
	switch (state::get())
	{
	case state_t::val_inside:
		if (result_t::s_done == r || result_t::s_done_rpt == r)
			return event_t::val_done;
		break;
	}

	return event_t::nothing;
}

result_t 
array_parser::putchar(const char& c, const int pos)
{
	result_t r = parser_impl::step(to_event(c), c, pos);

	event_t e = to_event(r);

	if (event_t::nothing == e)
		return r;

	if (event_t::val_done == e)
	{
		result_t new_r = parser_impl::step(e, c, pos);
		r = result_t::s_need_more == new_r && result_t::s_done_rpt == r ?
			parser_impl::step(to_event(c), c, pos) :
			new_r;
		
		return r;
	}

	assert(0);

	return r;
}

result_t
array_parser::on_more(const unsigned& c, const int pos)
{
	return result_t::s_need_more;
}

result_t
array_parser::on_new(const unsigned char& c, const int pos)
{
	if (!m_value.has_value())
		m_value.emplace();

	const value val = m_value_parser->get();

	(*m_value).push_back(val);

	m_value_parser->reset();
	return result_t::s_need_more;
}

result_t 
array_parser::on_val(const unsigned& c, const int pos)
{
	return m_value_parser->putchar(c, pos);
}

result_t
array_parser::on_done(const unsigned char& c, const int pos)
{
	return result_t::s_done;
}

result_t 
array_parser::on_fail(const unsigned char& c, const int pos)
{
	return result_t::e_unexpected;
}
