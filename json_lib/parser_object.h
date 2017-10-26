#pragma once
#ifndef __PARSER_JSON_H__
#define __PARSER_JSON_H__

namespace json
{
	enum class e_object_states
	{
		initial,
		before_key,
// 		after_key,
// 		before_val,
// 		after_val,
		done,
		failure,
	};

	enum class e_object_events
	{
		// ascii part
		left_curly_brace	= 0x007B,			// {
		right_curly_brace	= 0x007D,			// }
		other				= 0x0000,			// any symbol
	};

#ifdef _DEBUG
	template<>
	void state<e_object_states, e_object_states::initial>::set(e_object_states new_state)
	{
		auto state_2_string = [](e_object_states s)->std::string
		{
			switch (s)
			{
			case e_object_states::initial:		return std::string("initial");
			case e_object_states::before_key:	return std::string("before_key");
			case e_object_states::done:			return std::string("done");
			case e_object_states::failure:		return std::string("failure");
			}

			return std::string("unknown");
		};

		std::cout << "object parser:\t" << state_2_string(m_state) << " -> " << state_2_string(new_state) << std::endl;
		m_state = new_state;
	}
#endif // _DEBUG

	class object_parser
		: public parser_impl<e_object_events, e_object_states, e_object_states::initial>
	{
		using event_t				= e_object_events;
		using state_t				= e_object_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
	public:
		object_parser();
		~object_parser();

	protected:
		virtual result step(const char& c, const int pos) final;

		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }

		result on_initial(const char& c, const int pos);
		result on_before_val(const char& c, const int pos);
		result on_done(const char& c, const int pos);
		result on_fail(const char& c, const int pos);
		
		virtual event_t to_event(const char& c) const override;

		virtual void reset() final;

	protected:
		const EventToStateTable_t m_event_2_state_table;
	};
} 

#endif // __PARSER_JSON_H__