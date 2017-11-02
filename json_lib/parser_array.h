#ifndef __PARSER_ARRAY_H__
#define __PARSER_ARRAY_H__
namespace json
{
	enum class e_array_states
	{
		initial,	//
		val_before,	//
		val_inside,	//
		val_after,	//
		done,		//
		failure,	//
	};

	enum class e_array_events
	{
		arr_begin,	// [
		arr_end,	// ]
		val_done,	//
		val_error,	//
		comma,		// ,
		symbol,		// any symbol(depends on state)
		skip,		// space, tab, cr, lf
		nothing,	// no action event
	};

	class array_parser
		: public parser_impl<e_array_events, e_array_states, e_array_states::initial>
	{
		using event_t				= e_array_events;
		using state_t				= e_array_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= array_t;
	public:
		array_parser();

	protected:
		// Inherited via parser
		virtual void reset() final;
		virtual result_t putchar(const char& c, const int pos) final;
		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }
		virtual event_t to_event(const char& c) const override;
		virtual event_t to_event(const result_t& r) const override;
		
		// Own methods
		result_t on_begin(const unsigned char& c, const int pos);
		result_t on_new(const unsigned char& c, const int pos);
		result_t on_more(const unsigned& c, const int pos);
		result_t on_val(const unsigned& c, const int pos);
		result_t on_got_val(const char& c, const int pos);
		result_t on_done(const unsigned char& c, const int pos);
		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;
		
		parser::ptr m_value_parser;

		std::optional<array_t> m_value;
	};
}
#endif // __PARSER_ARRAY_H__