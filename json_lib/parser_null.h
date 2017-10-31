#ifndef __PARSER_NULL_H__
#define __PARSER_NULL_H__
namespace json
{
	enum class e_null_states
	{
		initial,
		got_n,
		got_u,
		got_l,
		got_2nd_l,
		done,
		failure,
	};

	enum class e_null_events
	{
		// ascii part
		letter_n = 0x6e,	// n
		letter_u = 0x75,	// u
		letter_l = 0x6c,	// l
		other	 = 0xff,
	};

#ifdef _DEBUG
	template<>
	void state<e_null_states, e_null_states::initial>::set(e_null_states new_state)
	{
		auto state_2_string = [](e_null_states s)->std::string
		{
			switch (s)
			{
			case e_null_states::initial:	return std::string("initial");
			case e_null_states::got_n:		return std::string("got_n");
			case e_null_states::got_u:		return std::string("got_u");
			case e_null_states::got_l:		return std::string("got_l");
			case e_null_states::done:		return std::string("done");
			case e_null_states::failure:	return std::string("failure");
			}
			return std::string("unknown");
		};

#ifdef _DEBUG
		std::cout << "null parser:\t" << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;
#endif // _DEBUG
		m_state = new_state;
	}
#endif // _DEBUG

	class null_parser
		: public parser_impl<e_null_events, e_null_states, e_null_states::initial>
	{
		using event_t				= e_null_events;
		using state_t				= e_null_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= nullptr_t;
	public:
		null_parser();
		~null_parser();

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
		result_t on_n(const unsigned char& c, const int pos);
		result_t on_u(const unsigned char& c, const int pos);
		result_t on_l(const unsigned char& c, const int pos);
		result_t on_done(const unsigned char& c, const int pos);
		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		std::optional<my_value_t> m_value;
	};
}
#endif // __PARSER_NULL_H__