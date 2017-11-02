#pragma once
#ifndef __PARSER_VALUE_H__
#define __PARSER_VALUE_H__
namespace json
{
	enum class e_value_states
	{
		initial,
		read,
		done,
		failure,
	};

	enum class e_value_events
	{
		symbol = 0xffff,
		val_done,
		nothing,
	};

	class value_parser
		: public parser_impl<e_value_events, e_value_states, e_value_states::initial>
	{
		using event_t				= e_value_events;
		using state_t				= e_value_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using ParserItem_t			= std::pair<bool, parser::ptr>;
		using my_value_t			= value_t<>;
	public:
		value_parser();

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
		result_t on_data(const unsigned char& c, const int pos);
		result_t on_done(const unsigned char& c, const int pos);
		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;
		
		std::list<ParserItem_t> parsing_unit;
	};
}
#endif // __PARSER_VALUE_H__