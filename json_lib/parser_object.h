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
		object_parser()
			: m_key_parser(create_string_parser())
			, m_val_parser(create_value_parser())
			, m_event_2_state_table
		{
			{ state_t::initial,		{	{ event_t::obj_begin,	{ state_t::key_before,	BIND(object_parser::on_begin) } },
										{ event_t::skip,		{ state_t::val_before,	BIND(object_parser::on_more) } },
										{ event_t::symbol,		{ state_t::failure,		BIND(object_parser::on_fail) } },
			} },
			{ state_t::key_before,	{	{ event_t::obj_end,		{ state_t::done,		BIND(object_parser::on_done) } },
										{ event_t::key_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
										{ event_t::symbol,		{ state_t::key_inside,	BIND(object_parser::on_key) } },
										{ event_t::skip,		{ state_t::key_before,	BIND(object_parser::on_more) } },
			} },
			{ state_t::key_inside,	{	{ event_t::key_done,	{ state_t::key_after,	BIND(object_parser::on_more) } },
										{ event_t::key_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
										{ event_t::symbol,		{ state_t::key_inside,	BIND(object_parser::on_key) } },
			} },
			{ state_t::key_after,	{	{ event_t::colon,		{ state_t::val_before,	BIND(object_parser::on_more) } },
										{ event_t::skip,		{ state_t::key_after,	BIND(object_parser::on_more) } },
			} },
			{ state_t::val_before,	{	{ event_t::symbol,		{ state_t::val_inside,	BIND(object_parser::on_val) } },
										{ event_t::val_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
										{ event_t::skip,		{ state_t::val_before,	BIND(object_parser::on_more) } },
			} },
			{ state_t::val_inside,	{	{ event_t::val_done,	{ state_t::val_after,	BIND(object_parser::on_got_val) } },
										{ event_t::val_error,	{ state_t::failure,		BIND(object_parser::on_fail) } },
										{ event_t::symbol,		{ state_t::val_inside,	BIND(object_parser::on_val) } },
			} },
			{ state_t::val_after,	{	{ event_t::comma,		{ state_t::key_before,	BIND(object_parser::on_new) } },
										{ event_t::obj_end,		{ state_t::done,		BIND(object_parser::on_done) } },
										{ event_t::skip,		{ state_t::val_after,	BIND(object_parser::on_more) } },
			} },
			{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		BIND(object_parser::on_fail) } },
			} },
			{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		BIND(object_parser::on_fail) } },
			} },
		} {};

	protected:
		// inherited via parser
		virtual void reset() final
		{
			state::set(state_t::initial);

			m_key_parser->reset();
			m_val_parser->reset();

			m_value.reset();
		};

		virtual result_t putchar(const char& c, const int pos) final
		{
			result_t r = parser_impl::step(to_event(c), c, pos);

			event_t e = to_event(r);

			if (event_t::nothing == e)
				return r;

			if (event_t::val_done == e || event_t::key_done == e)
			{
				result_t new_r = parser_impl::step(e, c, pos);
				r = result_t::s_need_more == new_r && result_t::s_done_rpt == r ?
					parser_impl::step(to_event(c), c, pos) :
					new_r;

				return r;
			}

			assert(0); // TODO: throw an exception

			return r;
		};

		virtual value get() const final
		{
			if (m_value.has_value())
				return *m_value;

			assert(0); // TODO: throw an exception
			return value();
		};

		// inherited via parser_impl
		virtual const EventToStateTable_t& table() override { return m_event_2_state_table; }
		
		virtual event_t to_event(const char& c)	  const override
		{
			auto is_space = [](const char& c)->bool
			{
				// space, tab, cr, lf
				return (0x20 == c || 0x09 == c || 0x0A == c || 0x0D == c);
			};

			switch (state::get())
			{
			case state_t::initial:
				if (0x7B == c)	//{
					return event_t::obj_begin;
				if (is_space(c))
					return event_t::skip;
				break;
			case state_t::key_before:
				if (0x7D == c)	//}
					return event_t::obj_end;
				if (is_space(c))
					return event_t::skip;
				break;
			case state_t::key_after:
				if (is_space(c))
					return event_t::skip;
				if (0x3A == c)	//:
					return event_t::colon;
				break;
			case state_t::val_before:
				if (is_space(c))
					return event_t::skip;
				break;
			case state_t::val_after:
				if (0x7D == c)	//}
					return event_t::obj_end;
				if (is_space(c))
					return event_t::skip;
				if (0x2c == c)	//,
					return event_t::comma;
				break;
			}

			return event_t::symbol;
		};

		virtual event_t to_event(const result_t& r) const override
		{
			switch (state::get())
			{
			case state_t::key_inside:
				if (result_t::s_done == r)
					return event_t::key_done;
				break;
			case state_t::val_inside:
				if (result_t::s_done == r || result_t::s_done_rpt == r)
					return event_t::val_done;
				break;
			}

			return event_t::nothing;
		};

		// own methods
		result_t on_more(const char& c, const int pos)
		{
			return result_t::s_need_more;
		}

		result_t on_begin(const char& c, const int pos)
		{
			if (!m_value.has_value())
				m_value.emplace();

			return result_t::s_need_more;
		}

		result_t on_new(const char& c, const int pos)
		{
			m_key_parser->reset();
			m_val_parser->reset();

			return result_t::s_need_more;
		}

		result_t on_key(const char& c, const int pos)
		{
			return m_key_parser->putchar(c, pos);
		}

		result_t on_val(const char& c, const int pos)
		{
			return m_val_parser->putchar(c, pos);
		}

		result_t on_done(const char& c, const int pos)
		{
			return result_t::s_done;
		}

		result_t on_fail(const char& c, const int pos)
		{
			m_value.reset();
			return result_t::e_unexpected;
		}

		result_t on_got_val(const char& c, const int pos)
		{
			assert(m_value.has_value());

			const std::string key = std::get<std::string>(m_key_parser->get());
			const value val = m_val_parser->get();

			(*m_value)[key] = val;

			return result_t::s_need_more;
		}
	protected:
		const EventToStateTable_t m_event_2_state_table;

		parser::ptr m_key_parser;
		parser::ptr m_val_parser;

		std::optional<my_value_t> m_value;
	};
} 

#endif // __PARSER_OBJECT_H__