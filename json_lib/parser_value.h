#pragma once
#ifndef __PARSER_VALUE_H__
#define __PARSER_VALUE_H__
namespace json
{
	enum class e_value_read_state
	{
		initial,
		read,
		done,
	};

	enum class e_value_special_symbols
	{
		other = 0xffff,
	};

	template<>
	void state<e_value_read_state, e_value_read_state::initial>::set(e_value_read_state new_state)
	{
		auto state_2_string = [](e_value_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_value_read_state::initial:	str = "initial";				break;
			case e_value_read_state::read:		str = "read";					break;
			case e_value_read_state::done:		str = "done";					break;
			default:							str = "unknown", assert(0);		break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

#ifdef _DEBUG
		std::cout << "value parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << ":\t";
#endif // _DEBUG
		m_state = new_state;
	}

	class value_parser
		: public parser_impl<e_value_special_symbols, e_value_read_state, e_value_read_state::initial>
	{
		using symbol_t		= e_value_special_symbols;
		using read_state_t	= e_value_read_state;
		using StateTable_t	= StateTable<read_state_t, symbol_t>;
	public:
		value_parser();
		~value_parser();

	protected:
		virtual const StateTable_t& table() override { return m_state_table; }

		result on_data(const unsigned char& c, const int pos);
		result on_done(const unsigned char& c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override
		{
			return symbol_t::other;
		}

		virtual void reset() final;

	protected:
		const StateTable_t m_state_table;
		std::list<std::pair<bool, parser::ptr>> parsing_unit;
	};
}
#endif // __PARSER_VALUE_H__