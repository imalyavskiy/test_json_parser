// main.cpp : Defines the entry point for the console application.
//
#include "../json_lib/json_lib.h"
#include <gtest/gtest.h>

typedef imalyavskiy::json::result_t result_t;
using json = imalyavskiy::json;

int read_file_data(const std::string& file_name, std::string& file_data)
{
    if (std::ifstream is{ file_name, std::ios::binary | std::ios::ate }) 
    {
        const auto size = is.tellg();

        file_data = std::string(size, '\0'); // construct string to stream size

        is.seekg(0);

        if (!is.read(&file_data[0], size))
            return -1;

        return 0;
    }

    return -1;
}

/*
{
    "glossary": {
        "title": "example glossary",
        "GlossDiv" : {
            "title": "S",
            "GlossList" : {
                "GlossEntry": {
                    "ID": "SGML",
                    "SortAs" : "SGML",
                    "GlossTerm" : "Standard Generalized Markup Language",
                    "Acronym" : "SGML",
                    "Abbrev" : "ISO 8879:1986",
                    "GlossDef" : {
                        "para": "A meta-markup language, used to create markup languages such as DocBook.",
                        "GlossSeeAlso" : [
                            "GML", 
                            "XML"
                        ]
                    },
                    "GlossSee" : "markup"
                }
            }
        }
    }
}
*/
const std::string& json_data_structure_2(std::string& str = std::string())
{
    using obj = json::obj;
    using arr = json::arr;

    auto glossary = obj
    {
        { "title", "example glossary"},
        { "GlossDiv", obj{
            { "title", "S"},
            { "GlossList", obj{
                { "GlossEntry", obj{
                    { "ID", "SGML"},
                    { "SortAs", "SGML"},
                    { "GlossTerm", "Standard Generalized Markup Language"},
                    { "Acronym", "SGML"},
                    { "Abbrev", "ISO 8879:1986"},
                    { "GlossDef", obj{
                        { "GlossDef", "A meta-markup language, used to create markup languages such as DocBook."},
                        { "GlossSeeAlso", arr{ 
                            "GML",
                            "XML",
                            true,
                            false,
                            nullptr,
                            (int64_t)-1,
                            (int64_t)100
                        }}
                    }}
                }}
            }}
        }}
    };

    return str;
}

const std::string& json_data_structure_1(std::string& str = std::string())
{
    json::arr GlossSeeAlso;
    GlossSeeAlso.push_back("GML");
    GlossSeeAlso.push_back("XML");
    GlossSeeAlso.push_back(true);
    GlossSeeAlso.push_back(false);
    GlossSeeAlso.push_back(nullptr);
    GlossSeeAlso.push_back((int64_t)-1);
    GlossSeeAlso.push_back((int64_t)100);

    json::obj GlossDef;
    GlossDef["GlossDef"]        = std::string("A meta-markup language, used to create markup languages such as DocBook.");
    GlossDef["GlossSeeAlso"]    = GlossSeeAlso;

    json::obj GlossEntry;
    GlossEntry["ID"]            = std::string("SGML");
    GlossEntry["SortAs"]        = std::string("SGML");
    GlossEntry["GlossTerm"]     = std::string("Standard Generalized Markup Language");
    GlossEntry["Acronym"]       = std::string("SGML");
    GlossEntry["Abbrev"]        = std::string("ISO 8879:1986");
    GlossEntry["GlossDef"]      = GlossDef;

    json::obj GlossList;
    GlossList["GlossEntry"]     = GlossEntry;

    json::obj GlossDiv;
    GlossDiv["title"]           = std::string("S");
    GlossDiv["GlossList"]       = GlossList;

    json::obj glossary;
    glossary["title"]           = std::string("example glossary");
    glossary["GlossDiv"]        = GlossDiv;

    return str;
}

TEST(CompleteObjectTest, test0000_EmptyObject)
{
    const std::string data(
        "{}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0001_EmptySubObject)
{
    const std::string data(
        "{\"1\":{}}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0002_TwoEmptySubObjects)
{
    const std::string data(
        "{\"1\":{},\"2\":{}}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0003_EmptyArray)
{
    const std::string data(
        "{\"1\":[]}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0004_EmptyArrayAndSubObject)
{
    const std::string data(
        "{\"1\":[],\"2\":{}}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0005_TwoStringValues)
{
    const std::string data(
        "{\"1\":\"1\",\"2\":\"2\"}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0006_TwoStringValuesFormatted)
{
    const std::string data(
        "{\r\n\t\"1\":\"1\",\r\n\t\"2\":\"2\"\r\n}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0007_TwoValuesNumberAndStringFormatted)
{
    const std::string data(
        "{\n\t\"1\": 1,\n\t\"2\" : \"two\"\n}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0008_MultipleVersatileValuesFormatted)
{
    const std::string data(
        "{\n\t\"1\": 1,\n\t\"2\": \"two\",\n\t\"3\": null,\n\t\"4""\": false,\n"
        "\t\"5\": true,\n\t\"6\": [],\n\t\"5\": {}\n}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0009_SimpleArray)
{
    const std::string data(
        "{\"array\":[1,null]}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0010_ComplexArray)
{
    const std::string data(
        "{\"array\":[-1.0,null,true,false,\"string\",[\"another string\"],{\"one\":1}]}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0011_ElementWithFloatingPointNumberValue)
{
    const std::string data(
        "{\"num\":-3.14159261e-05}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, SimpleObject)
{
    const std::string data(
        "{\"jsonrpc\":\"2.0\",\"id\":0,\"result\":{\"message\":\"logged in\",\""
        "sessid\":\"4013c48f-884d-4cba-aab5-20eeff10b0fc\"}}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0013_ARealObjectWithSDP)
{
    const std::string data = 
        "{\"jsonrpc\":\"2.0\",\"id\":69,\"method\":\"verto.media\",\"params\":"
        "{\"callID\":\"3c2b1fae-7665-40e6-b4e1-e61f1b738e8d\",\"sdp\":\"v=0\\r"
        "\\no=FreeSWITCH 1510258435 1510258436 IN IP4 54.202.245.29\\r\\ns=Fre"
        "eSWITCH\\r\\nc=IN IP4 54.202.245.29\\r\\nt=0 0\\r\\na=msid-semantic: "
        "WMS qtCaCWAeHZ3OzNlbMKnEIpHtudwz2myj\\r\\nm=audio 29220 UDP/TLS/RTP/S"
        "AVPF 111 110\\r\\na=rtpmap:111 opus/48000/2\\r\\na=fmtp:111 useinband"
        "fec=1; minptime=10\\r\\na=rtpmap:110 telephone-event/48000\\r\\na=sil"
        "enceSupp:off - - - -\\r\\na=ptime:20\\r\\na=sendrecv\\r\\na=fingerpri"
        "nt:sha-256 6C:50:94:1D:6F:18:67:1F:ED:41:2E:B3:93:97:9C:71:D9:FB:43:3"
        "8:91:76:67:DE:AE:D3:01:8B:D4:B7:19:A7\\r\\na=setup:active\\r\\na=rtcp"
        "-mux\\r\\na=rtcp:29220 IN IP4 54.202.245.29\\r\\na=ice-ufrag:LcQjroDM"
        "r0ckcAtI\\r\\na=ice-pwd:qDG3dJeDG4rg5hsGj9QBnjeF\\r\\na=candidate:657"
        "7026236 1 udp 659136 54.202.245.29 29220 typ host generation 0\\r\\na"
        "=end-of-candidates\\r\\na=ssrc:1525869198 cname:F0k8NKclz6ZYKbZm\\r\\"
        "na=ssrc:1525869198 msid:qtCaCWAeHZ3OzNlbMKnEIpHtudwz2myj a0\\r\\na=ss"
        "rc:1525869198 mslabel:qtCaCWAeHZ3OzNlbMKnEIpHtudwz2myj\\r\\na=ssrc:15"
        "25869198 label:qtCaCWAeHZ3OzNlbMKnEIpHtudwz2myja0\\r\\n\"}}";

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0014_SingleItemObjectWithEmptyStringValue)
{
    const std::string data(
        "{\"1\":\"\"}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0015_RealRestApiResponse)
{
    const std::string data(
        "{\"ok\":false,\"error\":{\"code\":\"access_denied\",\"desc\":\"\"}}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0016_RealRestApiResponseWithFilledArray)
{
    const std::string data(
        "{\"ok\":true,\"conference\":{\"userlist\":[\"ab3c2a18-a5cb-41d2-b4ee-"
        "66befc87728b\",\"c47d867b-08f0-4d37-a042-9989a299f7a8\"]}}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0100_ComplexObjectFotmatted)
{
    const std::string data(
        "{\n \"one\":\"1\",\n \"two\": 2,\n \"three\": { \"three\" : 3.0 },\n "
        "\"four\" : [\"4\", 4.0]\n}");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(CompleteObjectTest, test0200_BigComplexObjectFormatted)
{
    const std::string data(
        "{\n\t\"glossary\": \n\t\t{\n\t\t\t\"title\": \"example glossary\",\n"
        "\t\t\t\"GlossDiv\" : \n\t\t\t\t{\n\t\t\t\t\t\"title\": \"S\",\n\t\t\t"
        "\t\t\"GlossList\" : \n\t\t\t\t\t\t{\n\t\t\t\t\t\t\t\"GlossEntry\": \n"
        "\t\t\t\t\t\t\t\t{\n\t\t\t\t\t\t\t\t\t\"ID\": \"SGML\",\n\t\t\t\t\t\t"
        "\t\t\t\"SortAs\" : \"SGML\",\n\t\t\t\t\t\t\t\t\t\"GlossTerm\" : \"St"
        "andard Generalized Markup Language\",\n\t\t\t\t\t\t\t\t\t\"Acronym\""
        " : \"SGML\",\n\t\t\t\t\t\t\t\t\t\"Abbrev\" : \"ISO 8879:1986\",\n\t\t"
        "\t\t\t\t\t\t\t\"GlossDef\" : \n\t\t\t\t\t\t\t\t\t\t{\n\t\t\t\t\t\t\t"
        "\t\t\t\t\"para\": \"A meta-markup language, used to create markup la"
        "nguages such as DocBook.\",\n\t\t\t\t\t\t\t\t\t\t\t\"GlossSeeAlso\" "
        ": [\"GML\", \"XML\"]\n\t\t\t\t\t\t\t\t\t\t},\n\t\t\t\t\t\t\t\t\t\"Gl"
        "ossSee\" : \"markup\"\n\t\t\t\t\t\t\t\t}\n\t\t\t\t\t\t}\n\t\t\t\t}\n"
        "\t\t}\n}\n");

    json::obj jsobj{};

    ASSERT_EQ(json::result_t::s_done, json::parse(data, jsobj));
}

TEST(ObjectStructureCase, test0000_TwoWaysObjectCreationResultCompare)
{
    ASSERT_EQ(json_data_structure_1(), json_data_structure_2());
}

TEST(ObjectStructureCase, test0001_ElementAccess)
{
    json::obj test_obj{
        {"first", json::obj{
            {"second", json::obj{
                {"third", json::arr{
                    "fourth", "fifth"
                } }
            } }
        } }
    };

    json::obj _1 = test_obj["first"];
    json::obj _2 = test_obj["first"]["second"];
    json::arr _3 = test_obj["first"]["second"]["third"];
    json::string _4 = (json::string)test_obj["first"]["second"]["third"][0];
}


int main(int argc, char** argv)
{

    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
};


