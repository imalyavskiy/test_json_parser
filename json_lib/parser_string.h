#pragma once
#ifndef __PARSER_STRING_H__
#define __PARSER_STRING_H__
namespace json
{

	enum class e_string_states
	{
		initial,	// wait for " and skipping space charscters - space, hrisontal tab, crlf, lf
		inside,
		escape,
		cr,
		lf,
		unicode_1,
		unicode_2,
		unicode_3,
		unicode_4,
		done,
		failure,
	};

	enum class e_string_events
	{
		// ascii part
		symbol			= 0xffff,
		hex_digit		= 0xfffe,
		// ...
		quote			= 0x22,
		back_slash		= 0x5c,
		slash			= 0x2F,
		// ...
		numeric_0		= 0x30,
		// 0x31 - 0x38
		numeric_9		= 0x39,
		// ...
		alpha_A			= 0x41,
		// 0x42 - 0x45
		alpha_F			= 0x46,
		// ...
		alpha_a			= 0x61,
		alpha_b			= 0x62,
		// 0x63 - 0x65
		alpha_f			= 0x66,
		// ...
		alpha_n			= 0x6E,
		// ...
		alpha_r			= 0x72,
		// ...
		alpha_t			= 0x74,
		alpha_u			= 0x75,
	};

	class string_parser
		: public parser_impl<e_string_events, e_string_states, e_string_states::initial>
	{
	public:
		using event_t				= e_string_events;
		using state_t				= e_string_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= std::string;

		string_parser();

	protected:
		// Inherited via parser
		virtual void reset() final;
		virtual result_t putchar(const char& c, const int pos) final;
		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }
		virtual event_t to_event(const char& c) const override;
		virtual event_t to_event(const result_t& c) const override;

		// Own methods
		result_t on_initial(const char&c, const int pos);
		result_t on_inside(const char&c, const int pos);
		result_t on_escape(const char&c, const int pos);
		result_t on_unicode(const char&c, const int pos);
		result_t on_done(const char&c, const int pos);
		result_t on_fail(const char&c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		 std::optional<my_value_t> m_value;
	};

}
#endif //__PARSER_STRING_H__