#include "gtest/gtest.h"
#include <ten/json.hh>
#include <array>

#include "ten/logging.hh"
#include "ten/jsonstream.hh"
#include <map>

using namespace std;
using namespace ten;

const char json_text[] =
"{ \"store\": {"
"    \"book\": ["
"      { \"category\": \"reference\","
"        \"author\": \"Nigel Rees\","
"        \"title\": \"Sayings of the Century\","
"        \"price\": 8.95"
"      },"
"      { \"category\": \"fiction\","
"        \"author\": \"Evelyn Waugh\","
"        \"title\": \"Sword of Honour\","
"        \"price\": 12.99"
"      },"
"      { \"category\": \"fiction\","
"        \"author\": \"Herman Melville\","
"        \"title\": \"Moby Dick\","
"        \"isbn\": \"0-553-21311-3\","
"        \"price\": 8.99"
"      },"
"      { \"category\": \"fiction\","
"        \"author\": \"J. R. R. Tolkien\","
"        \"title\": \"The Lord of the Rings\","
"        \"isbn\": \"0-395-19395-8\","
"        \"price\": 22.99"
"      }"
"    ],"
"    \"bicycle\": {"
"      \"color\": \"red\","
"      \"price\": 19.95"
"    }"
"  }"
"}";


TEST(Json, Path1) {
    json o{json::load(json_text)};
    ASSERT_TRUE(o.get());

    static const char a1[] = "[\"Nigel Rees\", \"Evelyn Waugh\", \"Herman Melville\", \"J. R. R. Tolkien\"]";
    json r1{o.path("/store/book/author")};
    EXPECT_EQ(json::load(a1), r1);

    json r2{o.path("//author")};
    EXPECT_EQ(json::load(a1), r2);

    // jansson hashtable uses size_t for hash
    // we think this is causing the buckets to change on 32bit vs. 64bit
#if (__SIZEOF_SIZE_T__ == 4)
    static const char a3[] = "[{\"category\": \"reference\", \"author\": \"Nigel Rees\", \"title\": \"Sayings of the Century\", \"price\": 8.95}, {\"category\": \"fiction\", \"author\": \"Evelyn Waugh\", \"title\": \"Sword of Honour\", \"price\": 12.99}, {\"category\": \"fiction\", \"author\": \"Herman Melville\", \"title\": \"Moby Dick\", \"isbn\": \"0-553-21311-3\", \"price\": 8.99}, {\"category\": \"fiction\", \"author\": \"J. R. R. Tolkien\", \"title\": \"The Lord of the Rings\", \"isbn\": \"0-395-19395-8\", \"price\": 22.99}, {\"color\": \"red\", \"price\": 19.95}]";
#elif (__SIZEOF_SIZE_T__ == 8)
    static const char a3[] = "[{\"color\": \"red\", \"price\": 19.95}, {\"category\": \"reference\", \"author\": \"Nigel Rees\", \"title\": \"Sayings of the Century\", \"price\": 8.95}, {\"category\": \"fiction\", \"author\": \"Evelyn Waugh\", \"title\": \"Sword of Honour\", \"price\": 12.99}, {\"category\": \"fiction\", \"author\": \"Herman Melville\", \"title\": \"Moby Dick\", \"isbn\": \"0-553-21311-3\", \"price\": 8.99}, {\"category\": \"fiction\", \"author\": \"J. R. R. Tolkien\", \"title\": \"The Lord of the Rings\", \"isbn\": \"0-395-19395-8\", \"price\": 22.99}]";
#endif
    json r3{o.path("/store/*")};
    json t3{json::load(a3)};
    EXPECT_EQ(t3, r3);

#if (__SIZEOF_SIZE_T__ == 4)
    static const char a4[] = "[8.95, 12.99, 8.99, 22.99, 19.95]";
#elif (__SIZEOF_SIZE_T__ == 8)
    static const char a4[] = "[19.95, 8.95, 12.99, 8.99, 22.99]";
#endif
    json r4{o.path("/store//price")};
    EXPECT_EQ(json::load(a4), r4);

    static const char a5[] = "{\"category\": \"fiction\", \"author\": \"J. R. R. Tolkien\", \"title\": \"The Lord of the Rings\", \"isbn\": \"0-395-19395-8\", \"price\": 22.99}";
    json r5{o.path("//book[3]")};
    EXPECT_EQ(json::load(a5), r5);

    static const char a6[] = "\"J. R. R. Tolkien\"";
    json r6{o.path("/store/book[3]/author")};
    EXPECT_EQ(json::load(a6), r6);
    EXPECT_TRUE(json::load(a6) == r6);

    static const char a7[] = "[{\"category\": \"fiction\", \"author\": \"Evelyn Waugh\", \"title\": \"Sword of Honour\", \"price\": 12.99}, {\"category\": \"fiction\", \"author\": \"Herman Melville\", \"title\": \"Moby Dick\", \"isbn\": \"0-553-21311-3\", \"price\": 8.99}, {\"category\": \"fiction\", \"author\": \"J. R. R. Tolkien\", \"title\": \"The Lord of the Rings\", \"isbn\": \"0-395-19395-8\", \"price\": 22.99}]";
    json r7{o.path("/store/book[category=\"fiction\"]")};
    EXPECT_EQ(json::load(a7), r7);
}

TEST(Json, Path2) {
    json o{json::load("[{\"type\": 0}, {\"type\": 1}]")};
    EXPECT_EQ(json::load("[{\"type\":1}]"), o.path("/[type=1]"));
}

TEST(Json, FilterKeyExists) {
    json o{json::load(json_text)};
    ASSERT_TRUE(o.get());

    static const char a[] = "["
"      { \"category\": \"fiction\","
"        \"author\": \"Herman Melville\","
"        \"title\": \"Moby Dick\","
"        \"isbn\": \"0-553-21311-3\","
"        \"price\": 8.99"
"      },"
"      { \"category\": \"fiction\","
"        \"author\": \"J. R. R. Tolkien\","
"        \"title\": \"The Lord of the Rings\","
"        \"isbn\": \"0-395-19395-8\","
"        \"price\": 22.99"
"      }]";
    json r{o.path("//book[isbn]")};
    EXPECT_EQ(json::load(a), r);

    json r1{o.path("//book[doesnotexist]")};
    ASSERT_TRUE(r1.is_array());
    EXPECT_EQ(0, r1.asize());
}

TEST(Json, Truth) {
    json o{{}}; // empty init list
    EXPECT_TRUE(o.get("nothing").is_true() == false);
    EXPECT_TRUE(o.get("nothing").is_false() == false);
    EXPECT_TRUE(o.get("nothing").is_null() == false);
    EXPECT_TRUE(!o.get("nothing"));
}

TEST(Json, Path3) {
    json o{json::load(json_text)};
    ASSERT_TRUE(o.get());

    EXPECT_EQ(o, o.path("/"));
    EXPECT_EQ("Sayings of the Century",
        o.path("/store/book[category=\"reference\"]/title"));

    static const char text[] = "["
    "{\"type\":\"a\", \"value\":0},"
    "{\"type\":\"b\", \"value\":1},"
    "{\"type\":\"c\", \"value\":2},"
    "{\"type\":\"c\", \"value\":3}"
    "]";

    EXPECT_EQ(json(1), json::load(text).path("/[type=\"b\"]/value"));
}

TEST(Json, InitList) {
    json meta{
        { "foo", 17 },
        { "bar", 23 },
        { "baz", true },
        { "corge", json::array({ 1, 3.14159 }) },
        { "grault", json::array({ "hello", string("world") }) },
    };
    ASSERT_TRUE((bool)meta);
    ASSERT_TRUE(meta.is_object());
    EXPECT_EQ(meta.osize(), 5);
    EXPECT_EQ(meta["foo"].integer(), 17);
    EXPECT_EQ(meta["corge"][0].integer(), 1);
    EXPECT_EQ(meta["grault"][1].str(), "world");
}

template <class T>
inline void test_conv(T val, json j, json_type t) {
    json j2 = to_json(val);
    EXPECT_EQ((json_type)j2.type(), t);
    EXPECT_EQ(j, j2);
    T val2 = json_cast<T>(j2);
    EXPECT_EQ(val, val2);
}

template <class T, json_type TYPE = JSON_INTEGER>
inline void test_conv_num() {
    typedef numeric_limits<T> lim;
    T range[5] = { lim::min(), T(-1), 0, T(1), lim::max() };
    for (unsigned i = 0; i < 5; ++i)
        test_conv<T>(range[i], json(range[i]), TYPE);
}

TEST(Json, Conversions) {
    test_conv<string>(string("hello"), json::str("hello"), JSON_STRING);
    EXPECT_EQ(to_json("world"), json::str("world"));

    test_conv_num<short>();
    test_conv_num<int>();
    test_conv_num<long>();
    test_conv_num<long long>();
    test_conv_num<unsigned short>();
    test_conv_num<unsigned>();
#if ULONG_MAX < LLONG_MAX
    test_conv_num<unsigned long>();
#endif

    test_conv_num<double, JSON_REAL>();
    test_conv_num<float,  JSON_REAL>();

    test_conv<bool>(true,  json::jtrue(),  JSON_TRUE);
    test_conv<bool>(false, json::jfalse(), JSON_FALSE);
}

TEST(Json, Create) {
    json obj1{{}};
    EXPECT_TRUE((bool)obj1);
    obj1.set("test", "set");
    EXPECT_TRUE((bool)obj1.get("test"));
    json root{
        {"obj1", obj1}
    };
    EXPECT_EQ(root.get("obj1"), obj1);
    obj1.set("this", "that");
    EXPECT_EQ(root.get("obj1").get("this").str(), "that");
    json obj2{ {"answer", 42} };
    obj1.set("obj2", obj2);
    EXPECT_EQ(root.get("obj1").get("obj2"), obj2);
}

TEST(Json, Stream) {
    // TODO: improve these tests or don't. this is a hack anyway
    using namespace jsonstream_manip;
    std::stringstream ss;
    jsonstream s(ss);
    int8_t c = 'C'; // printable
    float fval = -1.28;
    double dval = -1.28;
    s << jsobject
        << "key1" << 1234
        << "key2" << "value"
        << "list" << jsarray << "1" << 2.0f << 3.14e-20 << 4 << 5 << jsend
        << "list2" << jsarray << jsobject << jsend << jsend
        << "max_dbl" << std::numeric_limits<double>::max()
        << "inf" << std::numeric_limits<float>::infinity()
        << "nan" << (1.0 / 0.0)
        << "vec" << std::vector<int>({0, 1, 2, 3})
        << "char" << c
        << "bool" << false
        << jsescape
        << "escape" << "\n\t\""
        << "noescape" << "blahblah"
        << "raw" << jsraw
            << "[]"
        << jsnoraw
        << "lahalha" << 666
        << "fval" << fval
        << "dval" << dval
        //<< "map" << std::map<const char *, int>({{"key", 1}})
    << jsend;
    VLOG(1) << ss.str();
    auto js = json::load(ss.str());
    EXPECT_TRUE((bool)js);
    EXPECT_EQ(js.get("fval"), js.get("dval"));
}

