#ifndef __PARSER_BASE_H__
#define __PARSER_BASE_H__

#define BIND(__CLASS_METHOD__) std::bind(&__CLASS_METHOD__, this, std::placeholders::_1, std::placeholders::_2)

namespace json
{
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
	using state_change_handler_t = std::function<result(const char&, const int)>;

	template <typename READSTATE, typename _STATE_CHANGE_HANDLER>
	using TTransition = std::pair<READSTATE, _STATE_CHANGE_HANDLER>;

	template<typename STATE, typename STATE_CHANGE_HANDLER>
	using Transition = TTransition<STATE, STATE_CHANGE_HANDLER>;

	template <typename EVENT, typename _TRANSITION>
	using TTransitionTable = std::map<typename EVENT, typename _TRANSITION>;

	template<typename STATE, typename EVENT, typename STATE_CHANGE_HANDLER>
	using TransitionTable = TTransitionTable<EVENT, Transition<STATE, STATE_CHANGE_HANDLER>>;

	template <typename READSTATE, typename TRANSITION_TABLE = TransitionTable>
	using TStateTable = std::map<READSTATE, TRANSITION_TABLE>;

	template<typename STATE, typename EVENT, typename STATE_CHANGE_HANDLER = state_change_handler_t>
	using StateTable = TStateTable<STATE, TransitionTable<STATE, EVENT, STATE_CHANGE_HANDLER>>;
#pragma endregion

	template<typename STATE, STATE initial_state>
	class state
	{
	private:
		STATE m_state = initial_state;

	protected:
		STATE get() const { return m_state; };
		void set(STATE new_state) { m_state = new_state; }
	};

	template<typename EventsType, typename StateType, StateType initial_state>
	class parser_impl
		: protected state<StateType, initial_state>
		, public parser
	{
	public:
		using event_t		= EventsType;
		using StateTable_t	= StateTable<StateType, EventsType>;

		parser_impl() {};

		// The step of the automata
		virtual result step(const char& c, const int pos) override
		{
			event_t symbol = to_event(c);

			if (nullptr == m_current_processing_func)
			{
				const StateType s = state::get();
				auto transition_group = table().at(s);
				if (transition_group.end() != transition_group.find(symbol))
				{
					auto transition = transition_group.at(symbol);
					assert(transition.second);
					result res = transition.second(c, pos);
					state::set(transition.first);

					return res;
				}
				else
					return result::e_unexpected;
			}

			return m_current_processing_func(c, pos);
		}


	protected:
		virtual event_t to_event(const char& c) const = 0;
		virtual const StateTable_t& table() = 0;

	protected:
		state_change_handler_t m_current_processing_func;
	};
}
#endif // __PARSER_BASE_H__