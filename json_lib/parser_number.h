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
		minus = 0x2D,		// -
		plus = 0x2B,		// +
		dec_zero = 0x30,	// 0
		dec_digit,			// 0x31 - 0x39
		dot = 0x2E,			// .
		exponent = 0x45,	// E or 0x65 - e
		symbol,
	};

#ifdef _DEBUG
	template<>
	void state<e_number_states, e_number_states::initial>::set(e_number_states new_state)
	{
		auto state_2_string = [](e_number_states s)->std::string
		{
			switch (s)
			{
			case e_number_states::initial:			return std::string("initial");
			case e_number_states::leading_minus:	return std::string("leading_minus");
			case e_number_states::zero:				return std::string("zero");
			case e_number_states::dot:				return std::string("dot");
			case e_number_states::integer:			return std::string("integer");
			case e_number_states::fractional:		return std::string("fractional");
			case e_number_states::exponent_delim:	return std::string("exponent_delim");
			case e_number_states::exponent_sign:	return std::string("exponent_sign");
			case e_number_states::exponent_val:		return std::string("exponent_val");
			case e_number_states::done:				return std::string("done");
			case e_number_states::failure:			return std::string("failure");
			}
			
			return std::string("unknown");
		};

#ifdef _DEBUG
		std::cout << "number parser:\t" << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;
#endif // _DEBUG
		m_state = new_state;
	}
#endif // _DEBUG

	class number_parser
		: public parser_impl<e_number_events, e_number_states, e_number_states::initial>
	{
		using event_t				= e_number_events;
		using state_t				= e_number_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
	public:
		number_parser();
		~number_parser();

	protected:
		virtual result_t putchar(const char& c, const int pos) final;

		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }

		result_t on_initial(const unsigned char& c, const int pos);
		result_t on_minus(const unsigned char& c, const int pos);
		result_t on_integer(const unsigned char& c, const int pos);
		result_t on_fractional(const unsigned char& c, const int pos);
		result_t on_exponent(const unsigned char& c, const int pos);
		result_t on_exponent_sign(const unsigned char& c, const int pos);
		result_t on_exponent_value(const unsigned char& c, const int pos);
		result_t on_zero(const unsigned char& c, const int pos);
		result_t on_dot(const unsigned char& c, const int pos);
		result_t on_done(const unsigned char& c, const int pos);
		result_t on_fail(const unsigned char& c, const int pos);

		virtual event_t to_event(const char& c) const override;
		virtual event_t to_event(const result_t& c) const override;

		virtual void reset() final;

		static result_t append_digit(int& val, const unsigned char& c);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		bool m_positive;
		int  m_integer;
		int  m_fractional;
		bool m_has_exponent;
		bool m_exponent_positive;
		int  m_exponent_value;
	};
}
#endif // __PARSER_NUMBER_H__