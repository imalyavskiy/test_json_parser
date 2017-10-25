#ifndef __PARSER_NULL_H__
#define __PARSER_NULL_H__
namespace json
{
	enum class e_null_read_state
	{
		initial,
		got_n,
		got_u,
		got_l,
		got_2nd_l,
		done,
	};

	enum class e_null_special_symbols
	{
		// ascii part
		letter_n = 0x6e,	// n
		letter_u = 0x75,	// u
		letter_l = 0x6c,	// l
		other	 = 0xff,
	};

	template<>
	void state<e_null_read_state, e_null_read_state::initial>::set(e_null_read_state new_state)
	{
		auto state_2_string = [](e_null_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_null_read_state::initial:	str = "initial";				break;
			case e_null_read_state::got_n:		str = "got_n";					break;
			case e_null_read_state::got_u:		str = "got_u";					break;
			case e_null_read_state::got_l:		str = "got_l";					break;
			case e_null_read_state::done:		str = "done";					break;
			default:							str = "unknown";				break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

#ifdef _DEBUG
		std::cout << "null parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << ":\t";
#endif // _DEBUG
		m_state = new_state;
	}

	class null_parser
		: public parser_impl<e_null_special_symbols, e_null_read_state, e_null_read_state::initial>
	{
		using symbol_t		= e_null_special_symbols;
		using read_state_t	= e_null_read_state;
		using StateTable_t	= StateTable<read_state_t, symbol_t>;
	public:
		null_parser();
		~null_parser();

	protected:
		virtual const StateTable_t& table() override { return m_state_table; }

		result on_n(const unsigned char& c, const int pos);
		result on_u(const unsigned char& c, const int pos);
		result on_l(const unsigned char& c, const int pos);
		result on_done(const unsigned char& c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override
		{
			switch (c)
			{
			case 0x6e:
				return symbol_t::letter_n;
			case 0x75:
				return symbol_t::letter_u;
			case 0x6c:
				return symbol_t::letter_l;
			}

			return symbol_t::other;
		}

		virtual void reset() final;

	protected:
		const StateTable_t m_state_table;
	};
}
#endif // __PARSER_NULL_H__