//////////////////////////////////////////////////////////////////////////
// Author: Ilya Malyavskiy (ilya.malyavskiy@gmail.com)
// Changelog:   2017/11/02 - first std based non templated version
//              2017/11/07 - templated version
// Desciption: This is a simple type independent(or at least is was desined as type independent) JSON (de)serilization library.
// Class list:
//      json_t          
//            - The top level class. Contains all the data representation, interface and worker classes.
//      container
//            - Base data representation container class
//      object(inherits container)
//            - JSON object representation class. Cosists of a key:value pairs.
//      array(inherits container)
//            - JSON array representaion class. Cosists of a values.
//      parser
//            - Interface class for all parsers.
//      parser_impl(inherits parser)
//            - Base class for all parsers. Incapsulates finite automata state, step and creates abstract methods mandatory for all parsers. 
//              Inherits parser.
//      string_parser_t(inherits praser_impl)
//            - String parser class. Implements logics of the string parser.
//      number_parser_t(inherits praser_impl)
//            - Number parser class. Implements logics of the number parser.
//      null_parser_t(inherits praser_impl)
//            - Null parser class. Implements logics of the null value parser.
//      bool_parser_t(inherits praser_impl)
//            - Bool parser class. Implements logics of the boolean values(true, false) parser.
//      value_parser_t(inherits praser_impl)
//            - Abstract value parser class. Unites all parsers inside because value can be null, bool, number, string, array, object.
//      array_parser_t(inherits praser_impl)
//            - Arrray parser class. Implements logic for the JSON array to be parsed. Inherits parser_impl. Based on std::vector similar class.
//      object_parser_t(inherits praser_impl)
//            - Object parser class. Implements logic for the JSON object to be parsed. Inherits parser_impl. Based on std::map similar class.
//            - The root parser of the library because any JSON represents an object.
//////////////////////////////////////////////////////////////////////////

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
#include <sstream>
#include <string>
#include <vector>

#if _HAS_CXX17
#include <optional>
#include <variant>
#else 
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#endif

#define STD_BIND_TO_THIS(__CLASS__, __METHOD__) std::bind(&__CLASS__::__METHOD__, this, std::placeholders::_1, std::placeholders::_2)

#define JSON_TEMPLATE_PARAMS                                              \
template <                                                                \
    class SymbolT,                                                        \
    class IntegerT,                                                       \
    class FloatingPtT,                                                    \
    class BooleanT,                                                       \
    class NullT,                                                          \
    template <class _Kty, class _Ty> class PairT,                         \
    template <class _Ty> class LessT,                                     \
    template <class _Elem> class CharTraitsT,                             \
    template <class _Ty, class _Alloc> class VectorT,                     \
    template <class _Ty, class _Alloc> class ListT,                       \
    template <class _Kty, class _Ty, class _Pr, class _Alloc> class MapT, \
    template <class _Elem, class _Traits, class _Alloc> class StringT,    \
    template <class _Elem, class _Traits, class _Alloc> class StrStrmT,   \
    template <class _Elem, class _Traits> class IStrmT,                   \
    template <class _Ty> class AllocatorT                                 \
>
#define JSON_TEMPLATE_CLASS \
json_t< SymbolT,            \
        IntegerT,           \
        FloatingPtT,        \
        BooleanT,           \
        NullT,              \
        PairT,              \
        LessT,              \
        CharTraitsT,        \
        VectorT,            \
        ListT,              \
        MapT,               \
        StringT,            \
        StrStrmT,           \
        IStrmT,             \
        AllocatorT          \
>
namespace imalyavskiy
{
    template <
        class SymbolT           = char,
        class IntegerT          = int64_t,
        class FloatingPtT       = double,
        class BooleanT          = bool,
        class NullT             = nullptr_t,
        template <class _Kty, class _Ty>
            class PairT         = std::pair,
        template <class _Ty>
            class LessT         = std::less,
        template <class _Elem>
            class CharTraitsT   = std::char_traits,
        template <class _Ty, class _Alloc> 
            class VectorT       = std::vector, 
        template <class _Ty, class _Alloc>
            class ListT         = std::list,
        template <class _Kty, class _Ty, class _Pr, class _Alloc>
            class MapT          = std::map,
        template <class _Elem, class _Traits, class _Alloc>
            class StringT       = std::basic_string,
        template <class _Elem, class _Traits, class _Alloc>
            class StrStrmT      = std::basic_stringstream,
        template <class _Elem, class _Traits>
            class IStrmT        = std::basic_istream,
        template <class _Ty>
            class AllocatorT    = std::allocator
    >
    class json_t
    {
    public:
        using symbol_t          = SymbolT;
        using integer_t         = IntegerT;
        using floatingpt_t      = FloatingPtT;
        using boolean_t         = BooleanT;
        using null_t            = NullT;

        template <class _Ty> 
            using allocator_t   = AllocatorT<_Ty>;
        template <class _Kty, class _Ty> 
            using pair_t        = PairT<_Kty, _Ty>;
        template <class _Elem>
            using char_traits_t = CharTraitsT<_Elem>;
        template <class _Ty>
            using less_t        = LessT<_Ty>;
        template <class _Ty, class _Alloc = allocator_t<_Ty>> 
            using vector_t      = VectorT<_Ty, _Alloc>;
        template <class _Ty, class _Alloc = allocator_t<_Ty>> 
            using list_t        = ListT<_Ty, _Alloc>;
        template <class _Kty, class _Ty, class _Pr = less_t<_Kty>, class _Alloc = allocator_t<pair_t<const _Kty, _Ty>>> 
            using map_t         = MapT<_Kty, _Ty, _Pr, _Alloc>;
        template <class _Elem, class _Traits = char_traits_t<_Elem>, class _Alloc = allocator_t<_Elem>> 
            using string_t      = StringT<_Elem, _Traits, _Alloc>;
            using string = string_t<symbol_t>;
        template <class _Elem, class _Traits = char_traits_t<_Elem>, class _Alloc = allocator_t<_Elem>>
            using sstream_t     = StrStrmT<_Elem, _Traits, _Alloc>;
            using sstream = sstream_t<symbol_t>;
        template <class _Elem, class _Traits = char_traits_t<_Elem>>
            using istream_t     = IStrmT<_Elem, _Traits>;
            using istream = istream_t<symbol_t>;

        /// Forward declaration for JSON object data structure
        class obj;

        /// Forward declaration for JSON array data structure
        class arr;

        /// possible results
        enum class result_t
        {
            s_need_more     =  3, // Need more data.
            s_done_rpt      =  2, // Symbol succesfully terminates parsing but makes no sense to current parser. Reparse symbol by above parser.
            s_done          =  1, // Symbol succesfully terminates parsing and makes sense to current parser.
            s_ok            =  0, // General success.
            e_fatal         = -1, // General failure.
            e_unexpected    = -2, // Unexpected parameter or value.
        };

        inline static boolean_t failed(const result_t& r) { return r < result_t::s_ok; }
        inline static boolean_t succeded(const result_t& r) { return r >= result_t::s_ok; }

        static result_t parse(istream& input, obj& jsobj)
        {
            symbol_t c = 0;
            result_t result = result_t::s_ok;

            typename parser::ptr p = create();
            if (!p)
                return result_t::e_fatal;

            while (input >> std::noskipws >> c || result != result_t::s_done)
            {
                result = p->putchar(c, (int)input.tellg() - 1);

                if (result_t::s_ok > result)
                    break;

                if (result_t::s_done == result)
                {
                    jsobj = p->get().get<obj>();
                    break;
                }
            }

            return result;
        }

        static result_t parse(const string& input, obj& jsobj)
        {
            sstream sstr;
            sstr.str(input);
            return parse(sstr, jsobj);
        }

    #pragma region -- value definition --
        class value
#if _HAS_CXX17
            : public std::variant<string, obj, arr, integer_t, floatingpt_t, boolean_t, null_t>
#else
            : public boost::variant<string, obj, arr, integer_t, floatingpt_t, boolean_t, null_t>
#endif
        {
#if _HAS_CXX17
            using base_t = std::variant<string, obj, arr, integer_t, floatingpt_t, boolean_t, null_t>;
#else
            using base_t = boost::variant<string, obj, arr, integer_t, floatingpt_t, boolean_t, null_t>;
#endif
        public:
            /// The enumeration for mnemonic correlation of indeces and types used for std::variant
            enum class vt
            {   /// IMPORTANT: The order of parameters is the same as value_t template parameters order
                t_string     = 0,
                t_object     = 1,
                t_array      = 2,
                t_integer    = 3,
                t_floatingpt = 4,
                t_boolean    = 5,
                t_null       = 6,
            };

            /// {ctor}s
            value() { }
            value(const string& other)      : base_t(other)         {}
            value(const obj& other)         : base_t(other)         {}
            value(const arr&  other)        : base_t(other)         {}
            value(const integer_t   other)  : base_t(other)         {}
            value(const floatingpt_t other) : base_t(other)         {}
            value(const boolean_t other)    : base_t(other)         {}
            value(const symbol_t* other)    : base_t(string(other)) {}
            value(const null_t other)       : base_t(other)         {}

#if _HAS_CXX17
            vt index() const {
                return (vt)base_t::index();
            }
            template <class T>
            T get(T* t = nullptr) const {
                return std::get<T>(*this);
            }
#else            
            vt index() const {
                return (vt)base_t::which();
            }
            template <class T>
            T get(T* t = nullptr) const {
                return boost::get<T>(*this);
            }
#endif

            /// Assign operators
            const value& operator=(const string& other)
            {
                base_t::operator=(other);
                return (*this);
            }

            const value& operator=(const obj& other)
            {
                base_t::operator=(other);
                return (*this);
            }

            const value& operator=(const arr& other)
            {
                base_t::operator=(other);
                return (*this);
            }

            const value& operator=(const integer_t other)
            {
                base_t::operator=(other);
                return (*this);
            }

            const value& operator=(const floatingpt_t other)
            {
                base_t::operator=(other);
                return (*this);
            }

            const value& operator=(const boolean_t other)
            {
                base_t::operator=(other);
                return (*this);
            }

            const value& operator=(const symbol_t* other)
            {
                base_t::operator=(string(other));
                return (*this);
            }

            const value& operator=(const null_t other)
            {
                base_t::operator=(other);
                return (*this);
            }

            inline bool is_string()     const { return vt::t_string     == index(); }
            inline bool is_object()     const { return vt::t_object     == index(); }
            inline bool is_array()      const { return vt::t_array      == index(); }
            inline bool is_integer()    const { return vt::t_integer    == index(); }
            inline bool is_floatingpt() const { return vt::t_floatingpt == index(); }
            inline bool is_number()     const { return is_integer() || is_floatingpt(); };
            inline bool is_boolean()    const { return vt::t_boolean    == index(); }
            inline bool is_null()       const { return vt::t_null       == index(); }

            operator string() const
            {
                if (index() != vt::t_string)
                    assert(0);

                return get<string>();
            }

            operator obj() const
            {
                if (index() != vt::t_object)
                    assert(0);

                return get<obj>();
            }

            operator arr() const
            {
                if (index() != vt::t_array)
                    assert(0);

                return get<arr>();
            }

            operator int64_t() const
            {
                if (index() != vt::t_integer)
                    assert(0);

                return (int64_t)get<integer_t>();
            }

            operator int32_t() const
            {
                return (int32_t)operator int64_t();
            }

            operator int16_t() const
            {
                return (int16_t)operator int64_t();
            }

            operator floatingpt_t() const
            {
                if (index() != vt::t_floatingpt)
                    assert(0);

                return get<floatingpt_t>();
            }

            operator boolean_t() const
            {
                if (index() != vt::t_boolean)
                    assert(0);

                return get<boolean_t>();
            }

            operator null_t() const
            {
                if (index() != vt::t_null)
                    assert(0);

                return get<null_t>();
            }
        };

    #pragma endregion
    //
    #pragma region -- json data declaration --
        template<class BaseType>
        class container
            : public BaseType
        {
        public:
            using my_base_t = BaseType;

            /// {ctor}s
            container() = default;
            container(std::initializer_list<value> l) : BaseType(l) {}

            virtual const string str(sstream& str = stream()) const = 0;

            protected:
                string serialize_string(const string& s) const
                {
                    string r;

                    for (symbol_t c : s)
                    {
                        switch (c)
                        {
                        case '\b':          // backspace
                            r += "\\b";
                            break;
                        case '\f':          // form feed
                            r += "\\f";
                            break;
                        case '\n':          // new line
                            r += "\\n";
                            break;
                        case '\r':          // carriage return
                            r += "\\r";
                            break;
                        case '\t':          // horizontal tab
                            r += "\\t";
                            break;
                        case '\\':          // back slash
                            r += "\\\\";
                            break;
                        case '/':           // forward slash
                            r += "\\/";
                            break;
                        // TODO: needed to properly serialize unicode(2-byte) symbols
                        default:
                            r.push_back(c); // all other
                            break;
                        }
                    }

                    return r;
                }
        };

        /// Declaration of the object JSON data structure
        class obj 
            : public container<map_t<string, value>>
        {
        public:
            // constant iterator type 
            typedef typename container<map_t<string, value>>::my_base_t::const_iterator cit;

            // iterator type
            typedef typename container<map_t<string, value>>::my_base_t::iterator it;

            // default constructor
            obj() = default;

            // initializers list constructor
            obj(std::initializer_list<pair_t<string, value>> l);

            // random access operator 
            value& operator[](const string& key)
            {
                return container::operator[](key);
            }

            // constant random access operator
            const value& operator[](const string& key) const
            {
                return container::at(key);
            }

            // certain key presence test
            boolean_t exists(const string& key) const
            {
                return end() != find(key);
            }

            // convert to string
            operator string() const
            {
                return str();
            }

            /// serialization
            virtual const string str(sstream& str = sstream()) const final;
        };

        /// Declaration of the array JSON data structure
        class arr 
            : public container<vector_t<value>>
        {
        public:
            // constant iterator type
            typedef typename container<vector_t<value>>::my_base_t::const_iterator cit;

            // iterator
            typedef typename container<vector_t<value>>::my_base_t::iterator it;

            // default constructor type
            arr() = default;

            // initializer list constructor
            arr(std::initializer_list<value> l);

            // convert to string
            operator string() const
            {
                return str();
            }

            // serialization
            virtual const string str(sstream& str = sstream()) const final;
        };
    #pragma endregion
    //
    #pragma region -- parser interface --
        /// Common parser interface
        class parser
        {
        public:
            typedef std::shared_ptr<parser> ptr;

            virtual ~parser() {};

            /// Drops the internal state to initial(i.e. as just constructed)
            virtual void        reset() = 0;

            /// Puts a character to the parsing routine
            virtual result_t    putchar(const symbol_t& c, const int pos) = 0;

            /// Retrieves the parsing result
            virtual value       get() const = 0;
        };
    #pragma endregion
    //
    #pragma region -- parser_base -- 
    #pragma region -- state machine types --
        using state_change_handler_t = std::function<result_t(const symbol_t&, const int)>;

        template <typename READSTATE, typename _STATE_CHANGE_HANDLER>
        using TTransition = pair_t<READSTATE, _STATE_CHANGE_HANDLER>;

        template<typename STATE, typename STATE_CHANGE_HANDLER>
        using Transition = TTransition<STATE, STATE_CHANGE_HANDLER>;

        template <typename EVENT, typename TRANSITION>
        using TTransitionTable = map_t<typename EVENT, typename TRANSITION>;

        template<typename STATE, typename EVENT, typename STATE_CHANGE_HANDLER>
        using TransitionTable = TTransitionTable<EVENT, Transition<STATE, STATE_CHANGE_HANDLER>>;

        template <typename READSTATE, typename TRANSITION_TABLE = TransitionTable>
        using TStateTable = map_t<READSTATE, TRANSITION_TABLE>;

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

        template<class ValueT, class EventsT, class StateT, StateT initial_state>
        class parser_impl
            : protected state<StateT, initial_state>
            , public parser
        {
        public:
            using event_t = EventsT;
            using StateTable_t = StateTable<StateT, EventsT>;

            parser_impl() {};

            // The step of the automata
            result_t step(const event_t& e, const symbol_t& c, const int pos)
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
            virtual event_t to_event(const symbol_t& c) const = 0;
            virtual event_t to_event(const result_t& c) const = 0;

            virtual const StateTable_t& table() = 0;

            template <class V>
            class optional
                : public
#if _HAS_CXX17
                std::optional<V>
#else
                boost::optional<V>
#endif
            {
            public:
                operator bool() const
                {
#if _HAS_CXX17
                    return has_value();
#else
                    return is_initialized();
#endif
                }
            };
            
            optional<ValueT> m_value;
        };
    #pragma endregion
    //////////////////////////////////////////////////////////////////////////
    #pragma region -- string parser declaration -- 
        enum class e_string_states
        {
            initial,    // wait for " and skipping space charscters - space, hrisontal tab, crlf, lf
            inside,
            escape,
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
            alpha_b,    // stands for backspace
            alpha_f,    // stands for form feed
            alpha_n,    // stands for new line
            alpha_r,    // stands for carriage return
            alpha_t,    // stands for tab
            alpha_u,    // stands for unicode
        };

        class string_parser_t
            : public parser_impl<string, e_string_events, e_string_states, e_string_states::initial>
        {
        public:
            using event_t               = e_string_events;
            using state_t               = e_string_states;
            using EventToStateTable_t   = StateTable<state_t, event_t>;

            string_parser_t()
                : m_event_2_state_table
            {
                { state_t::initial,   { { event_t::quote,      { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_initial ) } },
                                        { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
                { state_t::inside,    { { event_t::quote,      { state_t::done,      STD_BIND_TO_THIS( string_parser_t, on_done    ) } },
                                        { event_t::back_slash, { state_t::escape,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::symbol,     { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_inside  ) } },
                } },
                { state_t::escape,    { { event_t::quote,      { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::back_slash, { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::slash,      { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::alpha_b,    { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::alpha_f,    { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::alpha_n,    { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::alpha_r,    { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::alpha_t,    { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_escape  ) } },
                                        { event_t::alpha_u,    { state_t::unicode_1, STD_BIND_TO_THIS( string_parser_t, on_unicode ) } },
                                        { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
                { state_t::unicode_1, { { event_t::hex_digit,  { state_t::unicode_2, STD_BIND_TO_THIS( string_parser_t, on_unicode ) } },
                                        { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
                { state_t::unicode_2, { { event_t::hex_digit,  { state_t::unicode_3, STD_BIND_TO_THIS( string_parser_t, on_unicode ) } },
                                        { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
                { state_t::unicode_3, { { event_t::hex_digit,  { state_t::unicode_4, STD_BIND_TO_THIS( string_parser_t, on_unicode ) } },
                                        { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
                { state_t::unicode_4, { { event_t::hex_digit,  { state_t::inside,    STD_BIND_TO_THIS( string_parser_t, on_inside  ) } },
                                        { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
                { state_t::done,      { { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
                { state_t::failure,   { { event_t::symbol,     { state_t::failure,   STD_BIND_TO_THIS( string_parser_t, on_fail    ) } },
                } },
            } {};

        protected:
            // Inherited via parser
            virtual void reset() final;

            virtual result_t putchar(const symbol_t& c, const int pos) final;

            virtual value get() const final;

            // Inherited via parser_impl
            virtual const EventToStateTable_t& table() override;

            virtual event_t to_event(const symbol_t& c) const override;

            virtual event_t to_event(const result_t& c) const override;

            // Own methods
            result_t on_initial(const symbol_t&c, const int pos);

            result_t on_inside(const symbol_t&c, const int pos);

            result_t on_escape(const symbol_t&c, const int pos);

            result_t on_unicode(const symbol_t&c, const int pos);

            result_t on_done(const symbol_t&c, const int pos);

            result_t on_fail(const symbol_t&c, const int pos);

        protected:
            const EventToStateTable_t m_event_2_state_table;

            string m_cache;
        };
    #pragma endregion
    //
    #pragma region -- number parser declaration -- 
        enum class e_number_states
        {
            initial,
            leading_minus,
            zero,
            decimal_dot,
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
            minus,      // -
            plus,       // +
            dec_zero,   // 0
            dec_digit,  // 0x31 - 0x39
            dot,        // .
            exponent,   // E or 0x65 - e
            symbol,
        };

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

            boolean_t   m_positive;
            integer_t   m_integer;
            integer_t   m_fractional_value;
            integer_t   m_fractional_digits;
            boolean_t   m_has_exponent;
            boolean_t   m_exponent_positive;
            integer_t   m_exponent_value;
        };

        class number_parser_t
            : public parser_impl<number, e_number_events, e_number_states, e_number_states::initial>
        {
            using event_t = e_number_events;
            using state_t = e_number_states;
            using EventToStateTable_t = StateTable<state_t, event_t>;
        
        public:
            number_parser_t()
                : m_event_2_state_table
            {
                { state_t::initial,        { { event_t::minus,     { state_t::leading_minus,  STD_BIND_TO_THIS( number_parser_t, on_minus      ) } },
                                             { event_t::dec_zero,  { state_t::zero,           STD_BIND_TO_THIS( number_parser_t, on_zero       ) } },
                                             { event_t::dec_digit, { state_t::integer,        STD_BIND_TO_THIS( number_parser_t, on_integer    ) } },
                                             { event_t::symbol,    { state_t::failure,        STD_BIND_TO_THIS( number_parser_t, on_fail       ) } },
                } },
                { state_t::leading_minus,  { { event_t::dec_zero,  { state_t::zero,           STD_BIND_TO_THIS( number_parser_t, on_zero       ) } },
                                             { event_t::dec_digit, { state_t::integer,        STD_BIND_TO_THIS( number_parser_t, on_integer    ) } },
                                             { event_t::symbol,    { state_t::failure,        STD_BIND_TO_THIS( number_parser_t, on_fail       ) } },
                } },
                { state_t::zero,           { { event_t::dot,       { state_t::decimal_dot,    STD_BIND_TO_THIS( number_parser_t, on_dot        ) } },
                                             { event_t::symbol,    { state_t::done,           STD_BIND_TO_THIS( number_parser_t, on_done       ) } },
                } },
                { state_t::decimal_dot,    { { event_t::dec_zero,  { state_t::fractional,     STD_BIND_TO_THIS( number_parser_t, on_fractional ) } },
                                             { event_t::dec_digit, { state_t::fractional,     STD_BIND_TO_THIS( number_parser_t, on_fractional ) } },
                                             { event_t::symbol,    { state_t::failure,        STD_BIND_TO_THIS( number_parser_t, on_fail       ) } },
                } },
                { state_t::integer,        { { event_t::dec_zero,  { state_t::integer,        STD_BIND_TO_THIS( number_parser_t, on_integer    ) } },
                                             { event_t::dec_digit, { state_t::integer,        STD_BIND_TO_THIS( number_parser_t, on_integer    ) } },
                                             { event_t::dot,       { state_t::decimal_dot,    STD_BIND_TO_THIS( number_parser_t, on_dot        ) } },
                                             { event_t::symbol,    { state_t::done,           STD_BIND_TO_THIS( number_parser_t, on_done       ) } },
                } },
                { state_t::fractional,     { { event_t::dec_zero,  { state_t::fractional,     STD_BIND_TO_THIS( number_parser_t, on_fractional ) } },
                                             { event_t::dec_digit, { state_t::fractional,     STD_BIND_TO_THIS( number_parser_t, on_fractional ) } },
                                             { event_t::exponent,  { state_t::exponent_delim, STD_BIND_TO_THIS( number_parser_t, on_exponent   ) } },
                                             { event_t::symbol,    { state_t::done,           STD_BIND_TO_THIS( number_parser_t, on_done       ) } },
                } },
                { state_t::exponent_delim, { { event_t::minus,     { state_t::exponent_sign,  STD_BIND_TO_THIS( number_parser_t, on_exp_sign   ) } },
                                             { event_t::plus,      { state_t::exponent_sign,  STD_BIND_TO_THIS( number_parser_t, on_exp_sign   ) } },
                                             { event_t::dec_zero,  { state_t::exponent_delim, STD_BIND_TO_THIS( number_parser_t, on_exp_value  ) } },
                                             { event_t::dec_digit, { state_t::exponent_delim, STD_BIND_TO_THIS( number_parser_t, on_exp_value  ) } },
                                             { event_t::symbol,    { state_t::failure,        STD_BIND_TO_THIS( number_parser_t, on_fail       ) } },
                } },
                { state_t::exponent_sign,  { { event_t::dec_zero,  { state_t::exponent_val,   STD_BIND_TO_THIS( number_parser_t, on_exp_value  ) } },
                                             { event_t::dec_digit, { state_t::exponent_val,   STD_BIND_TO_THIS( number_parser_t, on_exp_value  ) } },
                                             { event_t::symbol,    { state_t::failure,        STD_BIND_TO_THIS( number_parser_t, on_fail       ) } },
                } },
                { state_t::exponent_val,   { { event_t::dec_zero,  { state_t::exponent_val,   STD_BIND_TO_THIS( number_parser_t, on_exp_value  ) } },
                                             { event_t::dec_digit, { state_t::exponent_val,   STD_BIND_TO_THIS( number_parser_t, on_exp_value  ) } },
                                             { event_t::symbol,    { state_t::done,           STD_BIND_TO_THIS( number_parser_t, on_done       ) } },
                } },
                { state_t::done,           { { event_t::symbol,    { state_t::failure,        STD_BIND_TO_THIS( number_parser_t, on_fail       ) } },
                } },
                { state_t::failure,        { { event_t::symbol,    { state_t::failure,        STD_BIND_TO_THIS( number_parser_t, on_fail       ) } },
                } },
            } {};

        protected:
            // Inherited via parser
            virtual void reset() final;

            virtual result_t putchar(const symbol_t& c, const int pos) final;

            virtual value get() const final;

            // Inherited via parser_impl
            virtual const EventToStateTable_t& table() override;

            virtual event_t to_event(const symbol_t& c) const override;

            virtual event_t to_event(const result_t& c) const override;

            // Own methods
            result_t on_initial(const symbol_t& c, const int pos);

            result_t on_minus(const symbol_t& c, const int pos);

            result_t on_integer(const symbol_t& c, const int pos);

            result_t on_fractional(const symbol_t& c, const int pos);

            result_t on_exponent(const symbol_t& c, const int pos);

            result_t on_exp_sign(const symbol_t& c, const int pos);

            result_t on_exp_value(const symbol_t& c, const int pos);


            result_t on_zero(const symbol_t& c, const int pos);

            result_t on_dot(const symbol_t& c, const int pos);

            result_t on_done(const symbol_t& c, const int pos);

            result_t on_fail(const symbol_t& c, const int pos);

            static result_t append_digit(integer_t& val, const symbol_t& c);

        protected:
            const EventToStateTable_t m_event_2_state_table;
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
            done,
            failure,
        };

        enum class e_null_events
        {
            // ascii part
            letter_n,   // n
            letter_u,   // u
            letter_l,   // l
            other,
        };

        class null_parser_t
            : public parser_impl<null_t, e_null_events, e_null_states, e_null_states::initial>
        {
            using event_t = e_null_events;
            using state_t = e_null_states;
            using EventToStateTable_t = StateTable<state_t, event_t>;

        public:
            null_parser_t()
                : m_event_2_state_table
            {
                { state_t::initial, { { event_t::letter_n, { state_t::got_n,   STD_BIND_TO_THIS(null_parser_t, on_n    ) } },
                                      { event_t::other,    { state_t::failure, STD_BIND_TO_THIS(null_parser_t, on_fail ) } },
                } },
                { state_t::got_n,   { { event_t::letter_u, { state_t::got_u,   STD_BIND_TO_THIS(null_parser_t, on_u    ) } },
                                      { event_t::other,    { state_t::failure, STD_BIND_TO_THIS(null_parser_t, on_fail ) } },
                } },
                { state_t::got_u,   { { event_t::letter_l, { state_t::got_l,   STD_BIND_TO_THIS(null_parser_t, on_l    ) } },
                                      { event_t::other,    { state_t::failure, STD_BIND_TO_THIS(null_parser_t, on_fail ) } },
                } },
                { state_t::got_l,   { { event_t::letter_l, { state_t::done,    STD_BIND_TO_THIS(null_parser_t, on_done ) } },
                                      { event_t::other,    { state_t::failure, STD_BIND_TO_THIS(null_parser_t, on_fail ) } },
                } },
                { state_t::done,    { { event_t::other,    { state_t::failure, STD_BIND_TO_THIS(null_parser_t, on_fail ) } },
                } },
                { state_t::failure, { { event_t::other,    { state_t::failure, STD_BIND_TO_THIS(null_parser_t, on_fail ) } },
                } },
            }
            {};

        protected:
            // Inherited via parser
            virtual void reset() final;

            virtual result_t putchar(const symbol_t& c, const int pos) final;

            virtual value get() const final;

            // Inherited via parser_impl
            virtual const EventToStateTable_t& table() override;

            virtual event_t to_event(const symbol_t& c) const override;

            virtual event_t to_event(const result_t& c) const override;

            // Own methods
            result_t on_n(const symbol_t& c, const int pos);

            result_t on_u(const symbol_t& c, const int pos);

            result_t on_l(const symbol_t& c, const int pos);

            result_t on_done(const symbol_t& c, const int pos);

            result_t on_fail(const symbol_t& c, const int pos);

        protected:
            const EventToStateTable_t m_event_2_state_table;
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

        class bool_parser_t
            : public parser_impl<boolean_t, e_bool_events, e_bool_states, e_bool_states::initial>
        {
            using event_t = e_bool_events;
            using state_t = e_bool_states;
            using EventToStateTable_t = StateTable<state_t, event_t>;
        public:
            bool_parser_t()
                : m_event_2_state_table
            {
                { state_t::initial, { { event_t::letter_t, { state_t::got_t,   STD_BIND_TO_THIS( bool_parser_t, on_t    ) } },
                                      { event_t::letter_f, { state_t::got_f,   STD_BIND_TO_THIS( bool_parser_t, on_f    ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::got_t,   { { event_t::letter_r, { state_t::got_r,   STD_BIND_TO_THIS( bool_parser_t, on_r    ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::got_r,   { { event_t::letter_u, { state_t::got_u,   STD_BIND_TO_THIS( bool_parser_t, on_u    ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::got_u,   { { event_t::letter_e, { state_t::done,    STD_BIND_TO_THIS( bool_parser_t, on_done ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::got_f,   { { event_t::letter_a, { state_t::got_a,   STD_BIND_TO_THIS( bool_parser_t, on_a    ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::got_a,   { { event_t::letter_l, { state_t::got_l,   STD_BIND_TO_THIS( bool_parser_t, on_l    ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::got_l,   { { event_t::letter_s, { state_t::got_s,   STD_BIND_TO_THIS( bool_parser_t, on_s    ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::got_s,   { { event_t::letter_e, { state_t::done,    STD_BIND_TO_THIS( bool_parser_t, on_done ) } },
                                      { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::done,    { { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
                { state_t::failure, { { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( bool_parser_t, on_fail ) } },
                } },
            } {};

        protected:
            // Inherited via parser
            virtual void reset() final;

            virtual result_t putchar(const symbol_t& c, const int pos) final;

            virtual value get() const final;

            // Inherited via parser_impl
            virtual const EventToStateTable_t& table() override;

            virtual event_t to_event(const symbol_t& c) const override;

            virtual event_t to_event(const result_t& c) const override;

            // Own methods
            result_t on_t(const symbol_t& c, const int pos);

            result_t on_r(const symbol_t& c, const int pos);

            result_t on_u(const symbol_t& c, const int pos);

            result_t on_f(const symbol_t& c, const int pos);

            result_t on_a(const symbol_t& c, const int pos);

            result_t on_l(const symbol_t& c, const int pos);

            result_t on_s(const symbol_t& c, const int pos);

            result_t on_done(const symbol_t& c, const int pos);

            result_t on_fail(const symbol_t& c, const int pos);

        protected:
            const EventToStateTable_t m_event_2_state_table;

            string m_str;
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

        class value_parser_t
            : public parser_impl<nullptr_t, e_value_events, e_value_states, e_value_states::initial>
        {
            using event_t               = e_value_events;
            using state_t               = e_value_states;
            using EventToStateTable_t   = StateTable<state_t, event_t>;
            using ParserItem_t          = pair_t<boolean_t, parser::ptr>;
        public:
            value_parser_t()
                : m_event_2_state_table
            {
                { state_t::initial, { { event_t::symbol,   { state_t::read,    STD_BIND_TO_THIS( value_parser_t, on_data ) } },
                } },
                { state_t::read,    { { event_t::symbol,   { state_t::read,    STD_BIND_TO_THIS( value_parser_t, on_data ) } },
                                      { event_t::val_done, { state_t::done,    STD_BIND_TO_THIS( value_parser_t, on_done ) } },
                } },
                { state_t::done,    { { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( value_parser_t, on_fail ) } },
                } },
                { state_t::failure, { { event_t::symbol,   { state_t::failure, STD_BIND_TO_THIS( value_parser_t, on_fail ) } },
                } },
            } {};

        protected:
            // Inherited via parser
            virtual void reset() final;

            virtual result_t putchar(const symbol_t& c, const int pos) final;

            virtual value get() const final;

            // Inherited via parser_impl
            virtual const EventToStateTable_t& table() override;

            virtual event_t to_event(const symbol_t& c) const override;

            virtual event_t to_event(const result_t& c) const override;

            // Own methods
            result_t on_data(const symbol_t& c, const int pos);

            result_t on_done(const symbol_t& c, const int pos);

            result_t on_fail(const symbol_t& c, const int pos);

        protected:
            const EventToStateTable_t m_event_2_state_table;

            std::list<ParserItem_t> parsing_unit;
        };
    #pragma endregion
    //
    #pragma region -- array parser declaration -- 
        enum class e_array_states
        {
            initial,    //
            val_before, //
            val_inside, //
            val_after,  //
            done,       //
            failure,    //
        };

        enum class e_array_events
        {
            arr_begin,  // [
            arr_end,    // ]
            val_done,   //
            val_error,  //
            comma,      // ,
            symbol,     // any symbol(depends on state)
            skip,       // space, tab, cr, lf
            nothing,    // no action event
        };

        class array_parser_t
            : public parser_impl<arr, e_array_events, e_array_states, e_array_states::initial>
        {
            using event_t = e_array_events;
            using state_t = e_array_states;
            using EventToStateTable_t = StateTable<state_t, event_t>;

        public:
            array_parser_t()
                : m_event_2_state_table
            {
                { state_t::initial,    { { event_t::arr_begin, { state_t::val_before,  STD_BIND_TO_THIS( array_parser_t, on_begin   ) } },
                                         { event_t::skip,      { state_t::val_before,  STD_BIND_TO_THIS( array_parser_t, on_more    ) } },
                                         { event_t::symbol,    { state_t::failure,     STD_BIND_TO_THIS( array_parser_t, on_fail    ) } },
                } },
                { state_t::val_before, { { event_t::symbol,    { state_t::val_inside,  STD_BIND_TO_THIS( array_parser_t, on_val     ) } },
                                         { event_t::skip,      { state_t::val_before,  STD_BIND_TO_THIS( array_parser_t, on_more    ) } },
                                         { event_t::arr_end,   { state_t::done,        STD_BIND_TO_THIS( array_parser_t, on_done    ) } },
                } },
                { state_t::val_after,  { { event_t::arr_end,   { state_t::done,        STD_BIND_TO_THIS( array_parser_t, on_done    ) } },
                                         { event_t::comma,     { state_t::val_before,  STD_BIND_TO_THIS( array_parser_t, on_new     ) } },
                                         { event_t::skip,      { state_t::val_before,  STD_BIND_TO_THIS( array_parser_t, on_more    ) } },
                                         { event_t::symbol,    { state_t::failure,     STD_BIND_TO_THIS( array_parser_t, on_fail    ) } },
                } },
                { state_t::val_inside, { { event_t::symbol,    { state_t::val_inside,  STD_BIND_TO_THIS( array_parser_t, on_val     ) } },
                                         { event_t::val_done,  { state_t::val_after,   STD_BIND_TO_THIS( array_parser_t, on_got_val ) } },
                                         { event_t::val_error, { state_t::failure,     STD_BIND_TO_THIS( array_parser_t, on_fail    ) } },
                } },
                { state_t::done,       { { event_t::symbol,    { state_t::failure,     STD_BIND_TO_THIS( array_parser_t, on_fail    ) } },
                } },
                { state_t::failure,    { { event_t::symbol,    { state_t::failure,     STD_BIND_TO_THIS( array_parser_t, on_fail    ) } },
                } },
            } {};

        protected:
            // Inherited via parser
            virtual void reset() final;

            virtual result_t putchar(const symbol_t& c, const int pos) final;

            virtual value get() const final;

            // Inherited via parser_impl
            virtual const EventToStateTable_t& table() override;

            virtual event_t to_event(const symbol_t& c) const override;

            virtual event_t to_event(const result_t& r) const override;

            // Own methods
            result_t on_more(const unsigned& c, const int pos);

            result_t on_begin(const symbol_t& c, const int pos);

            result_t on_new(const symbol_t& c, const int pos);

            result_t on_val(const symbol_t& c, const int pos);

            result_t on_got_val(const symbol_t& c, const int pos);

            result_t on_done(const symbol_t& c, const int pos);

            result_t on_fail(const symbol_t& c, const int pos);

        protected:
            const EventToStateTable_t m_event_2_state_table;

            parser::ptr m_val_parser;
        };
    #pragma endregion
    //
    #pragma region -- object parser declaration -- 
        enum class e_object_states
        {
            initial,        //
            key_before,     //
            key_inside,     //
            key_after,      //
            val_before,     //
            val_inside,     //
            val_after,      //
            done,           //
            failure,        //
        };

        enum class e_object_events
        {
            // ascii part
            obj_begin,      // {
            obj_end,        // }
            key_done,       //
            key_error,      //
            val_done,       //
            val_error,      //
            colon,          // :
            comma,          // ,
            symbol,         // any symbol
            skip,           // space, tab, cr, lf
            nothing,        // no action event
        };

        class object_parser_t
            : public parser_impl<obj, e_object_events, e_object_states, e_object_states::initial>
        {
            using event_t = e_object_events;
            using state_t = e_object_states;
            using EventToStateTable_t = StateTable<state_t, event_t>;

        public:
            // {ctor}
            object_parser_t()
                : m_event_2_state_table
            {
                { state_t::initial,    { { event_t::obj_begin, { state_t::key_before, STD_BIND_TO_THIS( object_parser_t, on_begin   ) } },
                                         { event_t::skip,      { state_t::val_before, STD_BIND_TO_THIS( object_parser_t, on_more    ) } },
                                         { event_t::symbol,    { state_t::failure,    STD_BIND_TO_THIS( object_parser_t, on_fail    ) } },
                } },
                { state_t::key_before, { { event_t::obj_end,   { state_t::done,       STD_BIND_TO_THIS( object_parser_t, on_done    ) } },
                                         { event_t::key_error, { state_t::failure,    STD_BIND_TO_THIS( object_parser_t, on_fail    ) } },
                                         { event_t::symbol,    { state_t::key_inside, STD_BIND_TO_THIS( object_parser_t, on_key     ) } },
                                         { event_t::skip,      { state_t::key_before, STD_BIND_TO_THIS( object_parser_t, on_more    ) } },
                } },
                { state_t::key_inside, { { event_t::key_done,  { state_t::key_after,  STD_BIND_TO_THIS( object_parser_t, on_more    ) } },
                                         { event_t::key_error, { state_t::failure,    STD_BIND_TO_THIS( object_parser_t, on_fail    ) } },
                                         { event_t::symbol,    { state_t::key_inside, STD_BIND_TO_THIS( object_parser_t, on_key     ) } },
                } },
                { state_t::key_after,  { { event_t::colon,     { state_t::val_before, STD_BIND_TO_THIS( object_parser_t, on_more    ) } },
                                         { event_t::skip,      { state_t::key_after,  STD_BIND_TO_THIS( object_parser_t, on_more    ) } },
                } },
                { state_t::val_before, { { event_t::symbol,    { state_t::val_inside, STD_BIND_TO_THIS( object_parser_t, on_val     ) } },
                                         { event_t::val_error, { state_t::failure,    STD_BIND_TO_THIS( object_parser_t, on_fail    ) } },
                                         { event_t::skip,      { state_t::val_before, STD_BIND_TO_THIS( object_parser_t, on_more    ) } },
                } },
                { state_t::val_inside, { { event_t::val_done,  { state_t::val_after,  STD_BIND_TO_THIS( object_parser_t, on_got_val ) } },
                                         { event_t::val_error, { state_t::failure,    STD_BIND_TO_THIS( object_parser_t, on_fail    ) } },
                                         { event_t::symbol,    { state_t::val_inside, STD_BIND_TO_THIS( object_parser_t, on_val     ) } },
                } },
                { state_t::val_after,  { { event_t::comma,     { state_t::key_before, STD_BIND_TO_THIS( object_parser_t, on_new     ) } },
                                         { event_t::obj_end,   { state_t::done,       STD_BIND_TO_THIS( object_parser_t, on_done    ) } },
                                         { event_t::skip,      { state_t::val_after,  STD_BIND_TO_THIS( object_parser_t, on_more    ) } },
                } },
                { state_t::done,       { { event_t::symbol,    { state_t::failure,    STD_BIND_TO_THIS( object_parser_t, on_fail    ) } },
                } },
                { state_t::failure,    { { event_t::symbol,    { state_t::failure,    STD_BIND_TO_THIS( object_parser_t, on_fail    ) } },
                } },
            } {};

        protected:
            // inherited via parser
            virtual void reset() final;

            virtual result_t putchar(const symbol_t& c, const int pos) final;

            virtual value get() const final;

            // inherited via parser_impl
            virtual const EventToStateTable_t& table() override;

            virtual event_t to_event(const symbol_t& c)   const override;

            virtual event_t to_event(const result_t& r) const override;

            // own methods
            result_t on_more(const symbol_t& c, const int pos);

            result_t on_begin(const symbol_t& c, const int pos);

            result_t on_new(const symbol_t& c, const int pos);

            result_t on_key(const symbol_t& c, const int pos);

            result_t on_val(const symbol_t& c, const int pos);

            result_t on_done(const symbol_t& c, const int pos);

            result_t on_fail(const symbol_t& c, const int pos);

            result_t on_got_val(const symbol_t& c, const int pos);
        protected:
            const EventToStateTable_t m_event_2_state_table;

            parser::ptr m_key_parser;
            parser::ptr m_val_parser;
        };
    #pragma endregion 
    //////////////////////////////////////////////////////////////////////////
    #pragma region -- factory --
        /// creates object parser
        inline static typename parser::ptr create() 
        { 
            return parser::ptr(new object_parser_t()); 
        }
    #pragma endregion
    //////////////////////////////////////////////////////////////////////////
    };
    //////////////////////////////////////////////////////////////////////////
    using json = json_t<>;
    //////////////////////////////////////////////////////////////////////////
    #pragma region -- json data definition --
    JSON_TEMPLATE_PARAMS
    JSON_TEMPLATE_CLASS::obj::obj(std::initializer_list<JSON_TEMPLATE_CLASS::pair_t<typename JSON_TEMPLATE_CLASS::string, typename JSON_TEMPLATE_CLASS::value>> l)
    {
        for (auto arg : l)
            insert(arg);
    }

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::string
    JSON_TEMPLATE_CLASS::obj::str(typename JSON_TEMPLATE_CLASS::sstream& str) const
    {
        // leading curly brace
        str << "{";

        for (auto it = begin(); it != end(); ++it)
        {
            const boolean_t last = (--end() == it);

            // key
            str << "\"" << it->first << "\":";

            // value
            switch ((value::vt)it->second.index())
            {
            case value::vt::t_string:
                str << "\"" << container::serialize_string(it->second.get<string>()) << "\"";
                break;
            case value::vt::t_object:
                it->second.get<obj>().str(str);
                break;
            case value::vt::t_array:
                it->second.get<arr>().str(str);
                break;
            case value::vt::t_integer:
                str << it->second.get<integer_t>();
                break;
            case value::vt::t_floatingpt:
                str << std::scientific << it->second.get<floatingpt_t>();
                break;
            case value::vt::t_boolean:
                str << (it->second.get<boolean_t>() ? "true" : "false");
                break;
            case value::vt::t_null:
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

    JSON_TEMPLATE_PARAMS
    JSON_TEMPLATE_CLASS::arr::arr(std::initializer_list<value> l)
    {
        for (auto arg : l)
            push_back(arg);
    }

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::string
    JSON_TEMPLATE_CLASS::arr::str(typename JSON_TEMPLATE_CLASS::sstream& str) const
    {
        // leading curly brace
        str << "[";

        for (auto it = begin(); it != end(); ++it)
        {
            const boolean_t last = (--end() == it);

            // value
            switch ((value::vt)it->index())
            {
            case value::vt::t_string:
                str << "\"" << container::serialize_string((*it).get<string>())<< "\"";
                break;
            case value::vt::t_object:
                (*it).get<obj>().str(str);
                break;
            case value::vt::t_array:
                (*it).get<arr>().str(str);
                break;
            case value::vt::t_integer:
                str << (*it).get<integer_t>();
                break;
            case value::vt::t_floatingpt:
                str << std::scientific << (*it).get<floatingpt_t>();
                break;
            case value::vt::t_boolean:
                str << (((*it).get<boolean_t>() ? "true" : "false"));
                break;
            case value::vt::t_null:
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
    JSON_TEMPLATE_PARAMS
    void
    JSON_TEMPLATE_CLASS::string_parser_t::reset()
    {
        state::set(state_t::initial);
        m_value.reset();
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::string_parser_t::putchar(const symbol_t& c, const int pos)
    {
        return parser_impl::step(to_event(c), c, pos);
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value
    JSON_TEMPLATE_CLASS::string_parser_t::get() const
    {
        if (m_value)
            return *m_value;

        assert(0); // TODO: throw an exception
        return value();
    };

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::string_parser_t::EventToStateTable_t&
    JSON_TEMPLATE_CLASS::string_parser_t::table()
    {
        return m_event_2_state_table;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::string_parser_t::event_t
    JSON_TEMPLATE_CLASS::string_parser_t::to_event(const symbol_t& c) const
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
            case 0x22:  smb = event_t::quote;       break;
            case 0x5C:  smb = event_t::back_slash;  break;
            case 0x2F:  smb = event_t::slash;       break;
            case 0x62:  smb = event_t::alpha_b;     break;
            case 0x66:  smb = event_t::alpha_f;     break;
            case 0x6E:  smb = event_t::alpha_n;     break;
            case 0x72:  smb = event_t::alpha_r;     break;
            case 0x74:  smb = event_t::alpha_t;     break;
            case 0x75:  smb = event_t::alpha_u;     break;
            }
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::string_parser_t::event_t
    JSON_TEMPLATE_CLASS::string_parser_t::to_event(const result_t& c) const
    {
        return event_t::symbol;
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::string_parser_t::on_initial(const symbol_t&c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::string_parser_t::on_inside(const symbol_t&c, const int pos)
    {
        if (!m_value)
            m_value.emplace();

        (*m_value) += c;

        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::string_parser_t::on_escape(const symbol_t&c, const int pos)
    {
        result_t result = result_t::s_need_more;
        const state_t s = state::get();

        if (state_t::inside == s)
        {
            m_cache.clear();
            m_cache.push_back(c);
        }
        else if (state_t::escape == state::get() && m_cache.size() == 1 && m_cache[0] == '\\')
        {
            assert(m_value);
    
            m_cache.push_back(c);
#pragma warning( push )
#pragma warning( disable : 4129 )
            if (m_cache == "\\b")        // got backspace sequence
                (*m_value) += '\b', m_cache.clear();
            else if (m_cache == "\\f")   // got formfeed sequence
                (*m_value) += '\f', m_cache.clear();
            else if (m_cache == "\\n")   // got newline sequence
                (*m_value) += '\n', m_cache.clear();
            else if (m_cache == "\\r")   // got carriage return sequence
                (*m_value) += '\r', m_cache.clear();
            else if (m_cache == "\\t")   // got hotisontal tab sequence
                (*m_value) += '\t', m_cache.clear();
            else if (m_cache == "\\\"")  // got double quote as symbol sequence
                (*m_value) += m_cache, m_cache.clear();
            else if (m_cache == "\\\\")  // got backslash as symbol sequence
                (*m_value) += m_cache, m_cache.clear();
            else if (m_cache == "\\/")   // got slash as symbol sequence
                (*m_value) += "\/", m_cache.clear();
            else if (m_cache != "\\u")   // got any other symbol except \u
                result = result_t::e_unexpected;
        }
#pragma warning( pop )
        return result;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::string_parser_t::on_unicode(const symbol_t&c, const int pos)
    {
        result_t result = result_t::s_need_more;

        assert(m_value);

        switch (state::get())
        {
        case state_t::unicode_1:
        case state_t::unicode_2:
        case state_t::unicode_3:
            m_cache.push_back(c);
            break;
        case state_t::unicode_4:
            m_cache.push_back(c);
            assert(0); // once you got here - implement correct inserting of the 4byte symbol into the string
            (*m_value) += m_cache;
            m_cache.clear();
            break;
        default:
            result = result_t::e_unexpected;
        }
        (*m_value) += c;
        return result;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::string_parser_t::on_done(const symbol_t&c, const int pos)
    {
        if (!m_value)
            m_value.emplace();

        return result_t::s_done;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::string_parser_t::on_fail(const symbol_t&c, const int pos)
    {
        return result_t::e_unexpected;
    }
    #pragma endregion
    //
    #pragma region -- number parser definition --
    JSON_TEMPLATE_PARAMS
    void
    JSON_TEMPLATE_CLASS::number_parser_t::reset()
    {
        state::set(state_t::initial);

        m_value.reset();
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::putchar(const symbol_t& c, const int pos)
    {
        return parser_impl::step(to_event(c), c, pos);
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value
    JSON_TEMPLATE_CLASS::number_parser_t::get() const
    {
        if (m_value)
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

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::number_parser_t::EventToStateTable_t&
    JSON_TEMPLATE_CLASS::number_parser_t::table()
    {
        return m_event_2_state_table;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::number_parser_t::event_t
    JSON_TEMPLATE_CLASS::number_parser_t::to_event(const symbol_t& c) const
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::number_parser_t::event_t
    JSON_TEMPLATE_CLASS::number_parser_t::to_event(const result_t& c) const
    {
        return event_t::symbol;
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_initial(const symbol_t& c, const int pos)
    {
        // TODO: use symbol
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_minus(const symbol_t& c, const int pos)
    {
        if (!m_value)
            m_value.emplace();

        (*m_value).m_positive = false;

        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_integer(const symbol_t& c, const int pos)
    {
        if (!m_value)
            m_value.emplace();

        const result_t res = append_digit((*m_value).m_integer, c);
        return result_t::s_ok == res ? result_t::s_need_more : res;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_fractional(const symbol_t& c, const int pos)
    {
        assert(m_value);
        const result_t res = append_digit((*m_value).m_fractional_value, c);
        (*m_value).m_fractional_digits++;
        return result_t::s_ok == res ? result_t::s_need_more : res;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_exponent(const symbol_t& c, const int pos)
    {
        assert(m_value);
        (*m_value).m_has_exponent = true;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_exp_sign(const symbol_t& c, const int pos)
    {
        assert(m_value);

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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_exp_value(const symbol_t& c, const int pos)
    {
        assert(m_value);
        const result_t res = append_digit((*m_value).m_exponent_value, c);
        return result_t::s_ok == res ? result_t::s_need_more : res;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_zero(const symbol_t& c, const int pos)
    {

        const state_t s = state::get();
        result_t res = result_t::s_ok;

        switch (s)
        {
        case state_t::initial:
        case state_t::leading_minus:
        case state_t::integer:
            if (!m_value)
                m_value.emplace();
            res = append_digit((*m_value).m_integer, c);
            break;
        case state_t::decimal_dot:
        case state_t::fractional:
            assert(m_value);
            res = append_digit((*m_value).m_fractional_value, c);
            break;
        case state_t::exponent_delim:
        case state_t::exponent_sign:
            assert(m_value);
        case state_t::exponent_val:
            res = append_digit((*m_value).m_exponent_value, c);
            break;
        }

        return result_t::s_ok == res ? result_t::s_need_more : res;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_dot(const symbol_t& c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_done(const symbol_t& c, const int pos)
    {
        return result_t::s_done_rpt;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::on_fail(const symbol_t& c, const int pos)
    {
        return result_t::e_unexpected;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::number_parser_t::append_digit(integer_t& val, const symbol_t& c)
    {
        if (c < 0x30 || 0x39 < c)
            return result_t::e_fatal;

        val *= 10;

        switch (c)
        {
        case 0x31: val += 1; break; //1
        case 0x32: val += 2; break; //2
        case 0x33: val += 3; break; //3
        case 0x34: val += 4; break; //4
        case 0x35: val += 5; break; //5
        case 0x36: val += 6; break; //6
        case 0x37: val += 7; break; //7
        case 0x38: val += 8; break; //8
        case 0x39: val += 9; break; //9
        }

        return result_t::s_ok;
    };
    #pragma endregion
    //
    #pragma region -- null parser definition -- 
    JSON_TEMPLATE_PARAMS
    void
    JSON_TEMPLATE_CLASS::null_parser_t::reset()
    {
        state::set(state_t::initial);
        m_value.reset();
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::null_parser_t::putchar(const symbol_t& c, const int pos)
    {
        return parser_impl::step(to_event(c), c, pos);
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value
    JSON_TEMPLATE_CLASS::null_parser_t::get() const
    {
        if (m_value)
            return *m_value;

        assert(0); // TODO: throw an exception
        return value();
    };

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::null_parser_t::EventToStateTable_t&
    JSON_TEMPLATE_CLASS::null_parser_t::table()
    {
        return m_event_2_state_table;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::null_parser_t::event_t
    JSON_TEMPLATE_CLASS::null_parser_t::to_event(const symbol_t& c) const
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::null_parser_t::event_t
    JSON_TEMPLATE_CLASS::null_parser_t::to_event(const result_t& c) const
    {
        return event_t::other;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::null_parser_t::on_n(const symbol_t& c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::null_parser_t::on_u(const symbol_t& c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::null_parser_t::on_l(const symbol_t& c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::null_parser_t::on_done(const symbol_t& c, const int pos)
    {
        if (!m_value)
            m_value.emplace();

        (*m_value) = nullptr;

        return result_t::s_done;
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::null_parser_t::on_fail(const symbol_t& c, const int pos)
    {
        return result_t::e_unexpected;
    }
    #pragma endregion
    //
    #pragma region -- bool parser definition -- 
    JSON_TEMPLATE_PARAMS
    void
    JSON_TEMPLATE_CLASS::bool_parser_t::reset()
    {
        state::set(state_t::initial);
        m_str.clear();
        m_value.reset();
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::putchar(const symbol_t& c, const int pos)
    {
        return parser_impl::step(to_event(c), c, pos);
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value
    JSON_TEMPLATE_CLASS::bool_parser_t::get() const
    {
        if (m_value)
            return *m_value;

        assert(0); // TODO: throw an exception
        return value();
    };

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::bool_parser_t::EventToStateTable_t&
    JSON_TEMPLATE_CLASS::bool_parser_t::table()
    {
        return m_event_2_state_table;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::bool_parser_t::event_t
    JSON_TEMPLATE_CLASS::bool_parser_t::to_event(const symbol_t& c) const
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::bool_parser_t::event_t
    JSON_TEMPLATE_CLASS::bool_parser_t::to_event(const result_t& c) const
    {
        return event_t::symbol;
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_t(const symbol_t& c, const int pos)
    {
        m_str += c;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_r(const symbol_t& c, const int pos)
    {
        m_str += c;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_u(const symbol_t& c, const int pos)
    {
        m_str += c;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_f(const symbol_t& c, const int pos)
    {
        m_str += c;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_a(const symbol_t& c, const int pos)
    {
        m_str += c;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_l(const symbol_t& c, const int pos)
    {
        m_str += c;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_s(const symbol_t& c, const int pos)
    {
        m_str += c;
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_done(const symbol_t& c, const int pos)
    {
        m_str += c;

        auto update = [this](const boolean_t val)->result_t 
        {
            if (!m_value)
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::bool_parser_t::on_fail(const symbol_t& c, const int pos)
    {
        return result_t::e_unexpected;
    }
    #pragma endregion
    //
    #pragma region -- value parser definition --
    JSON_TEMPLATE_PARAMS
    void
    JSON_TEMPLATE_CLASS::value_parser_t::reset()
    {
        state::set(state_t::initial);
        for (ParserItem_t& p : parsing_unit)
            p.first = true, p.second->reset();
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::value_parser_t::putchar(const symbol_t& c, const int pos)
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value
    JSON_TEMPLATE_CLASS::value_parser_t::get() const
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

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::value_parser_t::EventToStateTable_t&
    JSON_TEMPLATE_CLASS::value_parser_t::table()
    {
        return m_event_2_state_table;
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value_parser_t::event_t
    JSON_TEMPLATE_CLASS::value_parser_t::to_event(const symbol_t& c) const
    {
        return event_t::symbol;
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value_parser_t::event_t
    JSON_TEMPLATE_CLASS::value_parser_t::to_event(const result_t& c) const
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::value_parser_t::on_data(const symbol_t& c, const int pos)
    {
        result_t res = result_t::e_fatal;
        uint8_t parsers_in_work = 0;

        if (parsing_unit.empty())
        {
            parsing_unit.push_back(ParserItem_t(true, new null_parser_t()));
            parsing_unit.push_back(ParserItem_t(true, new bool_parser_t()));
            parsing_unit.push_back(ParserItem_t(true, new string_parser_t()));
            parsing_unit.push_back(ParserItem_t(true, new number_parser_t()));
            parsing_unit.push_back(ParserItem_t(true, new array_parser_t()));
            parsing_unit.push_back(ParserItem_t(true, new object_parser_t()));
        }

        for (std::pair<boolean_t, parser::ptr>& p : parsing_unit)
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::value_parser_t::on_done(const symbol_t& c, const int pos)
    {
        return result_t::s_done;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::value_parser_t::on_fail(const symbol_t& c, const int pos)
    {
        return result_t::e_unexpected;
    }
    #pragma endregion
    //
    #pragma region -- array parser definition -- 
    JSON_TEMPLATE_PARAMS
    void
    JSON_TEMPLATE_CLASS::array_parser_t::reset()
    {
        state::set(state_t::initial);

        if (!m_val_parser)
            m_val_parser.reset(new value_parser_t());

        m_val_parser->reset();

        m_value.reset();
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::putchar(const symbol_t& c, const int pos)
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value
    JSON_TEMPLATE_CLASS::array_parser_t::get() const
    {
        if (m_value)
            return *m_value;

        assert(0); // TODO: throw an exception
        return value();
    }

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::array_parser_t::EventToStateTable_t&
    JSON_TEMPLATE_CLASS::array_parser_t::table()
    {
        return m_event_2_state_table;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::array_parser_t::event_t
    JSON_TEMPLATE_CLASS::array_parser_t::to_event(const symbol_t& c) const
    {
        auto is_space = [](const symbol_t& c)->boolean_t
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::array_parser_t::event_t
    JSON_TEMPLATE_CLASS::array_parser_t::to_event(const result_t& r) const
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::on_more(const unsigned& c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::on_begin(const symbol_t& c, const int pos)
    {
        if (!m_val_parser)
            m_val_parser.reset(new value_parser_t());

        if (!m_value)
            m_value.emplace();

        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::on_new(const symbol_t& c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::on_val(const symbol_t& c, const int pos)
    {
        return m_val_parser->putchar(c, pos);
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::on_got_val(const symbol_t& c, const int pos)
    {
        assert(m_value);

        const value val = m_val_parser->get();

        (*m_value).push_back(val);

        m_val_parser->reset();

        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::on_done(const symbol_t& c, const int pos)
    {
        assert(m_value);

        return result_t::s_done;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::array_parser_t::on_fail(const symbol_t& c, const int pos)
    {
        return result_t::e_unexpected;
    }
    #pragma endregion
    //
    #pragma region -- object parser definition --
    JSON_TEMPLATE_PARAMS
    void
    JSON_TEMPLATE_CLASS::object_parser_t::reset()
    {
        state::set(state_t::initial);

        if (!m_key_parser)
            m_key_parser.reset(new string_parser_t());
        else
            m_key_parser->reset();

        if (!m_val_parser)
            m_val_parser.reset(new value_parser_t());
        else
            m_val_parser->reset();

        m_value.reset();
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::putchar(const symbol_t& c, const int pos)
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::value
    JSON_TEMPLATE_CLASS::object_parser_t::get() const
    {
        if (m_value)
            return *m_value;

        assert(0); // TODO: throw an exception
        return value();
    };

    JSON_TEMPLATE_PARAMS
    const typename JSON_TEMPLATE_CLASS::object_parser_t::EventToStateTable_t&
    JSON_TEMPLATE_CLASS::object_parser_t::table()
    {
        return m_event_2_state_table;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::object_parser_t::event_t
    JSON_TEMPLATE_CLASS::object_parser_t::to_event(const symbol_t& c) const
    {
        auto is_space = [](const symbol_t& c)->boolean_t
        {
            // space, tab, cr, lf
            return (0x20 == c || 0x09 == c || 0x0A == c || 0x0D == c);
        };

        switch (state::get())
        {
        case state_t::initial:
            if (0x7B == c)  //{
                return event_t::obj_begin;
            if (is_space(c))
                return event_t::skip;
            break;
        case state_t::key_before:
            if (0x7D == c)  //}
                return event_t::obj_end;
            if (is_space(c))
                return event_t::skip;
            break;
        case state_t::key_after:
            if (is_space(c))
                return event_t::skip;
            if (0x3A == c)  //:
                return event_t::colon;
            break;
        case state_t::val_before:
            if (is_space(c))
                return event_t::skip;
            break;
        case state_t::val_after:
            if (0x7D == c)  //}
                return event_t::obj_end;
            if (is_space(c))
                return event_t::skip;
            if (0x2c == c)  //,
                return event_t::comma;
            break;
        }

        return event_t::symbol;
    };

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::object_parser_t::event_t
    JSON_TEMPLATE_CLASS::object_parser_t::to_event(const result_t& r) const
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

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_more(const symbol_t& c, const int pos)
    {
        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_begin(const symbol_t& c, const int pos)
    {
        if (!m_key_parser)
            m_key_parser.reset(new string_parser_t());

        if (!m_val_parser)
            m_val_parser.reset(new value_parser_t());

        if (!m_value)
            m_value.emplace();

        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_new(const symbol_t& c, const int pos)
    {
        if (!m_key_parser)
            m_key_parser.reset(new string_parser_t());
        else
            m_key_parser->reset();

        if (!m_val_parser)
            m_val_parser.reset(new value_parser_t());
        else
            m_val_parser->reset();

        return result_t::s_need_more;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_key(const symbol_t& c, const int pos)
    {
        return m_key_parser->putchar(c, pos);
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_val(const symbol_t& c, const int pos)
    {
        return m_val_parser->putchar(c, pos);
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_done(const symbol_t& c, const int pos)
    {
        return result_t::s_done;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_fail(const symbol_t& c, const int pos)
    {
        m_value.reset();
        return result_t::e_unexpected;
    }

    JSON_TEMPLATE_PARAMS
    typename JSON_TEMPLATE_CLASS::result_t
    JSON_TEMPLATE_CLASS::object_parser_t::on_got_val(const symbol_t& c, const int pos)
    {
        assert(m_value);

        const string key = m_key_parser->get().get<string>();
        const value val = m_val_parser->get();

        (*m_value)[key] = val;

        return result_t::s_need_more;
    }
#pragma endregion
}
#undef JSON_TEMPLATE_PARAMS
#undef JSON_TEMPLATE_CLASS
#undef STD_BIND_TO_THIS

#endif // __JSON_LIB_H__