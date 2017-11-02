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

	class null_parser
		: public parser_impl<e_null_events, e_null_states, e_null_states::initial>
	{
		using event_t				= e_null_events;
		using state_t				= e_null_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= nullptr_t;
	public:
		null_parser();

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