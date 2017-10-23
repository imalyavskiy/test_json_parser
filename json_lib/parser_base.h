#ifndef __PARSER_BASE_H__
#define __PARSER_BASE_H__

#define BIND(__CLASS_METHOD__) std::bind(&__CLASS_METHOD__, this, std::placeholders::_1, std::placeholders::_2)

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
	using TransitionTable = TTransitionTable<SPECIAL, Transition<STATE, PROC_FUNC>>;

	// map of states and transitions possible to certain state
	template <typename _READSTATE, typename _TRANSITION_TABLE = TransitionTable>
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
}
#endif // __PARSER_BASE_H__