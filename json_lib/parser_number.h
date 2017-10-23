#pragma once
#ifndef __PARSER_NUMBER_H__
#define __PARSER_NUMBER_H__
namespace json
{
	enum class e_number_read_state
	{
		initial,
		minus,
		zero,
		dot,
		integer,
		fractional,
		exponential,
		_failure_,
	};

	enum class e_number_special_symbols
	{
		// ascii part
		minus = 0x2D,	// -
		plus = 0x2B,	// +
		dec_zero = 0x30,	// 0
		dec_digit,			// 0x31 - 0x39
		dot = 0x2E,	// .
		exponent = 0x45,	// E or 0x65 - e
		other,
	};

	template<>
	void state<e_number_read_state, e_number_read_state::initial>::set(e_number_read_state new_state)
	{
		auto state_2_string = [](e_number_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_number_read_state::initial:		str = "initial";				break;
			case e_number_read_state::minus:		str = "minus";					break;
			case e_number_read_state::integer:		str = "integer";				break;
			case e_number_read_state::fractional:	str = "fractional";				break;
			case e_number_read_state::exponential:	str = "exponential";			break;
			default:								str = "unknown", assert(0);		break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

		std::cout << "\t" << "String parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << "." << std::endl;
		m_state = new_state;
	}

	class number_parser
		: public parser_impl<e_number_special_symbols, e_number_read_state, e_number_read_state::initial>
	{
		using symbol_t = e_number_special_symbols;
		using read_state_t = e_number_read_state;
		using StateTable_t = StateTable<read_state_t, symbol_t>;
	public:
		number_parser();
		~number_parser();

	protected:
		virtual const StateTable_t& table() override { return m_state_table; }

		error on_initial(const char& c, const int pos);
		error on_minus(const char& c, const int pos);
		error on_integer(const char& c, const int pos);
		error on_fractional(const char& c, const int pos);
		error on_exponential(const char& c, const int pos);
		error on_failure(const char& c, const int pos);
		error on_zero(const char& c, const int pos);
		error on_dot(const char& c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override;

	protected:
		const StateTable_t m_state_table;
	};
}
#endif // __PARSER_NUMBER_H__