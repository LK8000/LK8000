#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Waypointparser.h"
#include <assert.h>

TEST_CASE("testing the CUP file parsing function") {

	SUBCASE("CUPToLat") {
		CHECK(CUPToLat("4555.5555N") == doctest::Approx(46.472166666));
		CHECK(CUPToLat("4555.0S") == doctest::Approx(-45.9166666667));
		CHECK(CUPToLat("955.0S") == -9999);
		CHECK(CUPToLat("9555.55x") == -9999);
		CHECK(CUPToLat("9555.55") == -9999);
		CHECK(CUPToLat("9555") == -9999);
		CHECK(CUPToLat("955") == -9999);
		CHECK(CUPToLat("") == -9999);
		CHECK(CUPToLat(nullptr) == -9999);
	}

	SUBCASE("CUPToLon") {
		CHECK(CUPToLon("12555.5555E") == doctest::Approx(126.472166666));
		CHECK(CUPToLon("04555.0W") == doctest::Approx(-45.9166666667));
		CHECK(CUPToLon("9555.0S") == -9999);
		CHECK(CUPToLon("95555.55x") == -9999);
		CHECK(CUPToLon("99555.55") == -9999);
		CHECK(CUPToLon("9555") == -9999);
		CHECK(CUPToLon("955") == -9999);
		CHECK(CUPToLon("") == -9999);
		CHECK(CUPToLon(nullptr) == -9999);
	}

	SUBCASE("ReadAltitude") {
		CHECK(ReadAltitude("12345.12m") == doctest::Approx(12345.12));
		CHECK(ReadAltitude("12345.12M") == doctest::Approx(12345.12));
		CHECK(ReadAltitude("40502.362205f") == doctest::Approx(12345.12));
		CHECK(ReadAltitude("40502.362205F") == doctest::Approx(12345.12));
	}

	SUBCASE("ReadLength") {
		CHECK(ReadLength("12345.00m") == doctest::Approx(12345.12).epsilon(0.0001));
		CHECK(ReadLength("6.6657667387nm") == doctest::Approx(12345.12).epsilon(0.0001));
		CHECK(ReadLength("7.6708866800ml") == doctest::Approx(12345.12).epsilon(0.0001));
		CHECK(ReadLength("1000000.0m") == doctest::Approx(1000000.0).epsilon(0.0001));
		CHECK(ReadLength("539.956803nm") == doctest::Approx(1000000.0).epsilon(0.0001));
		CHECK(ReadLength("621.371192ml") == doctest::Approx(1000000.0).epsilon(0.0001));
	}

}
