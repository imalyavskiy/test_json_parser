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
		skip,			// space, tab, cr, lf
		nothing,		// no action event
	};

	class object_parser
		: public parser_impl<e_object_events, e_object_states, e_object_states::initial>
	{
		using event_t				= e_object_events;
		using state_t				= e_object_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= object_t;
	public:
		// {ctor}
		object_parser();

	protected:
		// inherited via parser
		virtual void reset() final;
		virtual result_t putchar(const char& c, const int pos) final;
		virtual value get() const final;

		// inherited via parser_impl
		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }
		virtual event_t to_event(const char& c)	  const override;
		virtual event_t to_event(const result_t& c) const override;

		// own methods
		result_t on_more(const char& c, const int pos);
		result_t on_begin(const char& c, const int pos);
		result_t on_new(const char& c, const int pos);
		result_t on_key(const char& c, const int pos);
		result_t on_val(const char& c, const int pos);
		result_t on_got_val(const char& c, const int pos);
		result_t on_done(const char& c, const int pos);
		result_t on_fail(const char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		parser::ptr m_key_parser;
		parser::ptr m_val_parser;

		std::optional<my_value_t> m_value;
	};
} 

#endif // __PARSER_OBJECT_H__