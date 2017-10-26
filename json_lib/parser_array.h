#ifndef __PARSER_ARRAY_H__
#define __PARSER_ARRAY_H__
namespace json
{
	enum class e_array_states
	{
		initial,
		before_val,
		in_value,
		after_val,
		done,
		failure,
	};

	enum class e_array_events
	{
		// ascii part
		other					= 0xffff,
		left_square_bracket		= 0x005B,
		right_square_bracket	= 0x005D,
		comma					= 0x00,
	};

#ifdef _DEBUG
	template<>
	void state<e_array_states, e_array_states::initial>::set(e_array_states new_state)
	{
		auto state_2_string = [](e_array_states s)->std::string
		{
			switch (s)
			{
			case e_array_states::initial:		return std::string("initial");
			case e_array_states::before_val:	return std::string("before_val");
			case e_array_states::in_value:		return std::string("in_value");
			case e_array_states::after_val:		return std::string("after_val");
			case e_array_states::done:			return std::string("done");
			case e_array_states::failure:		return std::string("failure");
			}

			return std::string("unknown");
		};

		std::cout << "array parser:\t" << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;
		m_state = new_state;
	}
#endif // _DEBUG

	class array_parser
		: public parser_impl<e_array_events, e_array_states, e_array_states::initial>
	{
		using event_t				= e_array_events;
		using state_t				= e_array_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
	public:
		array_parser();
		~array_parser();

	protected:

		virtual result step(const char& c, const int pos) final;

		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }
		
		result on_before_value(const unsigned char& c, const int pos);
		result on_after_value(const unsigned& c, const int pos);
		result on_value(const unsigned& c, const int pos);
		result on_done(const unsigned char& c, const int pos);
		result on_fail(const unsigned char& c, const int pos);

		virtual event_t to_event(const char& c) const override;

		virtual void reset() final;

	protected:
		const EventToStateTable_t m_event_2_state_table;
		
		parser::ptr m_value_parser;
	};
}
#endif // __PARSER_ARRAY_H__