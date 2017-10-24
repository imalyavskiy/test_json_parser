#pragma once
#ifndef __PARSER_JSON_H__
#define __PARSER_JSON_H__

namespace json
{
	enum class e_json_read_state
	{
		undefined,
		initial,
		rd_key_str,
		wt_colon,
		wt_value,
		rd_value,
		in_object,
		out_object,
		in_array,
		out_array,
		__fail__,
	};

	enum class e_json_special_symbols
	{
		// ascii part
		left_curly_brace	= 0x007B,			// {
		right_curly_brace	= 0x007D,			// }
		left_square_brace	= 0x005B,			// [
		right_square_brace	= 0x005D,			// ]
		comma				= 0x002C,			// ,
		colon				= 0x003A,			// :
		space				= 0x0020,			// ' '
		htab				= 0x0009,			// '\t'
		LF					= 0x000A,			// '\n'
		CR					= 0x000D,			// '\r'

		// specials
		WS					= 0x0100,

		other				= 0x0000,			// any symbol
	};

	template<>
	void state<e_json_read_state, e_json_read_state::initial>::set(e_json_read_state new_state)
	{
		auto state_2_string = [](e_json_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_json_read_state::undefined:		str = "undefined";					break;
			case e_json_read_state::initial:		str = "initial";					break;
			case e_json_read_state::rd_key_str:		str = "rd_key_str";					break;
			case e_json_read_state::wt_colon:		str = "wt_colon";					break;
			case e_json_read_state::wt_value:		str = "wt_value";					break;
			case e_json_read_state::rd_value:		str = "rd_value";					break;

			case e_json_read_state::in_object:		str = "in_object";					break;
			case e_json_read_state::out_object:		str = "out_object";					break;

			case e_json_read_state::in_array:		str = "in_array";					break;
			case e_json_read_state::out_array:		str = "out_array";					break;

			case e_json_read_state::__fail__:		str = "__fail__";					break;
			default:								str = "unknown", assert(0);			break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

		std::cout << "\t" << "JSON parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << "." << std::endl;
		m_state = new_state;
	}

	class object_parser
		: public parser_impl<e_json_special_symbols, e_json_read_state, e_json_read_state::initial>
	{
		using symbol_t		= e_json_special_symbols;
		using read_state_t	= e_json_read_state;
		using StateTable_t	= StateTable<read_state_t, symbol_t>;
	public:
		object_parser();
		~object_parser();

	protected:

		virtual const StateTable_t& table() override { return m_state_table; }

		result on_initial(const char& c, const int pos);
		result on_reading_key(const char& c, const int pos);
		result on_wait_colon(const char& c, const int pos);
		result on_wait_value(const char& c, const int pos);
		result on_reading_value(const char& c, const int pos);
		result on_in_object(const char& c, const int pos);
		result on_out_object(const char& c, const int pos);
		result on_failure(const char& c, const int pos);
		
		virtual symbol_t token_type_of(const char& c) const override;

		virtual void reset() final;

	protected:
		const StateTable_t m_state_table;
		
		parser::ptr				m_json_parser;
		parser::ptr				m_str_parser;
		parser::ptr				m_num_parser;
		parser::ptr				m_bool_parser;
		parser::ptr				m_null_parser;
		parser::ptr				m_array_parser;

		std::list<parser::ptr>	m_active_parsers;
	};
} 

#endif // __PARSER_JSON_H__