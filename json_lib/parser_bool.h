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
		symbol = 0xff,
	};

	class bool_parser
		: public parser_impl<e_bool_events, e_bool_states, e_bool_states::initial>
	{
		using event_t				= e_bool_events;
		using state_t				= e_bool_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= bool;
	public:
		bool_parser();

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
		result_t on_t(const unsigned char& c, const int pos);
		result_t on_r(const unsigned char& c, const int pos);
		result_t on_u(const unsigned char& c, const int pos);
		result_t on_f(const unsigned char& c, const int pos);
		result_t on_a(const unsigned char& c, const int pos);
		result_t on_l(const unsigned char& c, const int pos);
		result_t on_s(const unsigned char& c, const int pos);
		result_t on_done(const unsigned char& c, const int pos);
		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		std::string m_str;

		std::optional<my_value_t> m_value;
	};
}
#endif // __PARSER_BOOL_H__