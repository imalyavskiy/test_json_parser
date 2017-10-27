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
	};

#ifdef _DEBUG
	template<>
	void state<e_value_states, e_value_states::initial>::set(e_value_states new_state)
	{
		auto state_2_string = [](e_value_states s)->std::string
		{
			switch (s)
			{
			case e_value_states::initial:	return std::string("initial");
			case e_value_states::read:		return std::string("read");
			case e_value_states::done:		return std::string("done");
			case e_value_states::failure:	return std::string("failure");
			}
			
			return std::string("unknown");
		};

		std::cout << "value parser:\t" << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;
		m_state = new_state;
	}
#endif // _DEBUG

	class value_parser
		: public parser_impl<e_value_events, e_value_states, e_value_states::initial>
	{
		using event_t				= e_value_events;
		using state_t				= e_value_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using ParserItem_t			= std::pair<bool, parser::ptr>;
	public:
		value_parser();
		~value_parser();

	protected:
		virtual result_t putchar(const char& c, const int pos) final;

		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }

		result_t on_data(const unsigned char& c, const int pos);
		result_t on_done(const unsigned char& c, const int pos);
		result_t on_fail(const unsigned char& c, const int pos);

		virtual event_t to_event(const char& c) const override;
		virtual event_t to_event(const result_t& c) const override;

		virtual void reset() final;

	protected:
		const EventToStateTable_t m_event_2_state_table;
		
		std::list<ParserItem_t> parsing_unit;
	};
}
#endif // __PARSER_VALUE_H__