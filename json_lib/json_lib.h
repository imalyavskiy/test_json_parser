#ifndef __JSON_LIB_H__
#define __JSON_LIB_H__

#define json_failed(_x_) (int)_x_ < 0
#define json_succeded(_x_) (int)_x_ >= 0

#include <map>
#include <variant>
#include <vector>
#include <string>
#include <sstream>

namespace json
{
	/// Forward declaration for JSON object data structure
	struct object_t;

	/// Forward declaration for JSON array data structure
	struct array_t;

	/// possible results
	enum class result_t
	{
		s_need_more = 3,	// need more data
		s_done_rpt = 2,		// symbol succesfully terminates parsing but makes no sense to current parser, put it once again to above parser
		s_done = 1,			// symbol succesfully terminates parsing and makes sense to current parser
		s_ok = 0,			// general success
		e_fatal = -1,		// general failure
		e_unexpected = -2,  // unexpected parameter value
	};

	template<typename StringT = std::string, typename ObjectT = object_t, typename ArrayT = array_t, typename IntNumT = int64_t, typename RealNumT = double, typename BooleanT = bool, typename NullT = nullptr_t>
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
		value_t(const char* other) : base_t(std::string(other)) {}
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
			base_t::operator=(std::string(other));
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

	/// creates object parser
	parser::ptr create_object_parser();

	/// creates array parser
	parser::ptr create_array_parser();

	/// creates number parser
	parser::ptr create_number_parser();

	/// creates string parser
	parser::ptr create_string_parser();

	/// creates null parser
	parser::ptr create_null_parser();

	/// creates bool parser
	parser::ptr create_bool_parser();

	/// creates value parser
	parser::ptr create_value_parser();

	template<class BaseType>
	struct container 
		: public BaseType
	{
		using base_t = BaseType;

		/// The enumaration for mnemonic correlation of indeces and types used for std::variant
		enum class vt
		{	/// IMPORTANT: The order of parameters is the same as value_t template parameters order
			t_string		= 0,
			t_object		= 1,
			t_array			= 2,
			t_int64			= 3,
			t_floatingpt	= 4,
			t_boolean		= 5,
			t_null			= 6,
		};

		/// {ctor}s
		container() = default;
		container(std::initializer_list<value> l) : BaseType(l) {}

		virtual const std::string str(std::stringstream& str = std::stringstream()) = 0;
	};

	/// Declaration of the object JSON data structure
	struct object_t : public container<std::map<std::string, value>>
	{
		/// {ctor}s
		object_t() = default;
		object_t(std::initializer_list<std::pair<std::string, value>> l);

		/// serialization
		virtual const std::string str(std::stringstream& str = std::stringstream()) final;
	};

	/// Declaration of the array JSON data structure
	struct array_t : public container<std::vector<value>>
	{
		/// {ctor}s
		array_t() = default;
		array_t(std::initializer_list<value> l);

		/// serialization
		virtual const std::string str(std::stringstream& str = std::stringstream()) final;
	};
}
#endif // __JSON_LIB_H__