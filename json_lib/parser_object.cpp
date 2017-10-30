#include "parsers.h"

using namespace json;

object_parser::object_parser()
	: m_key_parser(create_string_parser())
	, m_val_parser(create_value_parser())
	, m_event_2_state_table
	{
		{ state_t::initial,		{	{ event_t::obj_begin,	{ state_t::key_before,	BIND(object_parser::on_more) } },
									{ event_t::skip,		{ state_t::val_before,	BIND(object_parser::on_more) } },
									{ event_t::symbol,		{ state_t::failure,		BIND(object_parser::on_fail) } },
		} },
		{ state_t::key_before,	{	{ event_t::obj_end,		{ state_t::done,		BIND(object_parser::on_done) } },
									{ event_t::key_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
									{ event_t::symbol,		{ state_t::key_inside,	BIND(object_parser::on_key ) } },
									{ event_t::skip,		{ state_t::key_before,	BIND(object_parser::on_more) } },
		} },
		{ state_t::key_inside,	{	{ event_t::key_done,	{ state_t::key_after,	BIND(object_parser::on_more) } },
									{ event_t::key_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
									{ event_t::symbol,		{ state_t::key_inside,	BIND(object_parser::on_key ) } },
		} },
		{ state_t::key_after,	{	{ event_t::colon,		{ state_t::val_before,	BIND(object_parser::on_more) } },
									{ event_t::skip,		{ state_t::key_after,	BIND(object_parser::on_more) } },
		} },
		{ state_t::val_before,	{	{ event_t::symbol,		{ state_t::val_inside,	BIND(object_parser::on_val ) } },
									{ event_t::val_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
									{ event_t::skip,		{ state_t::val_before,	BIND(object_parser::on_more) } },
		} },
		{ state_t::val_inside,	{	{ event_t::val_done,	{ state_t::val_after,	BIND(object_parser::on_more) } },
									{ event_t::val_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
									{ event_t::symbol,		{ state_t::val_inside,	BIND(object_parser::on_val ) } },
		} },
		{ state_t::val_after,	{	{ event_t::comma,		{ state_t::key_before,	BIND(object_parser::on_new ) } },
									{ event_t::obj_end,		{ state_t::done,		BIND(object_parser::on_done) } },
									{ event_t::skip,		{ state_t::val_after,	BIND(object_parser::on_more) } },
		} },
		{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		BIND(object_parser::on_fail) } },
		} },
		{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		BIND(object_parser::on_fail) } },
		} },
	}
{
}

object_parser::~object_parser()
{
}

// char to token name
object_parser::event_t
object_parser::to_event(const char& c) const
{
	auto is_space = [](const char& c)->bool
	{
		// space, tab, cr, lf
		return (0x20 == c || 0x09 == c || 0x0A == c || 0x0D == c);
	};

	switch(state::get())
	{
		case state_t::initial:
			if (0x7B == c)	//{
				return event_t::obj_begin;
			if(is_space(c)) 
				return event_t::skip;
			break;
		case state_t::key_before:
			if (0x7D == c)	//}
				return event_t::obj_end;
			if (is_space(c))
				return event_t::skip;
			break;
		case state_t::key_after:
			if (is_space(c))
				return event_t::skip;
			if (0x3A == c)	//:
				return event_t::colon;
			break;
		case state_t::val_before:
			if (is_space(c))
				return event_t::skip;
			break;
		case state_t::val_after:
			if (0x7D == c)	//}
				return event_t::obj_end;
			if (is_space(c))
				return event_t::skip;
			if (0x2c == c)	//,
				return event_t::comma;
			break;
	}

	return event_t::symbol;
};

object_parser::event_t
object_parser::to_event(const result_t& r) const
{
	switch (state::get())
	{
	case state_t::key_inside:
		if (result_t::s_done == r)
			return event_t::key_done;
		break;
	case state_t::val_inside:
		if (result_t::s_done == r || result_t::s_done_rpt == r)
			return event_t::val_done;
		break;
	}

	return event_t::nothing;
};

result_t
object_parser::putchar(const char& c, const int pos)
{
	result_t r = parser_impl::step(to_event(c), c, pos);
	
	event_t e = to_event(r);

	if (event_t::nothing == e)
		return r;

	if (event_t::val_done == e || event_t::key_done == e)
	{
		result_t new_r = parser_impl::step(e, c, pos);
		r = result_t::s_need_more == new_r && result_t::s_done_rpt == r? 
			parser_impl::step(to_event(c), c, pos) : 
			new_r;

		return r;
	}

	assert(0);
	
	return r;
}

result_t
object_parser::on_more(const char& c, const int pos)
{
	return result_t::s_need_more;
}

result_t 
object_parser::on_new(const char& c, const int pos)
{
	reset();
	return result_t::s_need_more;
}

result_t
object_parser::on_key(const char& c, const int pos)
{
	return m_key_parser->putchar(c, pos);
}

result_t
object_parser::on_val(const char& c, const int pos)
{
	return m_val_parser->putchar(c, pos);
}

result_t
object_parser::on_done(const char& c, const int pos)
{
	reset();
	return result_t::s_done;
}

result_t
object_parser::on_fail(const char& c, const int pos)
{
	return result_t::e_unexpected;
}

void 
object_parser::reset()
{
#ifdef _DEBUG
	std::cout << ">>> begin reset" << std::endl;
#endif // _DEBUG
	
	state::set(state_t::initial);
	
	m_key_parser->reset();
	m_val_parser->reset();
	
#ifdef _DEBUG
	std::cout << ">>> end reset" << std::endl;
#endif // _DEBUG
}
