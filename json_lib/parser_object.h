#pragma once
#ifndef __PARSER_OBJECT_H__
#define __PARSER_OBJECT_H__

namespace json
{
	enum class e_object_states
	{
		initial,		//
		key_before,		//
		key_inside,		//
 		key_after,		//
 		val_before,		//
		val_inside,		//
 		val_after,		//
		done,			//
		failure,		//
	};

	enum class e_object_events
	{
		// ascii part
		obj_begin,		// {
		obj_end,		// }
		key_done,		//
		key_error,		//
		val_done,		//
		val_error,		//
		colon,			// :
		comma,			// ,
		symbol,			// any symbol
		skip,	// space, tab, cr, lf
		nothing,		// no action event
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
			case e_object_states::key_before:	return std::string("key_before");
			case e_object_states::key_inside:	return std::string("key_inside");
			case e_object_states::key_after:	return std::string("key_after");
			case e_object_states::val_before:	return std::string("val_before");
			case e_object_states::val_inside:	return std::string("val_inside");
			case e_object_states::val_after:	return std::string("val_after");
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
		virtual result_t putchar(const char& c, const int pos) final;

		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }

		result_t on_initial(const char& c, const int pos);
		result_t on_more(const char& c, const int pos);
		result_t on_new(const char& c, const int pos);
		result_t on_key(const char& c, const int pos);
		result_t on_val(const char& c, const int pos);
		result_t on_done(const char& c, const int pos);
		result_t on_fail(const char& c, const int pos);
		
		virtual event_t to_event(const char& c)	  const override;
		virtual event_t to_event(const result_t& c) const override;

		virtual void reset() final;

	protected:
		const EventToStateTable_t m_event_2_state_table;

		parser::ptr m_key_parser;
		parser::ptr m_val_parser;
	};
} 

#endif // __PARSER_OBJECT_H__