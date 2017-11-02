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
		array_parser()
			: m_value_parser(create_value_parser())
			, m_event_2_state_table
		{
			{ state_t::initial,		{	{ event_t::arr_begin,	{ state_t::val_before,	BIND(array_parser::on_begin)} },
										{ event_t::skip,		{ state_t::val_before,	BIND(array_parser::on_more)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
			} },
 			{ state_t::val_before,	{	{ event_t::symbol,		{ state_t::val_inside,	BIND(array_parser::on_val)	} },
										{ event_t::skip,		{ state_t::val_before,	BIND(array_parser::on_more)	} },
										{ event_t::arr_end,		{ state_t::done,		BIND(array_parser::on_done)	} },
 			} },
			{ state_t::val_after,	{	{ event_t::arr_end,		{ state_t::done,		BIND(array_parser::on_done)	} },
										{ event_t::comma,		{ state_t::val_before,	BIND(array_parser::on_new)	} },
										{ event_t::skip,		{ state_t::val_before,	BIND(array_parser::on_more)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
			} },
 			{ state_t::val_inside,	{	{ event_t::symbol,		{ state_t::val_inside,	BIND(array_parser::on_val)	} },
										{ event_t::val_done,	{ state_t::val_after,	BIND(array_parser::on_got_val) } },
										{ event_t::val_error,	{ state_t::failure,		BIND(array_parser::on_fail) } },
 			} },
			{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
			} },
			{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		BIND(array_parser::on_fail)	} },
			} },
		}{};

	protected:
		// Inherited via parser
		virtual void reset() final
		{
			state::set(state_t::initial);

			m_value_parser->reset();

			m_value.reset();
		};

		virtual result_t putchar(const char& c, const int pos) final
		{
			result_t r = parser_impl::step(to_event(c), c, pos);

			event_t e = to_event(r);

			if (event_t::nothing == e)
				return r;

			if (event_t::val_done == e)
			{
				result_t new_r = parser_impl::step(e, c, pos);
				r = result_t::s_need_more == new_r && result_t::s_done_rpt == r ?
					parser_impl::step(to_event(c), c, pos) :
					new_r;

				return r;
			}

			assert(0);

			return r;
		};
		
		virtual value get() const final
		{
			if (m_value.has_value())
				return *m_value;

			assert(0); // TODO: throw an exception
			return value();
		};

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }

		virtual event_t to_event(const char& c) const override
		{
			auto is_space = [](const char& c)->bool
			{
				// space, tab, cr, lf
				return (0x20 == c || 0x09 == c || 0x0A == c || 0x0D == c);
			};

			switch (state::get())
			{
			case state_t::initial:
				if (0x5B == c) //[
					return event_t::arr_begin;
				if (is_space(c))
					return event_t::skip;
				break;
			case state_t::val_before:
				if (0x5D == c) //]
					return event_t::arr_end;
				if (is_space(c))
					return event_t::skip;
				break;
			case state_t::val_after:
				if (0x5D == c) //]
					return event_t::arr_end;
				if (0x2C == c) //,
					return event_t::comma;
				if (is_space(c))
					return event_t::skip;
				break;
			}

			return event_t::symbol;
		};
		
		virtual event_t to_event(const result_t& r) const override
		{
			switch (state::get())
			{
			case state_t::val_inside:
				if (result_t::s_done == r || result_t::s_done_rpt == r)
					return event_t::val_done;
				break;
			}

			return event_t::nothing;
		};
		
		// Own methods
		result_t on_more(const unsigned& c, const int pos)
		{
			return result_t::s_need_more;
		}

		result_t on_begin(const unsigned char& c, const int pos)
		{
			if (!m_value.has_value())
				m_value.emplace();

			return result_t::s_need_more;
		}

		result_t on_new(const unsigned char& c, const int pos)
		{
			return result_t::s_need_more;
		}

		result_t on_val(const unsigned& c, const int pos)
		{
			return m_value_parser->putchar(c, pos);
		}

		result_t on_got_val(const char& c, const int pos)
		{
			assert(m_value.has_value());

			const value val = m_value_parser->get();

			(*m_value).push_back(val);

			m_value_parser->reset();

			return result_t::s_need_more;
		}

		result_t on_done(const unsigned char& c, const int pos)
		{
			assert(m_value.has_value());

			return result_t::s_done;
		}

		result_t on_fail(const unsigned char& c, const int pos)
		{
			return result_t::e_unexpected;
		}

	protected:
		const EventToStateTable_t m_event_2_state_table;
		
		parser::ptr m_value_parser;

		std::optional<array_t> m_value;
	};
}
#endif // __PARSER_ARRAY_H__