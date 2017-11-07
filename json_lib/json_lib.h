#ifndef __JSON_LIB_H__
#define __JSON_LIB_H__

#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

struct json
{
	/// Forward declaration for JSON object data structure
	template <class StringT = std::string, class KeyT = StringT>
	struct object_t;

	/// Forward declaration for JSON array data structure
	template <class StringT = std::string>
	struct array_t;

	/// possible results
	enum class result_t
	{
		s_need_more		=  3, // need more data
		s_done_rpt		=  2, // symbol succesfully terminates parsing but makes no sense to current parser, put it once again to above parser
		s_done			=  1, // symbol succesfully terminates parsing and makes sense to current parser
		s_ok			=  0, // general success
		e_fatal			= -1, // general failure
		e_unexpected	= -2, // unexpected parameter value
	};

	inline static bool failed(const result_t& r) { return r < result_t::s_ok; }
	inline static bool succeded(const result_t& r) { return r >= result_t::s_ok; }

    static result_t parse(std::istream& input, json::object_t<>& jsobj)
    {
        char c = 0;
        result_t result = result_t::s_ok;

        parser::ptr p = create();
        if (!p)
            return result_t::e_fatal;

        while (input >> std::noskipws >> c || result != json::result_t::s_done)
        {
            result = p->putchar(c, (int)input.tellg() - 1);

            if (result_t::s_ok > result)
                break;

            if (result_t::s_done == result)
            {
                jsobj = std::get<json::object_t<>>(p->get());
                break;
            }
        }

        return result;
    }

    static result_t parse(const std::string& input, json::object_t<>& jsobj)
    {
        std::stringstream sstr;
        sstr.str(input);
        return parse(sstr, jsobj);
    }

#pragma region -- value definition --
	template<class StringT = std::string, class ObjectT = object_t<StringT, StringT>, class ArrayT = array_t<StringT>, class IntNumT = int64_t, class RealNumT = double, class BooleanT = bool, class NullT = nullptr_t>
	struct value_t
		: public std::variant<StringT, ObjectT, ArrayT, IntNumT, RealNumT, BooleanT, NullT>
	{
		using base_t = std::variant<StringT, ObjectT, ArrayT, IntNumT, RealNumT, BooleanT, NullT>;

		/// {ctor}s
		value_t() { }
		value_t(const StringT&	other) : base_t(other) {}
		value_t(const ObjectT&	other) : base_t(other) {}
		value_t(const ArrayT&	other) : base_t(other) {}
		value_t(const IntNumT	other) : base_t(other) {}
		value_t(const RealNumT other) : base_t(other) {}
		value_t(const BooleanT other) : base_t(other) {}
		value_t(const char* other) : base_t(StringT(other)) {}
		value_t(const NullT other) : base_t(other) {}

		/// Assign operators
		const value_t& operator=(const StringT& other)
		{
			base_t::operator=(other);
			return (*this);
		}

		const value_t& operator=(const ObjectT& other)
		{
			base_t::operator=(other);
			return (*this);
		}

		const value_t& operator=(const ArrayT& other)
		{
			base_t::operator=(other);
			return (*this);
		}

		const value_t& operator=(const IntNumT other)
		{
			base_t::operator=(other);
			return (*this);
		}

		const value_t& operator=(const RealNumT other)
		{
			base_t::operator=(other);
			return (*this);
		}

		const value_t& operator=(const BooleanT other)
		{
			base_t::operator=(other);
			return (*this);
		}

		const value_t& operator=(const char* other)
		{
			base_t::operator=(StringT(other));
			return (*this);
		}

		const value_t& operator=(const NullT other)
		{
			base_t::operator=(other);
			return (*this);
		}
	};

	/// Shortening
	using value = value_t<>;
#pragma endregion
//
#pragma region -- json data declaration --
	template<class BaseType>
	struct container
		: public BaseType
	{
		using base_t = BaseType;

		/// The enumeration for mnemonic correlation of indeces and types used for std::variant
		enum class vt
		{	/// IMPORTANT: The order of parameters is the same as value_t template parameters order
			t_string = 0,
			t_object = 1,
			t_array = 2,
			t_int64 = 3,
			t_floatingpt = 4,
			t_boolean = 5,
			t_null = 6,
		};

		/// {ctor}s
		container() = default;
		container(std::initializer_list<value> l) : BaseType(l) {}

		virtual const std::string str(std::stringstream& str = std::stringstream()) = 0;
	};

	/// Declaration of the object JSON data structure
	template <class StringT, class KeyT>
	struct object_t : public container<std::map<StringT, value>>
	{
		/// {ctor}s
		object_t() = default;
		object_t(std::initializer_list<std::pair<KeyT, value>> l);

		/// serialization
		virtual const StringT str(std::stringstream& str = std::stringstream()) final;
	};

	/// Declaration of the array JSON data structure
	template <class StringT>
	struct array_t : public container<std::vector<value>>
	{
		/// {ctor}s
		array_t() = default;
		array_t(std::initializer_list<value> l);

		/// serialization
		virtual const StringT str(std::stringstream& str = std::stringstream()) final;
	};
#pragma endregion
//
#pragma region -- parser interface --
    /// Common parser interface
	struct parser
	{
		typedef std::shared_ptr<parser> ptr;

		virtual ~parser() {};

		/// Drops the internal state to initial(i.e. as just constructed)
		virtual void		reset() = 0;

		/// Puts a character to the parsing routine
		virtual result_t	putchar(const char& c, const int pos) = 0;

		/// Retrieves the parsing result
		virtual value		get() const = 0;
	};
#pragma endregion
//
#pragma region -- parser_base -- 
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
		using event_t = EventsType;
		using StateTable_t = StateTable<StateType, EventsType>;

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
#pragma endregion
//////////////////////////////////////////////////////////////////////////
#pragma region -- string parser declaration -- 
	enum class e_string_states
	{
		initial,	// wait for " and skipping space charscters - space, hrisontal tab, crlf, lf
		inside,
		escape,
		cr,
		lf,
		unicode_1,
		unicode_2,
		unicode_3,
		unicode_4,
		done,
		failure,
	};

	enum class e_string_events
	{
		symbol,
		hex_digit,
		quote,
		back_slash,
		slash,
		alpha_b,	// stands for backspace
		alpha_f,	// stands for form feed
		alpha_n,	// stands for new line
		alpha_r,	// stands for carriage return
		alpha_t,	// stands for tab
		alpha_u,	// stands for unicode
	};

	template <class StringT = std::string>
	class string_parser_t
		: public parser_impl<e_string_events, e_string_states, e_string_states::initial>
	{
	public:
		using string_t				= StringT;
		using event_t				= e_string_events;
		using state_t				= e_string_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using my_value_t			= std::string;

		string_parser_t()
			: m_event_2_state_table
		{
			{ state_t::initial,		{	{ event_t::quote,		{ state_t::inside,		std::bind(&string_parser_t::on_initial,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::inside,		{	{ event_t::quote,		{ state_t::done,		std::bind(&string_parser_t::on_done,		this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::back_slash,	{ state_t::escape,		std::bind(&string_parser_t::on_escape,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::escape,		{	{ event_t::quote,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::back_slash,	{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::slash,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::alpha_b,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::alpha_f,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::alpha_n,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::alpha_r,		{ state_t::cr,			std::bind(&string_parser_t::on_escape,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::alpha_t,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::alpha_u,		{ state_t::unicode_1,	std::bind(&string_parser_t::on_unicode,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::cr,			{	{ event_t::back_slash,	{ state_t::lf,			std::bind(&string_parser_t::on_escape,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::lf,			{	{ event_t::alpha_n,		{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::unicode_1,	{	{ event_t::hex_digit,	{ state_t::unicode_2,	std::bind(&string_parser_t::on_unicode,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::unicode_2,	{	{ event_t::hex_digit,	{ state_t::unicode_3,	std::bind(&string_parser_t::on_unicode,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::unicode_3,	{	{ event_t::hex_digit,	{ state_t::unicode_4,	std::bind(&string_parser_t::on_unicode,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::unicode_4,	{	{ event_t::hex_digit,	{ state_t::inside,		std::bind(&string_parser_t::on_inside,	this, std::placeholders::_1, std::placeholders::_2)	} },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
			{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		std::bind(&string_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2)	} },
			} },
		} {};

	protected:
		// Inherited via parser
		virtual void reset() final;

		virtual result_t putchar(const char& c, const int pos) final;

		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override;

		virtual event_t to_event(const char& c) const override;

		virtual event_t to_event(const result_t& c) const override;

		// Own methods
		result_t on_initial(const char&c, const int pos);

		result_t on_inside(const char&c, const int pos);

		result_t on_escape(const char&c, const int pos);

		result_t on_unicode(const char&c, const int pos);

		result_t on_done(const char&c, const int pos);

		result_t on_fail(const char&c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		 std::optional<my_value_t> m_value;
	};
#pragma endregion
//
#pragma region -- number parser declaration -- 
	enum class e_number_states
	{
		initial,
		leading_minus,
		zero,
		dot,
		integer,
		fractional,
		exponent_delim,
		exponent_sign,
		exponent_val,
		done,
		failure,
	};

	enum class e_number_events
	{
		// ascii part
		minus,		// -
		plus,		// +
		dec_zero,	// 0
		dec_digit,			// 0x31 - 0x39
		dot,			// .
		exponent,	// E or 0x65 - e
		symbol,
	};

	template <class StringT = std::string>
	class number_parser_t
		: public parser_impl<e_number_events, e_number_states, e_number_states::initial>
	{
		using string_t = StringT;
		using event_t = e_number_events;
		using state_t = e_number_states;
		using EventToStateTable_t = StateTable<state_t, event_t>;
		using my_value_t = std::variant<int64_t, uint64_t, int32_t, uint32_t, int16_t, uint16_t, int8_t, uint8_t, double, float>;
		
	public:
		number_parser_t()
			: m_event_2_state_table
		{
			{ state_t::initial,			{	{ event_t::minus,		{ state_t::leading_minus,	std::bind(&number_parser_t::on_minus,		this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_zero,	{ state_t::zero,			std::bind(&number_parser_t::on_zero,		this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::integer,			std::bind(&number_parser_t::on_integer,	this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::leading_minus,	{	{ event_t::dec_zero,	{ state_t::zero,			std::bind(&number_parser_t::on_zero,		this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::integer,			std::bind(&number_parser_t::on_integer,	this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::zero,			{	{ event_t::dot,			{ state_t::dot,				std::bind(&number_parser_t::on_dot,		this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::dot,				{	{ event_t::dec_zero,	{ state_t::fractional,		std::bind(&number_parser_t::on_fractional,this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::fractional,		std::bind(&number_parser_t::on_fractional,this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::integer,			{	{ event_t::dec_zero,	{ state_t::integer,			std::bind(&number_parser_t::on_integer,	this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::integer,			std::bind(&number_parser_t::on_integer,	this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dot,			{ state_t::dot,				std::bind(&number_parser_t::on_dot,		this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::done,			std::bind(&number_parser_t::on_done,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::fractional,		{	{ event_t::dec_zero,	{ state_t::fractional,		std::bind(&number_parser_t::on_fractional,this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::fractional,		std::bind(&number_parser_t::on_fractional,this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::exponent,	{ state_t::exponent_delim,	std::bind(&number_parser_t::on_exponent,	this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::done,			std::bind(&number_parser_t::on_done,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::exponent_delim,	{	{ event_t::minus,		{ state_t::exponent_sign,	std::bind(&number_parser_t::on_exp_sign,	this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::plus,		{ state_t::exponent_sign,	std::bind(&number_parser_t::on_exp_sign,	this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_zero,	{ state_t::exponent_delim,	std::bind(&number_parser_t::on_exp_value, this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::exponent_delim,	std::bind(&number_parser_t::on_exp_value, this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::exponent_sign,	{	{ event_t::dec_zero,	{ state_t::exponent_val,	std::bind(&number_parser_t::on_exp_value, this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::exponent_val,	std::bind(&number_parser_t::on_exp_value, this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::exponent_val,	{	{ event_t::dec_zero,	{ state_t::exponent_val,	std::bind(&number_parser_t::on_exp_value, this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::dec_digit,	{ state_t::exponent_val,	std::bind(&number_parser_t::on_exp_value, this, std::placeholders::_1, std::placeholders::_2) } },
											{ event_t::symbol,		{ state_t::done,			std::bind(&number_parser_t::on_done,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::done,			{	{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::failure,			{	{ event_t::symbol,		{ state_t::failure,			std::bind(&number_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
		} {};

	protected:
		// Inherited via parser
		virtual void reset() final;

		virtual result_t putchar(const char& c, const int pos) final;

		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override;

		virtual event_t to_event(const char& c) const override;

		virtual event_t to_event(const result_t& c) const override;

		// Own methods
		result_t on_initial(const unsigned char& c, const int pos);

		result_t on_minus(const unsigned char& c, const int pos);

		result_t on_integer(const unsigned char& c, const int pos);

		result_t on_fractional(const unsigned char& c, const int pos);

		result_t on_exponent(const unsigned char& c, const int pos);

		result_t on_exp_sign(const unsigned char& c, const int pos);

		result_t on_exp_value(const unsigned char& c, const int pos);


		result_t on_zero(const unsigned char& c, const int pos);

		result_t on_dot(const unsigned char& c, const int pos);

		result_t on_done(const unsigned char& c, const int pos);

		result_t on_fail(const unsigned char& c, const int pos);

		static result_t append_digit(uint32_t& val, const unsigned char& c);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		struct number
		{
			number()
				: m_positive(true)
				, m_integer(0)
				, m_fractional_value(0)
				, m_has_exponent(false)
				, m_exponent_positive(true)
				, m_exponent_value(0)
			{}

			bool		m_positive;
			uint32_t	m_integer;
			uint32_t	m_fractional_value;
			uint32_t	m_fractional_digits;
			bool		m_has_exponent;
			bool		m_exponent_positive;
			uint32_t	m_exponent_value;
		};

		std::optional<number> m_value;
	};
#pragma endregion
//
#pragma region -- null parser declaration -- 
	enum class e_null_states
	{
		initial,
		got_n,
		got_u,
		got_l,
		got_2nd_l,
		done,
		failure,
	};

	enum class e_null_events
	{
		// ascii part
		letter_n,	// n
		letter_u,	// u
		letter_l,	// l
		other,
	};

	template <class StringT = std::string>
	class null_parser_t
		: public parser_impl<e_null_events, e_null_states, e_null_states::initial>
	{
		using string_t = StringT;
		using event_t = e_null_events;
		using state_t = e_null_states;
		using EventToStateTable_t = StateTable<state_t, event_t>;
		using my_value_t = nullptr_t;
		//std::bind(&string_parser::on_initial, this, std::placeholders::_1, std::placeholders::_2)
	public:
		null_parser_t()
			: m_event_2_state_table
		{
			{ state_t::initial,	{	{ event_t::letter_n,	{ state_t::got_n,	std::bind(&null_parser_t::on_n,		this, std::placeholders::_1, std::placeholders::_2) } },
									{ event_t::other,		{ state_t::failure,	std::bind(&null_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_n,	{	{ event_t::letter_u,	{ state_t::got_u,	std::bind(&null_parser_t::on_u,		this, std::placeholders::_1, std::placeholders::_2) } },
									{ event_t::other,		{ state_t::failure,	std::bind(&null_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_u,	{	{ event_t::letter_l,	{ state_t::got_l,	std::bind(&null_parser_t::on_l,		this, std::placeholders::_1, std::placeholders::_2) } },
									{ event_t::other,		{ state_t::failure,	std::bind(&null_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_l,	{	{ event_t::letter_l,	{ state_t::done,	std::bind(&null_parser_t::on_done,	this, std::placeholders::_1, std::placeholders::_2) } },
									{ event_t::other,		{ state_t::failure,	std::bind(&null_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::done,	{	{ event_t::other,		{ state_t::failure,	std::bind(&null_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::failure,	{	{ event_t::other,		{ state_t::failure,	std::bind(&null_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
		}
		{};

	protected:
		// Inherited via parser
		virtual void reset() final;

		virtual result_t putchar(const char& c, const int pos) final;

		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override;

		virtual event_t to_event(const char& c) const override;

		virtual event_t to_event(const result_t& c) const override;

		// Own methods
		result_t on_n(const unsigned char& c, const int pos);

		result_t on_u(const unsigned char& c, const int pos);

		result_t on_l(const unsigned char& c, const int pos);

		result_t on_done(const unsigned char& c, const int pos);

		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		std::optional<my_value_t> m_value;
	};
#pragma endregion
//
#pragma region -- bool parser declaration -- 
	enum class e_bool_states
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
		failure,
	};

	enum class e_bool_events
	{
		letter_a,
		letter_e,
		letter_f,
		letter_l,
		letter_r,
		letter_s,
		letter_t,
		letter_u,
		symbol,
	};

	template <class StringT = std::string>
	class bool_parser_t
		: public parser_impl<e_bool_events, e_bool_states, e_bool_states::initial>
	{
		using string_t = StringT;
		using event_t = e_bool_events;
		using state_t = e_bool_states;
		using EventToStateTable_t = StateTable<state_t, event_t>;
		using my_value_t = bool;
	public:
		bool_parser_t()
			: m_event_2_state_table
		{
			{ state_t::initial,		{	{ event_t::letter_t,	{ state_t::got_t,	std::bind(&bool_parser_t::on_t,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::letter_f,	{ state_t::got_f,	std::bind(&bool_parser_t::on_f,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_t,		{	{ event_t::letter_r,	{ state_t::got_r,	std::bind(&bool_parser_t::on_r,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_r,		{	{ event_t::letter_u,	{ state_t::got_u,	std::bind(&bool_parser_t::on_u,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_u,		{	{ event_t::letter_e,	{ state_t::done,	std::bind(&bool_parser_t::on_done,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_f,		{	{ event_t::letter_a,	{ state_t::got_a,	std::bind(&bool_parser_t::on_a,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_a,		{	{ event_t::letter_l,	{ state_t::got_l,	std::bind(&bool_parser_t::on_l,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_l,		{	{ event_t::letter_s,	{ state_t::got_s,	std::bind(&bool_parser_t::on_s,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::got_s,		{	{ event_t::letter_e,	{ state_t::done,	std::bind(&bool_parser_t::on_done,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,	std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure, std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure, std::bind(&bool_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
		} {};

	protected:
		// Inherited via parser
		virtual void reset() final;

		virtual result_t putchar(const char& c, const int pos) final;

		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override;

		virtual event_t to_event(const char& c) const override;

		virtual event_t to_event(const result_t& c) const override;

		// Own methods
		result_t on_t(const unsigned char& c, const int pos);

		result_t on_r(const unsigned char& c, const int pos);

		result_t on_u(const unsigned char& c, const int pos);

		result_t on_f(const unsigned char& c, const int pos);

		result_t on_a(const unsigned char& c, const int pos);

		result_t on_l(const unsigned char& c, const int pos);

		result_t on_s(const unsigned char& c, const int pos);

		result_t on_done(const unsigned char& c, const int pos);

		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		std::string m_str;

		std::optional<my_value_t> m_value;
	};
#pragma endregion 
//
#pragma region -- value parser declaration -- 
	enum class e_value_states
	{
		initial,
		read,
		done,
		failure,
	};

	enum class e_value_events
	{
		symbol = 0xffff,
		val_done,
		nothing,
	};

	template <class StringT = std::string>
	class value_parser_t
		: public parser_impl<e_value_events, e_value_states, e_value_states::initial>
	{
		using string_t				= StringT;
		using event_t				= e_value_events;
		using state_t				= e_value_states;
		using EventToStateTable_t	= StateTable<state_t, event_t>;
		using ParserItem_t			= std::pair<bool, parser::ptr>;
		using my_value_t			= value_t<>;
	public:
		value_parser_t()
			: m_event_2_state_table
		{
			{ state_t::initial,	{	{ event_t::symbol,	{ state_t::read,	std::bind(&value_parser_t::on_data, this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::read,	{	{ event_t::symbol,	{ state_t::read,	std::bind(&value_parser_t::on_data, this, std::placeholders::_1, std::placeholders::_2) } },
									{ event_t::val_done,{ state_t::done,	std::bind(&value_parser_t::on_done, this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::done,	{	{ event_t::symbol,	{ state_t::failure,	std::bind(&value_parser_t::on_fail, this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::failure,	{	{ event_t::symbol,	{ state_t::failure,	std::bind(&value_parser_t::on_fail, this, std::placeholders::_1, std::placeholders::_2) } },
			} },
		} {};

	protected:
		// Inherited via parser
		virtual void reset() final;

		virtual result_t putchar(const char& c, const int pos) final;

		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override;

		virtual event_t to_event(const char& c) const override;

		virtual event_t to_event(const result_t& c) const override;

		// Own methods
		result_t on_data(const unsigned char& c, const int pos);

		result_t on_done(const unsigned char& c, const int pos);

		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		std::list<ParserItem_t> parsing_unit;
	};
#pragma endregion
//
#pragma region -- array parser declaration -- 
	enum class e_array_states
	{
		initial,	//
		val_before,	//
		val_inside,	//
		val_after,	//
		done,		//
		failure,	//
	};

	enum class e_array_events
	{
		arr_begin,	// [
		arr_end,	// ]
		val_done,	//
		val_error,	//
		comma,		// ,
		symbol,		// any symbol(depends on state)
		skip,		// space, tab, cr, lf
		nothing,	// no action event
	};

	template <class StringT = std::string>
	class array_parser_t
		: public parser_impl<e_array_events, e_array_states, e_array_states::initial>
	{
		using string_t = StringT;
		using event_t = e_array_events;
		using state_t = e_array_states;
		using EventToStateTable_t = StateTable<state_t, event_t>;
		using my_value_t = array_t<>;
	public:
		array_parser_t()
			: m_event_2_state_table
		{
			{ state_t::initial,		{	{ event_t::arr_begin,	{ state_t::val_before,	std::bind(&array_parser_t::on_begin,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::val_before,	std::bind(&array_parser_t::on_more,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&array_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::val_before,	{	{ event_t::symbol,		{ state_t::val_inside,	std::bind(&array_parser_t::on_val,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::val_before,	std::bind(&array_parser_t::on_more,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::arr_end,		{ state_t::done,		std::bind(&array_parser_t::on_done,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::val_after,	{	{ event_t::arr_end,		{ state_t::done,		std::bind(&array_parser_t::on_done,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::comma,		{ state_t::val_before,	std::bind(&array_parser_t::on_new,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::val_before,	std::bind(&array_parser_t::on_more,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&array_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::val_inside,	{	{ event_t::symbol,		{ state_t::val_inside,	std::bind(&array_parser_t::on_val,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::val_done,	{ state_t::val_after,	std::bind(&array_parser_t::on_got_val,this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::val_error,	{ state_t::failure,		std::bind(&array_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		std::bind(&array_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		std::bind(&array_parser_t::on_fail,	this, std::placeholders::_1, std::placeholders::_2) } },
			} },
		} {};

	protected:
		// Inherited via parser
		virtual void reset() final;

		virtual result_t putchar(const char& c, const int pos) final;

		virtual value get() const final;

		// Inherited via parser_impl
		virtual const EventToStateTable_t& table() override;

		virtual event_t to_event(const char& c) const override;

		virtual event_t to_event(const result_t& r) const override;

		// Own methods
		result_t on_more(const unsigned& c, const int pos);

		result_t on_begin(const unsigned char& c, const int pos);

		result_t on_new(const unsigned char& c, const int pos);

		result_t on_val(const unsigned& c, const int pos);

		result_t on_got_val(const char& c, const int pos);

		result_t on_done(const unsigned char& c, const int pos);

		result_t on_fail(const unsigned char& c, const int pos);

	protected:
		const EventToStateTable_t m_event_2_state_table;

		parser::ptr m_value_parser;

		std::optional<array_t<>> m_value;
	};
#pragma endregion
//
#pragma region -- object parser declaration -- 
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

	template <class StringT = std::string>
	class object_parser_t
		: public parser_impl<e_object_events, e_object_states, e_object_states::initial>
	{
		using string_t = StringT;
		using event_t = e_object_events;
		using state_t = e_object_states;
		using EventToStateTable_t = StateTable<state_t, event_t>;
		using my_value_t = object_t<std::string>;
	public:
		// {ctor}
		object_parser_t()
			: m_event_2_state_table
		{
			{ state_t::initial,		{	{ event_t::obj_begin,	{ state_t::key_before,	std::bind(&object_parser_t::on_begin,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::val_before,	std::bind(&object_parser_t::on_more,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::failure,		std::bind(&object_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::key_before,	{	{ event_t::obj_end,		{ state_t::done,		std::bind(&object_parser_t::on_done,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::key_error,	{ state_t::failure,		std::bind(&object_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::key_inside,	std::bind(&object_parser_t::on_key,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::key_before,	std::bind(&object_parser_t::on_more,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::key_inside,	{	{ event_t::key_done,	{ state_t::key_after,	std::bind(&object_parser_t::on_more,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::key_error,	{ state_t::failure,		std::bind(&object_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::key_inside,	std::bind(&object_parser_t::on_key,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::key_after,	{	{ event_t::colon,		{ state_t::val_before,	std::bind(&object_parser_t::on_more,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::key_after,	std::bind(&object_parser_t::on_more,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::val_before,	{	{ event_t::symbol,		{ state_t::val_inside,	std::bind(&object_parser_t::on_val,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::val_error,	{ state_t::failure,		std::bind(&object_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::val_before,	std::bind(&object_parser_t::on_more,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::val_inside,	{	{ event_t::val_done,	{ state_t::val_after,	std::bind(&object_parser_t::on_got_val,	this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::val_error,	{ state_t::failure,		std::bind(&object_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::symbol,		{ state_t::val_inside,	std::bind(&object_parser_t::on_val,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::val_after,	{	{ event_t::comma,		{ state_t::key_before,	std::bind(&object_parser_t::on_new,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::obj_end,		{ state_t::done,		std::bind(&object_parser_t::on_done,		this, std::placeholders::_1, std::placeholders::_2) } },
										{ event_t::skip,		{ state_t::val_after,	std::bind(&object_parser_t::on_more,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::done,		{	{ event_t::symbol,		{ state_t::failure,		std::bind(&object_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
			{ state_t::failure,		{	{ event_t::symbol,		{ state_t::failure,		std::bind(&object_parser_t::on_fail,		this, std::placeholders::_1, std::placeholders::_2) } },
			} },
		} {};

	protected:
		// inherited via parser
		virtual void reset() final;

		virtual result_t putchar(const char& c, const int pos) final;

		virtual value get() const final;

		// inherited via parser_impl
		virtual const EventToStateTable_t& table() override;

		virtual event_t to_event(const char& c)	  const override;

		virtual event_t to_event(const result_t& r) const override;

		// own methods
		result_t on_more(const char& c, const int pos);

		result_t on_begin(const char& c, const int pos);

		result_t on_new(const char& c, const int pos);

		result_t on_key(const char& c, const int pos);

		result_t on_val(const char& c, const int pos);

		result_t on_done(const char& c, const int pos);

		result_t on_fail(const char& c, const int pos);

		result_t on_got_val(const char& c, const int pos);
	protected:
		const EventToStateTable_t m_event_2_state_table;

		parser::ptr m_key_parser;
		parser::ptr m_val_parser;

		std::optional<my_value_t> m_value;
	};
#pragma endregion 
//////////////////////////////////////////////////////////////////////////
#pragma region -- factory --
 	/// creates object parser
 	inline static parser::ptr create() 
	{ 
		return json::parser::ptr(new object_parser_t<>()); 
	}
#pragma endregion
//////////////////////////////////////////////////////////////////////////
};
//////////////////////////////////////////////////////////////////////////
#pragma region -- json data definition --
template <class StringT, class KeyT>
json::object_t<StringT, KeyT>::object_t(std::initializer_list<std::pair<KeyT, value>> l)
{
    for (auto arg : l)
        insert(arg);
}

template <class StringT, class KeyT>
const StringT 
json::object_t<StringT, KeyT>::str(std::stringstream& str)
{
    // leading curly brace
    str << "{";

    for (auto it = begin(); it != end(); ++it)
    {
        const bool last = (--end() == it);

        // key
        str << "\"" << it->first << "\":";

        // value
        switch ((vt)it->second.index())
        {
        case vt::t_string:
            str << "\"" << std::get<StringT>(it->second) << "\"";
            break;
        case vt::t_object:
            std::get<object_t>(it->second).str(str);
            break;
        case vt::t_array:
            std::get<array_t<StringT>>(it->second).str(str);
            break;
        case vt::t_int64:
            str << std::get<int64_t>(it->second);
            break;
        case vt::t_floatingpt:
            str << std::scientific << std::get<double>(it->second);
            break;
        case vt::t_boolean:
            str << (std::get<bool>(it->second) ? "true" : "false");
            break;
        case vt::t_null:
            str << "null";
            break;
        default: // unknown(i.e. not mentioned) type
            assert(0);
            break;
        }

        // comma if not the last one
        str << (last ? "" : ",");
    }

    // trailing curly brace
    str << "}";

    return str.str();
}

template <class StringT>
json::array_t<StringT>::array_t(std::initializer_list<value> l)
{
    for (auto arg : l)
        push_back(arg);
}

template <class StringT>
const StringT 
json::array_t<StringT>::str(std::stringstream& str)
{
    // leading curly brace
    str << "[";

    for (auto it = begin(); it != end(); ++it)
    {
        const bool last = (--end() == it);

        // value
        switch ((vt)it->index())
        {
        case vt::t_string:
            str << "\"" << std::get<std::string>(*it) << "\"";
            break;
        case vt::t_object:
            std::get<object_t<StringT, StringT>>(*it).str(str);
            break;
        case vt::t_array:
            std::get<array_t>(*it).str(str);
            break;
        case vt::t_int64:
            str << std::get<int64_t>(*it);
            break;
        case vt::t_floatingpt:
            str << std::scientific << std::get<double>(*it);
            break;
        case vt::t_boolean:
            str << ((std::get<bool>(*it) ? "true" : "false"));
            break;
        case vt::t_null:
            str << "null";
            break;
        default:
            assert(0);
            break;
        }

        // comma if not the last one
        str << (last ? "" : ",");
    }

    // trailing curly brace
    str << "]";

    return str.str();
}
#pragma endregion
//
#pragma region -- string parser definition --
template <class StringT>
void
json::string_parser_t<StringT>::reset()
{
    state::set(state_t::initial);
    m_value.reset();
};

template <class StringT>
json::result_t
json::string_parser_t<StringT>::putchar(const char& c, const int pos)
{
    return parser_impl::step(to_event(c), c, pos);
};

template <class StringT>
json::value
json::string_parser_t<StringT>::get() const
{
    if (m_value.has_value())
        return *m_value;

    assert(0); // TODO: throw an exception
    return value();
};

template <class StringT>
const typename json::string_parser_t<StringT>::EventToStateTable_t&
json::string_parser_t<StringT>::table()
{
    return m_event_2_state_table;
}

template <class StringT>
typename json::string_parser_t<StringT>::event_t
json::string_parser_t<StringT>::to_event(const char& c) const
{
    event_t smb = event_t::symbol;
    switch (state::get())
    {
    case state_t::initial:
        if (0x22 == c)
            smb = event_t::quote;
        break;
    case state_t::inside:
        if (0x5C == c)
            smb = event_t::back_slash;
        if (0x22 == c)
            smb = event_t::quote;
        break;
    case state_t::escape:
        switch (c)
        {
        case 0x22:	smb = event_t::quote;		break;
        case 0x5C:	smb = event_t::back_slash;	break;
        case 0x2F:	smb = event_t::slash;		break;
        case 0x62:	smb = event_t::alpha_b;		break;
        case 0x66:	smb = event_t::alpha_f;		break;
        case 0x6E:	smb = event_t::alpha_n;		break;
        case 0x72:	smb = event_t::alpha_r;		break;
        case 0x74:	smb = event_t::alpha_t;		break;
        case 0x75:	smb = event_t::alpha_u;		break;
        }
        break;
    case state_t::cr:
        if (0x5C == c)
            return event_t::back_slash;
        break;
    case state_t::lf:
        if (0x6E == c)
            return event_t::alpha_n;
        break;
    case state_t::unicode_1:
    case state_t::unicode_2:
    case state_t::unicode_3:
    case state_t::unicode_4:
        if (0x30 <= c && c <= 0x39)
            smb = event_t::hex_digit;
        if (0x41 <= c && c <= 0x46)
            smb = event_t::hex_digit;
        if (0x61 <= c && c <= 0x66)
            smb = event_t::hex_digit;
        break;
    case state_t::done:
        assert(0);
        break;
    }

    return smb;
};

template <class StringT>
typename json::string_parser_t<StringT>::event_t
json::string_parser_t<StringT>::to_event(const result_t& c) const
{
    return event_t::symbol;
};

template <class StringT>
json::result_t
json::string_parser_t<StringT>::on_initial(const char&c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::string_parser_t<StringT>::on_inside(const char&c, const int pos)
{
    if (!m_value.has_value())
        m_value.emplace();

    (*m_value) += c;

    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::string_parser_t<StringT>::on_escape(const char&c, const int pos)
{
    assert(m_value.has_value());
    (*m_value) += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::string_parser_t<StringT>::on_unicode(const char&c, const int pos)
{
    assert(m_value.has_value());
    (*m_value) += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::string_parser_t<StringT>::on_done(const char&c, const int pos)
{
    return result_t::s_done;
}

template <class StringT>
json::result_t
json::string_parser_t<StringT>::on_fail(const char&c, const int pos)
{
    return result_t::e_unexpected;
}
#pragma endregion
//
#pragma region -- number parser definition --
template <class StringT>
void
json::number_parser_t<StringT>::reset()
{
    state::set(state_t::initial);

    m_value.reset();
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::putchar(const char& c, const int pos)
{
    return parser_impl::step(to_event(c), c, pos);
}

template <class StringT>
json::value
json::number_parser_t<StringT>::get() const
{
    if (m_value.has_value())
    {
        value val;
        const number& num = (*m_value);
        // contruct decimal fraction
        if (num.m_fractional_value > 0)
        {
            // 1. put fractional part and shift all it's digits to the right
            // 2. add integer part
            double result = ((double)num.m_fractional_value) / pow(10, num.m_fractional_digits) + num.m_integer;
            // 3. apply power
            const uint32_t power = (uint32_t)pow(10, num.m_exponent_value);
            if (num.m_has_exponent)
                result = num.m_exponent_positive ? result * power : result / power;
            // 4. apply sign
            val = (num.m_positive ? 1.0 : -1.0) * result;
        }
        else
        {
            int64_t i64 = num.m_integer;
            if (!num.m_positive)
                i64 *= -1;
            val = i64;
        }

        return val;
    }

    assert(0); // TODO: throw an exception
    return value();
};

// Inherited via parser_impl
template <class StringT>
const typename json::number_parser_t<StringT>::EventToStateTable_t&
json::number_parser_t<StringT>::table()
{
    return m_event_2_state_table;
}

template <class StringT>
typename json::number_parser_t<StringT>::event_t
json::number_parser_t<StringT>::to_event(const char& c) const
{
    if (0x2D == c)
        return event_t::minus;
    if (0x2B == c)
        return event_t::plus;
    if (0x30 == c)
        return event_t::dec_zero;
    if (0x2E == c)
        return event_t::dot;
    if (0x45 == c || 0x65 == c)
        return event_t::exponent;
    if (0x31 <= c && c <= 0x39)
        return event_t::dec_digit;

    return event_t::symbol;
};

template <class StringT>
typename json::number_parser_t<StringT>::event_t
json::number_parser_t<StringT>::to_event(const result_t& c) const
{
    return event_t::symbol;
};

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_initial(const unsigned char& c, const int pos)
{
    // TODO: use symbol
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_minus(const unsigned char& c, const int pos)
{
    if (!m_value.has_value())
        m_value.emplace();

    (*m_value).m_positive = false;

    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_integer(const unsigned char& c, const int pos)
{
    if (!m_value.has_value())
        m_value.emplace();

    const result_t res = append_digit((*m_value).m_integer, c);
    return result_t::s_ok == res ? result_t::s_need_more : res;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_fractional(const unsigned char& c, const int pos)
{
    assert(m_value.has_value());
    const result_t res = append_digit((*m_value).m_fractional_value, c);
    (*m_value).m_fractional_digits++;
    return result_t::s_ok == res ? result_t::s_need_more : res;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_exponent(const unsigned char& c, const int pos)
{
    assert(m_value.has_value());
    (*m_value).m_has_exponent = true;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_exp_sign(const unsigned char& c, const int pos)
{
    assert(m_value.has_value());

    switch (c)
    {
    case 0x2D:
        (*m_value).m_exponent_positive = false; break;
    case 0x2B:
        (*m_value).m_exponent_positive = true; break;
    default:
        return result_t::e_fatal;
    }
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_exp_value(const unsigned char& c, const int pos)
{
    assert(m_value.has_value());
    const result_t res = append_digit((*m_value).m_exponent_value, c);
    return result_t::s_ok == res ? result_t::s_need_more : res;
}


template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_zero(const unsigned char& c, const int pos)
{

    const state_t s = state::get();
    result_t res = result_t::s_ok;

    switch (s)
    {
    case state_t::initial:
    case state_t::leading_minus:
    case state_t::integer:
        if (!m_value.has_value())
            m_value.emplace();
        res = append_digit((*m_value).m_integer, c);
        break;
    case state_t::dot:
    case state_t::fractional:
        assert(m_value.has_value());
        res = append_digit((*m_value).m_fractional_value, c);
        break;
    case state_t::exponent_delim:
    case state_t::exponent_sign:
        assert(m_value.has_value());
    case state_t::exponent_val:
        res = append_digit((*m_value).m_exponent_value, c);
        break;
    }

    return result_t::s_ok == res ? result_t::s_need_more : res;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_dot(const unsigned char& c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_done(const unsigned char& c, const int pos)
{
    return result_t::s_done_rpt;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::on_fail(const unsigned char& c, const int pos)
{
    return result_t::e_unexpected;
}

template <class StringT>
json::result_t
json::number_parser_t<StringT>::append_digit(uint32_t& val, const unsigned char& c)
{
    if (c < 0x30 || 0x39 < c)
        return result_t::e_fatal;

    val *= 10;

    switch (c)
    {
    case 0x31: val += 1; break; //1
    case 0x32: val += 2; break; //2
    case 0x33: val += 3; break; //3
    case 0x34: val += 4; break;	//4
    case 0x35: val += 5; break;	//5
    case 0x36: val += 6; break;	//6
    case 0x37: val += 7; break;	//7
    case 0x38: val += 8; break;	//8
    case 0x39: val += 9; break;	//9
    }

    return result_t::s_ok;
};
#pragma endregion
//
#pragma region -- null parser definition -- 
template <class StringT>
void
json::null_parser_t<StringT>::reset()
{
    state::set(state_t::initial);
    m_value.reset();
}

template <class StringT>
json::result_t
json::null_parser_t<StringT>::putchar(const char& c, const int pos)
{
    return parser_impl::step(to_event(c), c, pos);
}

template <class StringT>
json::value
json::null_parser_t<StringT>::get() const
{
    if (m_value.has_value())
        return *m_value;

    assert(0); // TODO: throw an exception
    return value();
};

template <class StringT>
const typename json::null_parser_t<StringT>::EventToStateTable_t&
json::null_parser_t<StringT>::table()
{
    return m_event_2_state_table;
}

template <class StringT>
typename json::null_parser_t<StringT>::event_t
json::null_parser_t<StringT>::to_event(const char& c) const
{
    switch (c)
    {
    case 0x6e:
        return event_t::letter_n;
    case 0x75:
        return event_t::letter_u;
    case 0x6c:
        return event_t::letter_l;
    }

    return event_t::other;
}

template <class StringT>
typename json::null_parser_t<StringT>::event_t
json::null_parser_t<StringT>::to_event(const result_t& c) const
{
    return event_t::other;
}

template <class StringT>
json::result_t
json::null_parser_t<StringT>::on_n(const unsigned char& c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::null_parser_t<StringT>::on_u(const unsigned char& c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::null_parser_t<StringT>::on_l(const unsigned char& c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::null_parser_t<StringT>::on_done(const unsigned char& c, const int pos)
{
    if (!m_value.has_value())
        m_value.emplace();

    (*m_value) = nullptr;

    return result_t::s_done;
};

template <class StringT>
json::result_t
json::null_parser_t<StringT>::on_fail(const unsigned char& c, const int pos)
{
    return result_t::e_unexpected;
}
#pragma endregion
//
#pragma region -- bool parser definition -- 
template <class StringT>
void
json::bool_parser_t<StringT>::reset()
{
    state::set(state_t::initial);
    m_str.clear();
    m_value.reset();
};

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::putchar(const char& c, const int pos)
{
    return parser_impl::step(to_event(c), c, pos);
};

template <class StringT>
json::value
json::bool_parser_t<StringT>::get() const
{
    if (m_value.has_value())
        return *m_value;

    assert(0); // TODO: throw an exception
    return value();
};

template <class StringT>
const typename json::bool_parser_t<StringT>::EventToStateTable_t&
json::bool_parser_t<StringT>::table()
{
    return m_event_2_state_table;
}

template <class StringT>
typename json::bool_parser_t<StringT>::event_t
json::bool_parser_t<StringT>::to_event(const char& c) const
{
    switch (c)
    {
    case 0x61:
        return event_t::letter_a;
    case 0x65:
        return event_t::letter_e;
    case 0x66:
        return event_t::letter_f;
    case 0x6c:
        return event_t::letter_l;
    case 0x72:
        return event_t::letter_r;
    case 0x73:
        return event_t::letter_s;
    case 0x74:
        return event_t::letter_t;
    case 0x75:
        return event_t::letter_u;
    }
    return event_t::symbol;
};

template <class StringT>
typename json::bool_parser_t<StringT>::event_t
json::bool_parser_t<StringT>::to_event(const result_t& c) const
{
    return event_t::symbol;
};

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_t(const unsigned char& c, const int pos)
{
    m_str += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_r(const unsigned char& c, const int pos)
{
    m_str += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_u(const unsigned char& c, const int pos)
{
    m_str += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_f(const unsigned char& c, const int pos)
{
    m_str += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_a(const unsigned char& c, const int pos)
{
    m_str += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_l(const unsigned char& c, const int pos)
{
    m_str += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_s(const unsigned char& c, const int pos)
{
    m_str += c;
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_done(const unsigned char& c, const int pos)
{
    m_str += c;

    auto update = [this](const bool val)->result_t
    {
        if (!m_value.has_value())
            m_value.emplace();

        (*m_value) = val;

        return result_t::s_done;
    };

    if (m_str == "true")
        return update(true);
    if (m_str == "false")
        return update(false);

    assert(0);

    return result_t::e_unexpected;
}

template <class StringT>
json::result_t
json::bool_parser_t<StringT>::on_fail(const unsigned char& c, const int pos)
{
    return result_t::e_unexpected;
}
#pragma endregion
//
#pragma region -- value parser definition --
template <class StringT>
void
json::value_parser_t<StringT>::reset()
{
    state::set(state_t::initial);
    for (ParserItem_t& p : parsing_unit)
        p.first = true, p.second->reset();
};

template <class StringT>
json::result_t
json::value_parser_t<StringT>::putchar(const char& c, const int pos)
{
    result_t r = parser_impl::step(to_event(c), c, pos);

    if (state::get() == state_t::read && (result_t::s_done == r || result_t::s_done_rpt == r))
    {
        result_t new_r = parser_impl::step(to_event(r), c, pos);
        assert(result_t::s_done == new_r);
        return  r != new_r ? r : new_r;
    }

    return r;
};

template <class StringT>
json::value
json::value_parser_t<StringT>::get() const
{
    for (auto cit = parsing_unit.cbegin(); cit != parsing_unit.cend(); ++cit)
    {
        if (true == cit->first)
        {
            return cit->second->get();
        }
    }

    assert(0); // TODO: throw an exception
    return value();
};

template <class StringT>
const typename json::value_parser_t<StringT>::EventToStateTable_t&
json::value_parser_t<StringT>::table()
{
    return m_event_2_state_table;
};

template <class StringT>
typename json::value_parser_t<StringT>::event_t
json::value_parser_t<StringT>::to_event(const char& c) const
{
    return event_t::symbol;
};

template <class StringT>
typename json::value_parser_t<StringT>::event_t
json::value_parser_t<StringT>::to_event(const result_t& c) const
{
    switch (state::get())
    {
    case state_t::read:
        if (result_t::s_done == c || result_t::s_done_rpt == c)
            return event_t::val_done;
        break;
    }

    return event_t::nothing;
};

template <class StringT>
json::result_t
json::value_parser_t<StringT>::on_data(const unsigned char& c, const int pos)
{
    result_t res = result_t::e_fatal;
    uint8_t parsers_in_work = 0;

    if (parsing_unit.empty())
    {
        parsing_unit.push_back(ParserItem_t(true, new null_parser_t<StringT>()));
        parsing_unit.push_back(ParserItem_t(true, new bool_parser_t<StringT>()));
        parsing_unit.push_back(ParserItem_t(true, new string_parser_t<StringT>()));
        parsing_unit.push_back(ParserItem_t(true, new number_parser_t<StringT>()));
        parsing_unit.push_back(ParserItem_t(true, new array_parser_t<StringT>()));
        parsing_unit.push_back(ParserItem_t(true, new object_parser_t<StringT>()));
    }

    for (std::pair<bool, parser::ptr>& p : parsing_unit)
    {
        if (true == p.first)
        {
            result_t local_res = p.second->putchar(c, pos);

            if (failed(local_res))
                p.first = false;
            else if (succeded(local_res) && (failed(res) || local_res < res))
                res = local_res, parsers_in_work += 1;
        }
    }

    if (0 == parsers_in_work)
        res = result_t::e_unexpected;

    return res;
}

template <class StringT>
json::result_t
json::value_parser_t<StringT>::on_done(const unsigned char& c, const int pos)
{
    return result_t::s_done;
}

template <class StringT>
json::result_t
json::value_parser_t<StringT>::on_fail(const unsigned char& c, const int pos)
{
    return result_t::e_unexpected;
}
#pragma endregion
//
#pragma region -- array parser definition -- 
template <class StringT>
void
json::array_parser_t<StringT>::reset()
{
    state::set(state_t::initial);

    if (!m_value_parser)
        m_value_parser.reset(new value_parser_t<StringT>());

    m_value_parser->reset();

    m_value.reset();
}

template <class StringT>
json::result_t
json::array_parser_t<StringT>::putchar(const char& c, const int pos)
{
    result_t r = parser_impl::step(to_event(c), c, pos);

    event_t e = to_event(r);

    if (event_t::nothing == e)
        return r;

    if (event_t::val_done == e)
    {
        result_t new_r = parser_impl::step(e, c, pos);
        r = result_t::s_need_more == new_r && result_t::s_done_rpt == r ?
            parser_impl::step(to_event(c), c, pos) :
            new_r;

        return r;
    }

    assert(0);

    return r;
}

template <class StringT>
json::value
json::array_parser_t<StringT>::get() const
{
    if (m_value.has_value())
        return *m_value;

    assert(0); // TODO: throw an exception
    return value();
}

// Inherited via parser_impl
template <class StringT>
const typename json::array_parser_t<StringT>::EventToStateTable_t&
json::array_parser_t<StringT>::table()
{
    return m_event_2_state_table;
}

template <class StringT>
typename json::array_parser_t<StringT>::event_t
json::array_parser_t<StringT>::to_event(const char& c) const
{
    auto is_space = [](const char& c)->bool
    {
        // space, tab, cr, lf
        return (0x20 == c || 0x09 == c || 0x0A == c || 0x0D == c);
    };

    switch (state::get())
    {
    case state_t::initial:
        if (0x5B == c) //[
            return event_t::arr_begin;
        if (is_space(c))
            return event_t::skip;
        break;
    case state_t::val_before:
        if (0x5D == c) //]
            return event_t::arr_end;
        if (is_space(c))
            return event_t::skip;
        break;
    case state_t::val_after:
        if (0x5D == c) //]
            return event_t::arr_end;
        if (0x2C == c) //,
            return event_t::comma;
        if (is_space(c))
            return event_t::skip;
        break;
    }

    return event_t::symbol;
}

template <class StringT>
typename json::array_parser_t<StringT>::event_t
json::array_parser_t<StringT>::to_event(const result_t& r) const
{
    switch (state::get())
    {
    case state_t::val_inside:
        if (result_t::s_done == r || result_t::s_done_rpt == r)
            return event_t::val_done;
        break;
    }

    return event_t::nothing;
}

// Own methods
template <class StringT>
json::result_t
json::array_parser_t<StringT>::on_more(const unsigned& c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::array_parser_t<StringT>::on_begin(const unsigned char& c, const int pos)
{
    if (!m_value.has_value())
        m_value.emplace();

    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::array_parser_t<StringT>::on_new(const unsigned char& c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::array_parser_t<StringT>::on_val(const unsigned& c, const int pos)
{
    return m_value_parser->putchar(c, pos);
}

template <class StringT>
json::result_t
json::array_parser_t<StringT>::on_got_val(const char& c, const int pos)
{
    assert(m_value.has_value());

    const value val = m_value_parser->get();

    (*m_value).push_back(val);

    m_value_parser->reset();

    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::array_parser_t<StringT>::on_done(const unsigned char& c, const int pos)
{
    assert(m_value.has_value());

    return result_t::s_done;
}

template <class StringT>
json::result_t
json::array_parser_t<StringT>::on_fail(const unsigned char& c, const int pos)
{
    return result_t::e_unexpected;
}
#pragma endregion
//
#pragma region -- object parser definition --
template <class StringT>
void
json::object_parser_t<StringT>::reset()
{
    state::set(state_t::initial);

    if (!m_key_parser)
        m_key_parser.reset(new string_parser_t<StringT>());
    else
        m_key_parser->reset();

    if (!m_val_parser)
        m_val_parser.reset(new value_parser_t<StringT>());
    else
        m_val_parser->reset();

    m_value.reset();
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::putchar(const char& c, const int pos)
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

template <class StringT>
json::value
json::object_parser_t<StringT>::get() const
{
    if (m_value.has_value())
        return *m_value;

    assert(0); // TODO: throw an exception
    return value();
};

// inherited via parser_impl
template <class StringT>
const typename json::object_parser_t<StringT>::EventToStateTable_t&
json::object_parser_t<StringT>::table()
{
    return m_event_2_state_table;
}

template <class StringT>
typename json::object_parser_t<StringT>::event_t
json::object_parser_t<StringT>::to_event(const char& c) const
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

template <class StringT>
typename json::object_parser_t<StringT>::event_t
json::object_parser_t<StringT>::to_event(const result_t& r) const
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

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_more(const char& c, const int pos)
{
    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_begin(const char& c, const int pos)
{
    if (!m_key_parser)
        m_key_parser.reset(new string_parser_t<StringT>());

    if (!m_val_parser)
        m_val_parser.reset(new value_parser_t<StringT>());

    if (!m_value.has_value())
        m_value.emplace();

    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_new(const char& c, const int pos)
{
    if (!m_key_parser)
        m_key_parser.reset(new string_parser_t<StringT>());
    else
        m_key_parser->reset();

    if (!m_val_parser)
        m_val_parser.reset(new value_parser_t<StringT>());
    else
        m_val_parser->reset();

    return result_t::s_need_more;
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_key(const char& c, const int pos)
{
    return m_key_parser->putchar(c, pos);
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_val(const char& c, const int pos)
{
    return m_val_parser->putchar(c, pos);
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_done(const char& c, const int pos)
{
    return result_t::s_done;
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_fail(const char& c, const int pos)
{
    m_value.reset();
    return result_t::e_unexpected;
}

template <class StringT>
json::result_t
json::object_parser_t<StringT>::on_got_val(const char& c, const int pos)
{
    assert(m_value.has_value());

    const string_t key = std::get<string_t>(m_key_parser->get());
    const value val = m_val_parser->get();

    (*m_value)[key] = val;

    return result_t::s_need_more;
}
#pragma endregion

#endif // __JSON_LIB_H__