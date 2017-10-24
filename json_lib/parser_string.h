#pragma once
#ifndef __PARSER_STRING_H__
#define __PARSER_STRING_H__
namespace json
{

	enum class e_string_read_state
	{
		outside,
		inside,
		escape,
		unicode,
		_fail_
	};

	enum class e_string_special_symbols
	{
		// ascii part
		Other = 0x0000,
		Quote = 0x0022,			// "
		ReverseSolidus = 0x005c,			// backslash
		Solidus = 0x002F,			// slash
		BackSpace = 0x0062,			// backspace
		FormFeed = 0x0066,			// formfeed(FF) - A printer command that advances the paper in the printer to the top of the next page.
		LineFeed = 0x006E,			// LF
		CarriageReturn = 0x0072,			// CR
		HTab = 0x0074,			// tab
		Unicode = 0x0075,			// u + XXXX (4 hex digits)
	};

	template<>
	void state<e_string_read_state, e_string_read_state::outside>::set(e_string_read_state new_state)
	{
		auto state_2_string = [](e_string_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_string_read_state::outside:		str = "before";					break;
			case e_string_read_state::inside:		str = "inside";					break;
			case e_string_read_state::escape:		str = "escape";					break;
			case e_string_read_state::unicode:		str = "unicode";				break;
			case e_string_read_state::_fail_:		str = "failure";				break;
			default:								str = "unknown", assert(0);		break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

		std::cout << "\t" << "String parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << "." << std::endl;
		m_state = new_state;
	}

	class string_parser
		: public parser_impl<e_string_special_symbols, e_string_read_state, e_string_read_state::outside>
	{
	public:
		using symbol_t = e_string_special_symbols;
		using read_state_t = e_string_read_state;
		using StateTable_t = StateTable<read_state_t, symbol_t>;

		string_parser();
		~string_parser();

	protected:
		virtual const StateTable_t& table() override { return m_state_table; }

		result on_outside(const char&c, const int pos);
		result on_inside(const char&c, const int pos);
		result on_escape(const char&c, const int pos);
		result on_unicode(const char&c, const int pos);
		result on_failure(const char&c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override;

		virtual void reset() final;

	protected:
		const StateTable_t m_state_table;
	};

}
#endif //__PARSER_STRING_H__