#ifndef __PARSER_BOOL_H__
#define __PARSER_BOOL_H__
namespace json
{
	enum class e_bool_states
	{
		initial,
		got_t,
		got_r,
		got_u,
		got_f,
		got_a,
		got_l,
		got_s,
		done,
		failure,
	};

	enum class e_bool_events
	{
		// ascii part
		letter_a = 0x61,	// a
		letter_e = 0x65,	// e
		letter_f = 0x66,	// f 
		letter_l = 0x6c,	// l
		letter_r = 0x72,	// r
		letter_s = 0x73,	// s
		letter_t = 0x74,	// t
		letter_u = 0x75,	// u
		other = 0xff,
	};

#ifdef _DEBUG
	template<>
	void state<e_bool_states, e_bool_states::initial>::set(e_bool_states new_state)
	{
		auto state_2_string = [](e_bool_states s)->std::string
		{
			switch (s)
			{
			case e_bool_states::initial:	return std::string("initial");
			case e_bool_states::got_t:		return std::string("got_t");
			case e_bool_states::got_r:		return std::string("got_r");
			case e_bool_states::got_u:		return std::string("got_u");
			case e_bool_states::got_f:		return std::string("got_f");
			case e_bool_states::got_a:		return std::string("got_a");
			case e_bool_states::got_l:		return std::string("got_l");
			case e_bool_states::got_s:		return std::string("got_s");
			case e_bool_states::done:		return std::string("done");
			case e_bool_states::failure:	return std::string("failure");
			}

			return std::string("unknown");
		};

		std::cout << "bool parser:\t" << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;
		m_state = new_state;
	}
#endif // _DEBUG

	class bool_parser
		: public parser_impl<e_bool_events, e_bool_states, e_bool_states::initial>
	{
		using event_t		= e_bool_events;
		using state_t		= e_bool_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
	public:
		bool_parser();
		~bool_parser();

	protected:
		virtual result step(const char& c, const int pos) final;

		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }

		result on_t(const unsigned char& c, const int pos);
		result on_r(const unsigned char& c, const int pos);
		result on_u(const unsigned char& c, const int pos);
		result on_f(const unsigned char& c, const int pos);
		result on_a(const unsigned char& c, const int pos);
		result on_l(const unsigned char& c, const int pos);
		result on_s(const unsigned char& c, const int pos);
		result on_done(const unsigned char& c, const int pos);
		result on_fail(const unsigned char& c, const int pos);

		virtual event_t to_event(const char& c) const override;

		virtual void reset() final;

	protected:
		const EventToStateTable_t m_event_2_state_table;
	};
}
#endif // __PARSER_BOOL_H__