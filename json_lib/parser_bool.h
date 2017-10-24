#ifndef __PARSER_BOOL_H__
#define __PARSER_BOOL_H__
namespace json
{
	enum class e_bool_read_state
	{
		initial,
		got_t,
		got_r,
		got_u,
		got_f,
		got_a,
		got_l,
		got_s,
		done,
	};

	enum class e_bool_special_symbols
	{
		// ascii part
		letter_a = 0x61,	// a
		letter_e = 0x65,	// e
		letter_f = 0x66,	// f 
		letter_l = 0x6c,	// l
		letter_r = 0x72,	// r
		letter_s = 0x73,	// s
		letter_t = 0x74,	// t
		letter_u = 0x75,	// u
		other = 0xff,
	};

	template<>
	void state<e_bool_read_state, e_bool_read_state::initial>::set(e_bool_read_state new_state)
	{
		auto state_2_string = [](e_bool_read_state s)->std::string
		{
			std::string str;

			switch (s)
			{
			case e_bool_read_state::initial:	str = "initial";				break;
			case e_bool_read_state::got_t:		str = "got_t";					break;
			case e_bool_read_state::got_r:		str = "got_r";					break;
			case e_bool_read_state::got_u:		str = "got_u";					break;
			case e_bool_read_state::got_f:		str = "got_f";					break;
			case e_bool_read_state::got_a:		str = "got_a";					break;
			case e_bool_read_state::got_l:		str = "got_l";					break;
			case e_bool_read_state::got_s:		str = "got_s";					break;
			case e_bool_read_state::done:		str = "done";					break;
			default:							str = "unknown", assert(0);		break;
			}

			return str;
		};

		if (m_state == new_state)
			return;

		std::cout << "\t" << "String parser: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << "." << std::endl;
		m_state = new_state;
	}

	class bool_parser
		: public parser_impl<e_bool_special_symbols, e_bool_read_state, e_bool_read_state::initial>
	{
		using symbol_t		= e_bool_special_symbols;
		using read_state_t	= e_bool_read_state;
		using StateTable_t	= StateTable<read_state_t, symbol_t>;
	public:
		bool_parser();
		~bool_parser();

	protected:
		virtual const StateTable_t& table() override { return m_state_table; }

		result on_t(const unsigned char& c, const int pos);
		result on_r(const unsigned char& c, const int pos);
		result on_u(const unsigned char& c, const int pos);
		result on_f(const unsigned char& c, const int pos);
		result on_a(const unsigned char& c, const int pos);
		result on_l(const unsigned char& c, const int pos);
		result on_s(const unsigned char& c, const int pos);
		result on_done(const unsigned char& c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override
		{
			switch (c)
			{
			case 0x74:
				return symbol_t::letter_t;
			case 0x72:
				return symbol_t::letter_r;
			case 0x75:
				return symbol_t::letter_u;
			case 0x65:
				return symbol_t::letter_e;
			case 0x66:
				return symbol_t::letter_f;
			case 0x61:
				return symbol_t::letter_a;
			case 0x6c:
				return symbol_t::letter_l;
			case 0x73:
				return symbol_t::letter_s;
			}
			return symbol_t::other;
		}

		virtual void reset() final;

	protected:
		const StateTable_t m_state_table;
	};
}
#endif // __PARSER_BOOL_H__