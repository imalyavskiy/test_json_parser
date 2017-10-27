#ifndef __PARSER_BASE_H__
#define __PARSER_BASE_H__

#define BIND(__CLASS_METHOD__) std::bind(&__CLASS_METHOD__, this, std::placeholders::_1, std::placeholders::_2)

namespace json
{
#pragma region -- state machine types --
	using state_change_handler_t = std::function<result_t(const char&, const int)>;

	template <typename READSTATE, typename _STATE_CHANGE_HANDLER>
	using TTransition = std::pair<READSTATE, _STATE_CHANGE_HANDLER>;

	template<typename STATE, typename STATE_CHANGE_HANDLER>
	using Transition = TTransition<STATE, STATE_CHANGE_HANDLER>;

	template <typename EVENT, typename TRANSITION>
	using TTransitionTable = std::map<typename EVENT, typename TRANSITION>;

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
		result_t step(const event_t& e, const char& c, const int pos)
		{
			auto transition_group = table().at(state::get());
			if (transition_group.end() != transition_group.find(e))
			{
				auto transition = transition_group.at(e);
				assert(transition.second);
				result_t res = transition.second(c, pos);
					
				state::set(transition.first);

				return res;
			}

			return result_t::e_unexpected;
		}

	protected:
		virtual event_t to_event(const char& c) const = 0;
		virtual event_t to_event(const result_t& c) const = 0;

		virtual const StateTable_t& table() = 0;
	};
}
#endif // __PARSER_BASE_H__