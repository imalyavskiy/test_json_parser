#include "parsers.h"

using namespace json;

object_parser::object_parser()
	: m_event_2_state_table
	{
		{ e_object_states::initial,		{		{ e_object_events::left_curly_brace,		{ e_object_states::before_key,	BIND(object_parser::on_before_val)	} },
												{ e_object_events::other,					{ e_object_states::failure,		BIND(object_parser::on_fail)		} },
		} },
		{ e_object_states::before_key,	{		{ e_object_events::right_curly_brace,		{ e_object_states::done,		BIND(object_parser::on_done)		} },
												{ e_object_events::other,					{ e_object_states::failure,		BIND(object_parser::on_fail)		} },
		} },
		{ e_object_states::done,		{		{ e_object_events::other,					{ e_object_states::failure,		BIND(object_parser::on_fail)		} },
		} },
		{ e_object_states::failure,		{		{ e_object_events::other,					{ e_object_states::failure,		BIND(object_parser::on_fail)		} },
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
	if (0x7B == c)		// {
		return event_t::left_curly_brace;
	if (0x7D == c)		// }
		return event_t::right_curly_brace;

	return event_t::other;
};

result
object_parser::step(const char& c, const int pos)
{
	return parser_impl::step(c, pos);
}

result
object_parser::on_initial(const char& c, const int pos)
{
	return result::s_need_more;
}

result
object_parser::on_before_val(const char& c, const int pos)
{
	return result::s_need_more;
}

result
object_parser::on_done(const char& c, const int pos)
{
	return result::s_done;
}

result
object_parser::on_fail(const char& c, const int pos)
{
	return result::e_unexpected;
}

void 
object_parser::reset()
{
	std::cout << ">>> begin reset" << std::endl;
	state::set(state_t::initial);
	std::cout << ">>> end reset" << std::endl;
}
