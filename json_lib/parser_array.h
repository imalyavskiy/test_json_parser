#ifndef __PARSER_ARRAY_H__
#define __PARSER_ARRAY_H__
namespace json
{
	enum class e_array_states
	{
		initial,
		val_before,
		val_inside,
		val_after,
		done,
		failure,
	};

	enum class e_array_events
	{
		// ascii part
		arr_begin	= 0x005B,
		arr_end		= 0x005D,
		val_done,	//
		val_error,	//
		comma		= 0x00,
		symbol		= 0xffff,
		skip,		// space, tab, cr, lf
		nothing,	// no action event
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
			case e_array_states::val_before:	return std::string("val_before");
			case e_array_states::val_inside:	return std::string("val_inside");
			case e_array_states::val_after:		return std::string("val_after");
			case e_array_states::done:			return std::string("done");
			case e_array_states::failure:		return std::string("failure");
			}

			return std::string("unknown");
		};

		std::cout << "array parser(0x" << std::hex << std::setw(8) << std::setfill('0') << this << std::resetiosflags(std::ios_base::basefield) << "):\t";
		std::cout << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;

		m_state = new_state;
	}
#endif // _DEBUG

	class array_parser
		: public parser_impl<e_array_events, e_array_states, e_array_states::initial>
	{
		using event_t				= e_array_events;
		using state_t				= e_array_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= array_t;
	public:
		array_parser();
		~array_parser();

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