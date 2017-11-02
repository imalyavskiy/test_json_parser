#pragma once
#ifndef __PARSER_NUMBER_H__
#define __PARSER_NUMBER_H__
namespace json
{
	enum class e_number_states
	{
		initial,
		leading_minus,
		zero,
		dot,
		integer,
		fractional,
		exponent_delim,
		exponent_sign,
		exponent_val,
		done,
		failure,
	};

	enum class e_number_events
	{
		// ascii part
		minus,		// -
		plus,		// +
		dec_zero,	// 0
		dec_digit,			// 0x31 - 0x39
		dot,			// .
		exponent,	// E or 0x65 - e
		symbol,
	};

	class number_parser
		: public parser_impl<e_number_events, e_number_states, e_number_states::initial>
	{
		using event_t				= e_number_events;
		using state_t				= e_number_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= std::variant<int64_t, uint64_t, int32_t, uint32_t, int16_t, uint16_t, int8_t, uint8_t, double, float>;

	public:
		number_parser()
			: m_event_2_state_table
		{
			{ state_t::initial,			{	{ event_t::minus,		{ state_t::leading_minus,	BIND(number_parser::on_minus)		} },
											{ event_t::dec_zero,	{ state_t::zero,			BIND(number_parser::on_zero)		} },
											{ event_t::dec_digit,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
											{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
			} },
			{ state_t::leading_minus,	{	{ event_t::dec_zero,	{ state_t::zero,			BIND(number_parser::on_zero)		} },
											{ event_t::dec_digit,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
											{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
			} },
			{ state_t::zero,			{	{ event_t::dot,			{ state_t::dot,				BIND(number_parser::on_dot)			} },
											{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
			} },
			{ state_t::dot,				{	{ event_t::dec_zero,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
											{ event_t::dec_digit,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
											{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
			} },
			{ state_t::integer,			{	{ event_t::dec_zero,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
											{ event_t::dec_digit,	{ state_t::integer,			BIND(number_parser::on_integer)		} },
											{ event_t::dot,			{ state_t::dot,				BIND(number_parser::on_dot)			} },
											{ event_t::symbol,		{ state_t::done,			BIND(number_parser::on_done)		} },
			} },
			{ state_t::fractional,		{	{ event_t::dec_zero,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
											{ event_t::dec_digit,	{ state_t::fractional,		BIND(number_parser::on_fractional)	} },
											{ event_t::exponent,	{ state_t::exponent_delim,	BIND(number_parser::on_exponent)	} },
											{ event_t::symbol,		{ state_t::done,			BIND(number_parser::on_done)		} },
			} },
			{ state_t::exponent_delim,	{	{ event_t::minus,		{ state_t::exponent_sign,	BIND(number_parser::on_exp_sign)	} },
											{ event_t::plus,		{ state_t::exponent_sign,	BIND(number_parser::on_exp_sign)	} },
											{ event_t::dec_zero,	{ state_t::exponent_delim,	BIND(number_parser::on_exp_value)	} },
											{ event_t::dec_digit,	{ state_t::exponent_delim,	BIND(number_parser::on_exp_value)	} },
											{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
			} },
			{ state_t::exponent_sign,	{	{ event_t::dec_zero,	{ state_t::exponent_val,	BIND(number_parser::on_exp_value)	} },
											{ event_t::dec_digit,	{ state_t::exponent_val,	BIND(number_parser::on_exp_value)	} },
											{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
			} },
			{ state_t::exponent_val,	{	{ event_t::dec_zero,	{ state_t::exponent_val,	BIND(number_parser::on_exp_value)	} },
											{ event_t::dec_digit,	{ state_t::exponent_val,	BIND(number_parser::on_exp_value)	} },
											{ event_t::symbol,		{ state_t::done,			BIND(number_parser::on_done)		} },
			} },
			{ state_t::done,			{	{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
			} },
			{ state_t::failure,			{	{ event_t::symbol,		{ state_t::failure,			BIND(number_parser::on_fail)		} },
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
			{
				value val;
				const number& num = (*m_value);
				// contruct decimal fraction
				if (num.m_fractional_value > 0)
				{
					// 1. put fractional part and shift all it's digits to the right
					// 2. add integer part
					double result = ((double)num.m_fractional_value) / pow(10, num.m_fractional_digits) + num.m_integer;
					// 3. apply power
					const uint32_t power = (uint32_t)pow(10, num.m_exponent_value);
					if (num.m_has_exponent)
						result = num.m_exponent_positive ? result * power : result / power;
					// 4. apply sign
					val = (num.m_positive ? 1.0 : -1.0) * result;
				}
				else
				{
					int64_t i64 = num.m_integer;
					if (!num.m_positive)
						i64 *= -1;
					val = i64;
				}

				return val;
			}

			assert(0); // TODO: throw an exception
			return value();
		};

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }
		
		virtual event_t to_event(const char& c) const override
		{
			if (0x2D == c)
				return event_t::minus;
			if (0x2B == c)
				return event_t::plus;
			if (0x30 == c)
				return event_t::dec_zero;
			if (0x2E == c)
				return event_t::dot;
			if (0x45 == c || 0x65 == c)
				return event_t::exponent;
			if (0x31 <= c && c <= 0x39)
				return event_t::dec_digit;

			return event_t::symbol;
		};

		virtual event_t to_event(const result_t& c) const override
		{
			return event_t::symbol;
		};

		// Own methods
		result_t on_initial(const unsigned char& c, const int pos)
		{
			// TODO: use symbol
			return result_t::s_need_more;
		}

		result_t on_minus(const unsigned char& c, const int pos)
		{
			if (!m_value.has_value())
				m_value.emplace();

			(*m_value).m_positive = false;

			return result_t::s_need_more;
		}

		result_t on_integer(const unsigned char& c, const int pos)
		{
			if (!m_value.has_value())
				m_value.emplace();

			const result_t res = append_digit((*m_value).m_integer, c);
			return result_t::s_ok == res ? result_t::s_need_more : res;
		}

		result_t on_fractional(const unsigned char& c, const int pos)
		{
			assert(m_value.has_value());
			const result_t res = append_digit((*m_value).m_fractional_value, c);
			(*m_value).m_fractional_digits++;
			return result_t::s_ok == res ? result_t::s_need_more : res;
		}

		result_t on_exponent(const unsigned char& c, const int pos)
		{
			assert(m_value.has_value());
			(*m_value).m_has_exponent = true;
			return result_t::s_need_more;
		}

		result_t on_exp_sign(const unsigned char& c, const int pos)
		{
			assert(m_value.has_value());

			switch (c)
			{
			case 0x2D:
				(*m_value).m_exponent_positive = false; break;
			case 0x2B:
				(*m_value).m_exponent_positive = true; break;
			default:
				return result_t::e_fatal;
			}
			return result_t::s_need_more;
		}

		result_t on_exp_value(const unsigned char& c, const int pos)
		{
			assert(m_value.has_value());
			const result_t res = append_digit((*m_value).m_exponent_value, c);
			return result_t::s_ok == res ? result_t::s_need_more : res;
		}


		result_t on_zero(const unsigned char& c, const int pos)
		{

			const state_t s = state::get();
			result_t res = result_t::s_ok;

			switch (s)
			{
			case state_t::initial:
			case state_t::leading_minus:
			case state_t::integer:
				if (!m_value.has_value())
					m_value.emplace();
				res = append_digit((*m_value).m_integer, c);
				break;
			case state_t::dot:
			case state_t::fractional:
				assert(m_value.has_value());
				res = append_digit((*m_value).m_fractional_value, c);
				break;
			case state_t::exponent_delim:
			case state_t::exponent_sign:
				assert(m_value.has_value());
			case state_t::exponent_val:
				res = append_digit((*m_value).m_exponent_value, c);
				break;
			}

			return result_t::s_ok == res ? result_t::s_need_more : res;
		}

		result_t on_dot(const unsigned char& c, const int pos)
		{
			return result_t::s_need_more;
		}

		result_t on_done(const unsigned char& c, const int pos)
		{
			return result_t::s_done_rpt;
		}

		result_t on_fail(const unsigned char& c, const int pos)
		{
			return result_t::e_unexpected;
		}

		static result_t append_digit(uint32_t& val, const unsigned char& c)
		{
			if (c < 0x30 || 0x39 < c)
				return result_t::e_fatal;

			val *= 10;

			switch (c)
			{
			case 0x31: val += 1; break; //1
			case 0x32: val += 2; break; //2
			case 0x33: val += 3; break; //3
			case 0x34: val += 4; break;	//4
			case 0x35: val += 5; break;	//5
			case 0x36: val += 6; break;	//6
			case 0x37: val += 7; break;	//7
			case 0x38: val += 8; break;	//8
			case 0x39: val += 9; break;	//9
			}

			return result_t::s_ok;
		};

	protected:
		const EventToStateTable_t m_event_2_state_table;

		struct number
		{
			number()
			: m_positive(true)
			, m_integer(0)
			, m_fractional_value(0)
			, m_has_exponent(false)
			, m_exponent_positive(true)
			, m_exponent_value(0)
			{}

			bool		m_positive;
			uint32_t	m_integer;
			uint32_t	m_fractional_value;
			uint32_t	m_fractional_digits;
			bool		m_has_exponent;
			bool		m_exponent_positive;
			uint32_t	m_exponent_value;
		};

		std::optional<number> m_value;
	};
}
#endif // __PARSER_NUMBER_H__