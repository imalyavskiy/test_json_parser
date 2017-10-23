#pragma once
namespace json
{
	enum class error
	{
		ok = 0,
		done = 1,
		fatal = -1,
	};

	// for debug purpose
	enum class parser_id
	{
		parser_json,
		parser_string,
		parser_number,
		parser_bool,
		parser_null,
		parser_array,
	};

#pragma region -- state machine types --
	using processig_func = std::function<error(const char&, const int)>;

	// tt_pair: state, function
	template <typename _READSTATE, typename _STATE_CHANGE_HANDLER>
	using TTransition = std::pair<_READSTATE, _STATE_CHANGE_HANDLER>;

	template<typename STATE, typename PROC_FUNC>
	using Transition = TTransition<STATE, PROC_FUNC>;

	// transitions map
	template <typename _SYMBOL, typename _TRANSITION>
	using TTransitionTable = std::map<typename _SYMBOL, typename _TRANSITION>; // symbol->state

	template<typename STATE, typename SPECIAL, typename PROC_FUNC>
	using TransitionTable = TTransitionTable<SPECIAL, Transition<STATE, PROC_FUNC>> ;

	// map of states and transitions possible to certain state
	template <typename _READSTATE, typename _TRANSITION_TABLE=TransitionTable>
	using TStateTable = std::map<_READSTATE, _TRANSITION_TABLE>; // state->new_state(symbol)

	template<typename STATE, typename SPECIAL_SYMBOLS, typename PROC_FUNC = processig_func>
	using StateTable = TStateTable<STATE, TransitionTable<STATE, SPECIAL_SYMBOLS, PROC_FUNC>>;
#pragma endregion

	template<typename STATE, STATE initial_state>
	class state
	{
	private:
		STATE m_state = initial_state;

	protected:
		state() { set(initial_state); };
		STATE get() const { return m_state; };
		void set(STATE new_state) { m_state = new_state; }
	};


	struct parser
	{
		typedef std::shared_ptr<parser> ptr;

		parser(const parser_id id) : m_id(id) {}

		virtual ~parser() {};

		virtual error step(const char& c, const int pos) = 0;

		const parser_id m_id;
	};

	template<typename SymbolsType, typename StateType, StateType initial_state>
	class parser_impl
		: protected state<StateType, initial_state>
		, public parser
	{
	public:
		using symbol_t = SymbolsType;
		using StateTable_t = StateTable<StateType, SymbolsType>;

		parser_impl(const parser_id id) : parser(id) {};
		
		// The step of the automata
		virtual error step(const char& c, const int pos) final
		{
			symbol_t symbol = token_type_of(c);

			if (nullptr == m_current_processing_func)
			{
				auto state = state::get();
				auto transition_group = table().at(state);
				auto transition = transition_group.at(symbol);

				assert(transition.second);

				return transition.second(c, pos);
			}

			return m_current_processing_func(c, pos);
		}


	protected:
		virtual symbol_t token_type_of(const char& c) const = 0;
		virtual const StateTable_t& table() = 0;

	protected:
		processig_func m_current_processing_func;
	};

#pragma region -- string parser -- 
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
		Other				= 0x0000,
		Quote				= 0x0022,			// "
		ReverseSolidus		= 0x005c,			// backslash
		Solidus				= 0x002F,			// slash
		BackSpace			= 0x0062,			// backspace
		FormFeed			= 0x0066,			// formfeed(FF) - A printer command that advances the paper in the printer to the top of the next page.
		LineFeed			= 0x006E,			// LF
		CarriageReturn		= 0x0072,			// CR
		HTab				= 0x0074,			// tab
		Unicode				= 0x0075,			// u + XXXX (4 hex digits)
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
		using symbol_t		= e_string_special_symbols;
		using read_state_t	= e_string_read_state;
		using StateTable_t	= StateTable<read_state_t, symbol_t>;

		string_parser();
		~string_parser();

	protected:
		virtual const StateTable_t& table() override { return m_state_table; }

		error on_outside(const char&c, const int pos);
		error on_inside(const char&c, const int pos);
		error on_escape(const char&c, const int pos);
		error on_unicode(const char&c, const int pos);
		error on_failure(const char&c, const int pos);

		virtual symbol_t token_type_of(const char& c) const override;

	protected:
		const StateTable_t m_state_table;
	};
#pragma endregion

#pragma region -- number parser --
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
		minus		= 0x2D,	// -
		plus		= 0x2B,	// +
		dec_zero	= 0x30,	// 0
		dec_digit,			// 0x31 - 0x39
		dot			= 0x2E,	// .
		exponent	= 0x45,	// E or 0x65 - e
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
#pragma endregion

#pragma region -- json parser --
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

	class json_parser
		: public parser_impl<e_json_special_symbols, e_json_read_state, e_json_read_state::initial>
	{
		using symbol_t		= e_json_special_symbols;
		using read_state_t	= e_json_read_state;
		using StateTable_t	= StateTable<read_state_t, symbol_t>;
	public:
		json_parser();
		~json_parser();

	protected:

		virtual const StateTable_t& table() override { return m_state_table; }

		error on_initial(const char& c, const int pos);
		error on_reading_key(const char& c, const int pos);
		error on_wait_colon(const char& c, const int pos);
		error on_wait_value(const char& c, const int pos);
		error on_reading_value(const char& c, const int pos);
		error on_in_object(const char& c, const int pos);
		error on_out_object(const char& c, const int pos);
		error on_failure(const char& c, const int pos);
		
		virtual symbol_t token_type_of(const char& c) const override;

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
#pragma endregion

	error process(const std::string& input);
	error process(/*const*/ std::istream& input);
} 