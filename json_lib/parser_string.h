#pragma once
#ifndef __PARSER_STRING_H__
#define __PARSER_STRING_H__
namespace json
{

	enum class e_string_read_state
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
	};

	enum class e_string_special_symbols
	{
		// ascii part
		other			= 0xffff,
		hex_digit		= 0xfffe,
		// ...
		quote			= 0x22,
		back_slash		= 0x5c,
		slash			= 0x2F,
		// ...
		numeric_0		= 0x30,
		// 0x31 - 0x38
		numeric_9		= 0x39,
		// ...
		alpha_A			= 0x41,
		// 0x42 - 0x45
		alpha_F			= 0x46,
		// ...
		alpha_a			= 0x61,
		alpha_b			= 0x62,
		// 0x63 - 0x65
		alpha_f			= 0x66,
		// ...
		alpha_n			= 0x6E,
		// ...
		alpha_r			= 0x72,
		// ...
		alpha_t			= 0x74,
		alpha_u			= 0x75,
	};

	template<>
	void state<e_string_read_state, e_string_read_state::initial>::set(e_string_read_state new_state)
	{
		auto state_2_string = [](e_string_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_string_read_state::initial:		str = "initial";	break;
			case e_string_read_state::inside:		str = "inside";		break;
			case e_string_read_state::escape:		str = "escape";		break;
			case e_string_read_state::cr:			str = "cr";			break;
			case e_string_read_state::lf:			str = "lf";			break;
			case e_string_read_state::unicode_1:
			case e_string_read_state::unicode_2:
			case e_string_read_state::unicode_3:
			case e_string_read_state::unicode_4:
													str = "unicode";	break;
			case e_string_read_state::done:			str = "done";		break;
			default:								str = "unknown";	break;
			}

			return str;
		};

		if (m_state == new_state)
			return;
#ifdef _DEBUG
		std::cout << "\n" << "string parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << ":\t";
#endif // _DEBUG
		m_state = new_state;
	}

	class string_parser
		: public parser_impl<e_string_special_symbols, e_string_read_state, e_string_read_state::initial>
	{
	public:
		using symbol_t = e_string_special_symbols;
		using read_state_t = e_string_read_state;
		using StateTable_t = StateTable<read_state_t, symbol_t>;

		string_parser();
		~string_parser();

	protected:
		virtual const StateTable_t& table() override { return m_state_table; }

		result on_initial(const char&c, const int pos);
		result on_inside(const char&c, const int pos);
		result on_escape(const char&c, const int pos);
		result on_unicode(const char&c, const int pos);
		result on_done(const char&c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override;

		virtual void reset() final;

	protected:
		const StateTable_t m_state_table;
	};

}
#endif //__PARSER_STRING_H__