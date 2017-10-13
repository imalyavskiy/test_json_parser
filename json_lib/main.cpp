// main.cpp : Defines the entry point for the console application.
//

#include "json_tokenizer.h"

std::string src
(	"{"
		"\"glossary\": "
			"{"
				"\"title\": \"example glossary\","
				"\"GlossDiv\" : "
					"{"
						"\"title\": \"S\","
						"\"GlossList\" : "
							"{"
								"\"GlossEntry\": "
									"{"
										"\"ID\": \"SGML\","
										"\"SortAs\" : \"SGML\","
										"\"GlossTerm\" : \"Standard Generalized Markup Language\","
										"\"Acronym\" : \"SGML\","
										"\"Abbrev\" : \"ISO 8879:1986\","
										"\"GlossDef\" : "
											"{"
												"\"para\": \"A meta-markup language, used to create markup languages such as DocBook.\","
												"\"GlossSeeAlso\" : [\"GML\", \"XML\"]"
											"},"
										"\"GlossSee\" : \"markup\""
									"}"
							"}"
					"}"
			"}"
	"}");
std::string src2
(	"{"
	"\"one\""	":" "\"1\","
	"\"two\""	":" "\"2\","
	"\"three\"" ":" "\"3\","
	"\"four\""	":" "\"4\","
	"\"five\""	":" "\"5\","
	"\"six\""	":" "\"6\","
	"\"seven\"" ":" "\"7\","
	"\"eight\"" ":" "\"8\","
	"\"nine\""	":" "\"9\","
	"\"zero\""	":" "\"0\""
	"}");

int main()
{
	json::tokenizer tokenizer;
	return (int)tokenizer.process(src2);
}

