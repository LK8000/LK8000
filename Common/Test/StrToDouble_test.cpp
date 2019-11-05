//#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "tchar.h"

// TODO : replace by include right header 
extern double StrToDouble(const TCHAR* Source, const TCHAR** Stop);



TEST_CASE("testing the StrToDouble function") {

	SUBCASE("Valid literal string imput") {
		CHECK(StrToDouble("0", nullptr) == 0);
		CHECK(StrToDouble("1", nullptr) == 1);
		CHECK(StrToDouble("2.2", nullptr) == 2.2);
		CHECK(StrToDouble("1234567890.0123456789", nullptr) == 1234567890.0123456789);
		CHECK(StrToDouble("-1.1", nullptr) == -1.1);
		CHECK(StrToDouble("  -1.1", nullptr) == -1.1);
		CHECK(StrToDouble("  1.1", nullptr) == 1.1);
		CHECK(StrToDouble("0.1", nullptr) == 0.1);
		CHECK(StrToDouble(".1", nullptr) == 0.1);
		CHECK(StrToDouble("6.6657667387", nullptr) == 6.6657667387);

		const char* sz = nullptr;
		CHECK(StrToDouble(".1", &sz) == 0.1);
		CHECK((sz && sz[0] == '\0') == true);
	}

	SUBCASE("partial Valid literal string imput") {
		CHECK(StrToDouble("0a", nullptr) == 0);
		CHECK(StrToDouble("1a", nullptr) == 1);
		CHECK(StrToDouble("2.2a", nullptr) == 2.2);
		CHECK(StrToDouble("1234567890.0123456789a", nullptr) == 1234567890.0123456789);
		CHECK(StrToDouble("-1.1a", nullptr) == -1.1);

        const char *sz = nullptr;
        CHECK(StrToDouble("0a", &sz) == 0);
        CHECK((sz && sz[0] == 'a') == true);

        CHECK(StrToDouble("1b", &sz) == 1);
        CHECK((sz && sz[0] == 'b') == true);

        CHECK(StrToDouble("2.2c", &sz) == 2.2);
        CHECK((sz && sz[0] == 'c') == true);

        CHECK(StrToDouble("1234567890.0123456789d", &sz) == 1234567890.0123456789);
        CHECK((sz && sz[0] == 'd') == true);

        CHECK(StrToDouble("-1.1e", &sz) == -1.1);
        CHECK((sz && sz[0] == 'e') == true);
    }

	SUBCASE("invalid literal string imput") {
		const char* sz = nullptr;
		CHECK(StrToDouble("a", &sz) == 0);
		CHECK((sz && sz[0] == 'a') == true);

		CHECK(StrToDouble(" ba", &sz) == 0);
		CHECK((sz && sz[0] == 'b') == true);

		CHECK(StrToDouble(",2", &sz) == 0);
		CHECK((sz && sz[0] == ',') == true);
	}

	SUBCASE("invalid literal empty string") {
		const char* sz = nullptr;
		CHECK(StrToDouble("", &sz) == 0);
		CHECK(sz == nullptr);

		CHECK(StrToDouble(nullptr, nullptr) == 0);
	}
}
