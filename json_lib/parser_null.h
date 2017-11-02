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
		letter_n,	// n
		letter_u,	// u
		letter_l,	// l
		other	,
	};

	class null_parser
		: public parser_impl<e_null_events, e_null_states, e_null_states::initial>
	{
		using event_t				= e_null_events;
		using state_t				= e_null_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= nullptr_t;
	public:
		null_parser()
			: m_event_2_state_table
		{
			{ state_t::initial,	{	{ event_t::letter_n,{ state_t::got_n,	BIND(null_parser::on_n)		} },
			{ event_t::other,							{ state_t::failure,	BIND(null_parser::on_fail)	} },
			} },
			{ state_t::got_n,	{	{ event_t::letter_u,{ state_t::got_u,	BIND(null_parser::on_u)		} },
			{ event_t::other,							{ state_t::failure,	BIND(null_parser::on_fail)	} },
			} },
			{ state_t::got_u,	{	{ event_t::letter_l,{ state_t::got_l,	BIND(null_parser::on_l)		} },
			{ event_t::other,							{ state_t::failure,	BIND(null_parser::on_fail)	} },
			} },
			{ state_t::got_l,	{	{ event_t::letter_l,{ state_t::done,	BIND(null_parser::on_done)	} },
			{ event_t::other,							{ state_t::failure,	BIND(null_parser::on_fail)	} },
			} },
			{ state_t::done,	{	{ event_t::other,	{ state_t::failure,	BIND(null_parser::on_fail)	} },
			} },
			{ state_t::failure,	{	{ event_t::other,	{ state_t::failure,	BIND(null_parser::on_fail)	} },
			} },
		}
		{};

	protected:
		// Inherited via parser
		virtual void reset() final
		{
			state::set(state_t::initial);
			m_value.reset();
		};

		virtual result_t putchar(const char& c, const int pos) final
		{
			return parser_impl::step(to_event(c), c, pos);
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
			switch (c)
			{
			case 0x6e:
				return event_t::letter_n;
			case 0x75:
				return event_t::letter_u;
			case 0x6c:
				return event_t::letter_l;
			}

			return event_t::other;
		};

		virtual event_t to_event(const result_t& c) const override
		{
			return event_t::other;
		};

		// Own methods
		result_t on_n(const unsigned char& c, const int pos)
		{
			return result_t::s_need_more;
		};

		result_t on_u(const unsigned char& c, const int pos)
		{
			return result_t::s_need_more;
		};

		result_t on_l(const unsigned char& c, const int pos)
		{
			return result_t::s_need_more;
		};

		result_t on_done(const unsigned char& c, const int pos)
		{
			if (!m_value.has_value())
				m_value.emplace();

			(*m_value) = nullptr;

			return result_t::s_done;
		};

		result_t on_fail(const unsigned char& c, const int pos)
		{
			return result_t::e_unexpected;
		};

	protected:
		const EventToStateTable_t m_event_2_state_table;

		std::optional<my_value_t> m_value;
	};
}
#endif // __PARSER_NULL_H__