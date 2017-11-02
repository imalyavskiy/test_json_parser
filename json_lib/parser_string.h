#pragma once
#ifndef __PARSER_STRING_H__
#define __PARSER_STRING_H__
namespace json
{

	enum class e_string_states
	{
		initial,	// wait for " and skipping space charscters - space, hrisontal tab, crlf, lf
		inside,
		escape,
		cr,
		lf,
		unicode_1,
		unicode_2,
		unicode_3,
		unicode_4,
		done,
		failure,
	};

	enum class e_string_events
	{
		symbol,
		hex_digit,
		quote,
		back_slash,
		slash,
		alpha_b,	// stands for backspace
		alpha_f,	// stands for form feed
		alpha_n,	// stands for new line
		alpha_r,	// stands for carriage return
		alpha_t,	// stands for tab
		alpha_u,	// stands for unicode
	};

	class string_parser
		: public parser_impl<e_string_events, e_string_states, e_string_states::initial>
	{
	public:
		using event_t				= e_string_events;
		using state_t				= e_string_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= std::string;

		string_parser()
			: m_event_2_state_table
		{
			{ state_t::initial,		{	{ event_t::quote,		{ state_t::inside,		BIND(string_parser::on_initial)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::inside,		{	{ event_t::quote,		{ state_t::done,		BIND(string_parser::on_done)	} },
										{ event_t::back_slash,	{ state_t::escape,		BIND(string_parser::on_escape)	} },
										{ event_t::symbol,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
			} },
			{ state_t::escape,		{	{ event_t::quote,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::back_slash,	{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::slash,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::alpha_b,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::alpha_f,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::alpha_n,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::alpha_r,		{ state_t::cr,			BIND(string_parser::on_escape)	} },
										{ event_t::alpha_t,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::alpha_u,		{ state_t::unicode_1,	BIND(string_parser::on_unicode)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::cr,			{	{ event_t::back_slash,	{ state_t::lf,			BIND(string_parser::on_escape)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::lf,			{	{ event_t::alpha_n,		{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::unicode_1,	{	{ event_t::hex_digit,	{ state_t::unicode_2,	BIND(string_parser::on_unicode)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::unicode_2,	{	{ event_t::hex_digit,	{ state_t::unicode_3,	BIND(string_parser::on_unicode)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::unicode_3,	{	{ event_t::hex_digit,	{ state_t::unicode_4,	BIND(string_parser::on_unicode) } },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::unicode_4,	{	{ event_t::hex_digit,	{ state_t::inside,		BIND(string_parser::on_inside)	} },
										{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
			{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		BIND(string_parser::on_fail)	} },
			} },
		} {};

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
			event_t smb = event_t::symbol;
			switch (state::get())
			{
			case state_t::initial:
				if (0x22 == c)
					smb = event_t::quote;
				break;
			case state_t::inside:
				if (0x5C == c)
					smb = event_t::back_slash;
				if (0x22 == c)
					smb = event_t::quote;
				break;
			case state_t::escape:
				switch (c)
				{
				case 0x22:	smb = event_t::quote;		break;
				case 0x5C:	smb = event_t::back_slash;	break;
				case 0x2F:	smb = event_t::slash;		break;
				case 0x62:	smb = event_t::alpha_b;		break;
				case 0x66:	smb = event_t::alpha_f;		break;
				case 0x6E:	smb = event_t::alpha_n;		break;
				case 0x72:	smb = event_t::alpha_r;		break;
				case 0x74:	smb = event_t::alpha_t;		break;
				case 0x75:	smb = event_t::alpha_u;		break;
				}
				break;
			case state_t::cr:
				if (0x5C == c)
					return event_t::back_slash;
				break;
			case state_t::lf:
				if (0x6E == c)
					return event_t::alpha_n;
				break;
			case state_t::unicode_1:
			case state_t::unicode_2:
			case state_t::unicode_3:
			case state_t::unicode_4:
				if (0x30 <= c && c <= 0x39)
					smb = event_t::hex_digit;
				if (0x41 <= c && c <= 0x46)
					smb = event_t::hex_digit;
				if (0x61 <= c && c <= 0x66)
					smb = event_t::hex_digit;
				break;
			case state_t::done:
				assert(0);
				break;
			}

			return smb;
		};

		virtual event_t to_event(const result_t& c) const override
		{
			return event_t::symbol;
		};

		// Own methods
		result_t on_initial(const char&c, const int pos)
		{
			return result_t::s_need_more;
		}

		result_t on_inside(const char&c, const int pos)
		{
			if (!m_value.has_value())
				m_value.emplace();

			(*m_value) += c;

			return result_t::s_need_more;
		}

		result_t on_escape(const char&c, const int pos)
		{
			assert(m_value.has_value());
			(*m_value) += c;
			return result_t::s_need_more;
		}

		result_t on_unicode(const char&c, const int pos)
		{
			assert(m_value.has_value());
			(*m_value) += c;
			return result_t::s_need_more;
		}

		result_t on_done(const char&c, const int pos)
		{
			return result_t::s_done;
		}

		result_t on_fail(const char&c, const int pos)
		{
			return result_t::e_unexpected;
		}

	protected:
		const EventToStateTable_t m_event_2_state_table;

		 std::optional<my_value_t> m_value;
	};

}
#endif //__PARSER_STRING_H__