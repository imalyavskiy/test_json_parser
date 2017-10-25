#ifndef __JSON_LIB_H__
#define __JSON_LIB_H__
namespace json
{
	enum class result
	{
		s_need_more = 3,	// need more data
		s_done_rpt = 2,		// symbol succesfully terminates parsing but makes no sense to current parser, put it once again to above parser
		s_done = 1,			// symbol succesfully terminates parsing and makes sense to current parser
		s_ok = 0,			// general success
		e_fatal = -1,		// general failure
		e_unexpected = -2,  // unexpected parameter value
	};

	struct parser
	{
		typedef std::shared_ptr<parser> ptr;

		virtual ~parser() {};
		
		virtual void	reset() = 0;
		virtual result	step(const char& c, const int pos) = 0;
	};

	parser::ptr create_object_parser();
	parser::ptr create_array_parser();
	parser::ptr create_number_parser();
	parser::ptr create_string_parser();
	parser::ptr create_null_parser();
	parser::ptr create_bool_parser();
}
#endif // __JSON_LIB_H__