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
		other			= 0xffff,
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

#ifdef _DEBUG
	template<>
	void state<e_string_states, e_string_states::initial>::set(e_string_states new_state)
	{
		auto state_2_string = [](e_string_states s)->std::string
		{
			switch (s)
			{
			case e_string_states::initial:		return std::string("initial");
			case e_string_states::inside:		return std::string("inside");
			case e_string_states::escape:		return std::string("escape");
			case e_string_states::cr:			return std::string("cr");
			case e_string_states::lf:			return std::string("lf");
			case e_string_states::unicode_1:	return std::string("unicode_1");
			case e_string_states::unicode_2:	return std::string("unicode_2");
			case e_string_states::unicode_3:	return std::string("unicode_3");
			case e_string_states::unicode_4:	return std::string("unicode_4");
			case e_string_states::done:			return std::string("done");
			case e_string_states::failure:		return std::string("failure");
			}
			
			return std::string("unknown");
		};

		std::cout << "string parser:\t" << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;
		m_state = new_state;
	}
#endif // _DEBUG

	class string_parser
		: public parser_impl<e_string_events, e_string_states, e_string_states::initial>
	{
	public:
		using event_t				= e_string_events;
		using state_t				= e_string_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;

		string_parser();
		~string_parser();

	protected:
		virtual result step(const char& c, const int pos) final;

		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }

		result on_initial(const char&c, const int pos);
		result on_inside(const char&c, const int pos);
		result on_escape(const char&c, const int pos);
		result on_unicode(const char&c, const int pos);
		result on_done(const char&c, const int pos);
		result on_fail(const char&c, const int pos);

		virtual event_t to_event(const char& c) const override;

		virtual void reset() final;

	protected:
		const EventToStateTable_t m_event_2_state_table;
	};

}
#endif //__PARSER_STRING_H__