#ifndef __PARSER_ARRAY_H__
#define __PARSER_ARRAY_H__
namespace json
{
	enum class e_array_read_state
	{
		initial,
		before_val,
		in_value,
		after_val,
		done,
	};

	enum class e_array_special_symbols
	{
		// ascii part
		other					= 0xffff,
		left_square_bracket		= 0x005B,
		right_square_bracket	= 0x005D,
		comma					= 0x00,
	};

	template<>
	void state<e_array_read_state, e_array_read_state::initial>::set(e_array_read_state new_state)
	{
#ifdef _DEBUG
		auto state_2_string = [](e_array_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_array_read_state::initial:		str = "initial";				break;
			case e_array_read_state::before_val:	str = "before_val";				break;
			case e_array_read_state::in_value:		str = "in_value";				break;
			case e_array_read_state::after_val:		str = "after_val";				break;
			case e_array_read_state::done:			str = "done";					break;
			default:								str = "unknown", assert(0);		break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

		std::cout << "array parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << ":\t";
#endif // _DEBUG
		m_state = new_state;
	}

	class array_parser
		: public parser_impl<e_array_special_symbols, e_array_read_state, e_array_read_state::initial>
	{
		using symbol_t		= e_array_special_symbols;
		using read_state_t	= e_array_read_state;
		using StateTable_t	= StateTable<read_state_t, symbol_t>;
	public:
		array_parser();
		~array_parser();

	protected:

		virtual result step(const char& c, const int pos) final
		{
			result res = parser_impl::step(c, pos);
			if (state::get() == read_state_t::in_value)
			{
				if (result::s_done == res)
					state::set(read_state_t::after_val);
				else
				if (result::s_done_rpt == res)
				{
					state::set(read_state_t::after_val);
					res = parser_impl::step(c, pos);
				}
			}
			return res;
		}

		virtual const StateTable_t& table() override { return m_state_table; }
		
		result on_before_value(const unsigned char& c, const int pos);
		result on_after_value(const unsigned& c, const int pos);
		result on_value(const unsigned& c, const int pos);
		result on_done(const unsigned char& c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override
		{
			symbol_t s = symbol_t::other;

			switch (state::get())
			{
			case read_state_t::initial:
				if(0x5B == c)
					s = symbol_t::left_square_bracket;
				break;
			case read_state_t::after_val:
				switch (c)
				{
				case 0x5D: 
					s = symbol_t::right_square_bracket;
					break;
				case 0x2C: 
					s = symbol_t::comma;
					break;
				}
				break;
			}

			return s;
		}

		virtual void reset() final;

	protected:
		const StateTable_t m_state_table;
		parser::ptr m_value_parser;
	};
}
#endif // __PARSER_ARRAY_H__