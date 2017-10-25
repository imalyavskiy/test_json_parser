#pragma once
#ifndef __PARSER_NUMBER_H__
#define __PARSER_NUMBER_H__
namespace json
{
	enum class e_number_read_state
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
	};

	enum class e_number_special_symbols
	{
		// ascii part
		minus = 0x2D,		// -
		plus = 0x2B,		// +
		dec_zero = 0x30,	// 0
		dec_digit,			// 0x31 - 0x39
		dot = 0x2E,			// .
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
			case e_number_read_state::initial:			str = "initial";				break;
			case e_number_read_state::leading_minus:	str = "leading_minus";			break;
			case e_number_read_state::zero:				str = "zero";					break;
			case e_number_read_state::dot:				str = "dot";					break;
			case e_number_read_state::integer:			str = "integer";				break;
			case e_number_read_state::fractional:		str = "fractional";				break;
			case e_number_read_state::exponent_delim:	str = "exponent_delim";			break;
			case e_number_read_state::exponent_sign:	str = "exponent_sign";			break;
			case e_number_read_state::exponent_val:		str = "exponent_val";			break;
			case e_number_read_state::done:				str = "done";					break;
			default:									str = "unknown";				break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

#ifdef _DEBUG
		std::cout << "\t" << "number parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << ":\t";
#endif // _DEBUG
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
		virtual result step(const char& c, const int pos) final;

		virtual const StateTable_t& table() override { return m_state_table; }

		result on_initial(const unsigned char& c, const int pos);
		result on_minus(const unsigned char& c, const int pos);
		result on_integer(const unsigned char& c, const int pos);
		result on_fractional(const unsigned char& c, const int pos);
		result on_exponent(const unsigned char& c, const int pos);
		result on_exponent_sign(const unsigned char& c, const int pos);
		result on_exponent_value(const unsigned char& c, const int pos);
		result on_zero(const unsigned char& c, const int pos);
		result on_dot(const unsigned char& c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override;

		virtual void reset() final;

		static result append_digit(int& val, const unsigned char& c);

	protected:
		const StateTable_t m_state_table;

		bool m_positive;
		int  m_integer;
		int  m_fractional;
		bool m_has_exponent;
		bool m_exponent_positive;
		int  m_exponent_value;
	};
}
#endif // __PARSER_NUMBER_H__