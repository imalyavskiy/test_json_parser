#pragma once
namespace json
{
	enum class e_read_state
	{
		undefined,
		initial,
		wait_pair,
		off_pair,
		reading_key,
		wait_colon,
		wait_value,
		reading_value,
		off_object,
		__fail__,
	};

	enum e_token
	{
		left_curly_brace,		// {
		right_curly_brace,		// }
		left_square_brace,		// [
		right_square_brace,		// ]
		comma,					// ,
		colon,					// :
		quote,					// "
		eos,					// end of stream
		other,					// any symbol
	};

	// tt_pair: state, function
	template <typename _READSTATE, typename _STATE_CHANGE_HANDLER = std::function<void(const char&, const int)>>
	using TTransition = std::pair<_READSTATE, _STATE_CHANGE_HANDLER>;

	typedef TTransition<e_read_state> Transition;

	// transitions map
	template <typename _SYMBOL, typename _TRANSITION>
	using TTransitionTable = std::map<typename _SYMBOL, typename _TRANSITION>; // symbol->state

	typedef TTransitionTable<e_token, Transition> TransitionTable;

	// map of states and transitions possible to certain state
	template <typename _READSTATE, typename _TRANSITION_TABLE=TransitionTable>
	using TStateTable = std::map<_READSTATE, _TRANSITION_TABLE>; // state->new_state(symbol)

	typedef TStateTable<e_read_state, TransitionTable> StateTable;

	enum class error
	{
		ok = 0,
		fatal = -1,
	};

	class state
	{
	private:
		e_read_state m_state = e_read_state::undefined;

	public:
		state()
		{
			set(e_read_state::initial);
		}

		void set(e_read_state new_state)
		{
			auto state_2_string = [](e_read_state s)->std::string
			{
				std::string str;

				switch (s)
				{
				case e_read_state::undefined:			str = "undefined";					break;
					case e_read_state::initial:			str = "initial";					break;
					case e_read_state::wait_pair:		str = "waintg for key:value pair";	break;
					case e_read_state::off_pair:		str = "read key:value pair";		break;
					case e_read_state::reading_key:		str = "reading key";				break;
					case e_read_state::wait_colon:		str = "wait for colon sign";		break;
					case e_read_state::wait_value:		str = "waiting for value";			break;
					case e_read_state::reading_value:	str = "reading value";				break;
					case e_read_state::off_object:		str = "off object";					break;
					case e_read_state::__fail__:		str = "fail";						break;
					default:							str = "unknown", assert(0);			break;
				}

				return str;
			};

			if (m_state == new_state)
				return;

			std::cout << "State change: " << state_2_string(m_state) << " -> " << state_2_string(new_state) << "." << std::endl;
			m_state = new_state;
		}

		e_read_state get() const
		{
			return m_state;
		}
	};

	class tokenizer
		: protected state
	{
	public:
		tokenizer();
		~tokenizer();

		error process(const std::string& input);
		error process(/*const*/ std::istream& input);
	
	protected:
		void on_initial(const char& c, const int pos);
		void on_wait_pair(const char& c, const int pos);
		void on_off_pair(const char& c, const int pos);
		void on_reading_key(const char& c, const int pos);
		void on_wait_colon(const char& c, const int pos);
		void on_wait_value(const char& c, const int pos);
		void on_reading_value(const char& c, const int pos);
		void on_off_object(const char& c, const int pos);
		void on_failure(const char& c, const int pos);
		
		// The step of the automata
		error step(char& c, const int pos);

		static e_token token_type_of(char& c);

	protected:
		std::function<void(const char&, const int)> m_on_initial_func;
		std::function<void(const char&, const int)> m_on_wait_pair_func;
		std::function<void(const char&, const int)> m_on_off_pair_func;
		std::function<void(const char&, const int)> m_on_reading_key_func;
		std::function<void(const char&, const int)> m_on_wait_colon_func;
		std::function<void(const char&, const int)> m_on_wait_value_func;
		std::function<void(const char&, const int)> m_on_reading_value_func;
		std::function<void(const char&, const int)> m_on_off_object_func;
		std::function<void(const char&, const int)> m_on_failure_func;

		const StateTable m_state_table;
	};
} 