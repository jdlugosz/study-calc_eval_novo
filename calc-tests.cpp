#include "calc1.h"

#include <catch.hpp>

using std::string;
using std::string_view;
using std::cout;

void show_vars (const EqEv_t& calc)
{
    for (auto [name,value] : calc.get_variables()) {
        cout << name << " = " << value << '\n';
    }
}

TEST_CASE ("Demo1 Calc tests") {
    EqEv_t calc;
    string var;
    var= calc.eval("  left = 5");
    REQUIRE( var == "left");
    REQUIRE( calc.get_value(var) == 5);
    var= calc.eval("right=6  ");
    REQUIRE( var == "right");
    REQUIRE( calc.get_value(var) == 6);
    var= calc.eval("total =left +right");
    REQUIRE( var == "total");
    REQUIRE( calc.get_value(var) == 11);

    show_vars (calc);
}


struct should_error_tester {
    EqEv_t calc;
    void operator() (string_view s)
    {
        CHECK_THROWS_AS (calc.eval(s), EqEv_t::parse_error);
    }
};

TEST_CASE ("parsing errors") {

    should_error_tester be_bad;

    be_bad (" lady_bug = 17");  // no underscores allowed in names

    try {
        EqEv_t calc;
        calc.eval (" lady_bug = 17");
    }
    catch (const EqEv_t::parse_error& err) {
        cout << "Exception details: " << err.what() << ", position= " << err.cursor << ", code= " << err.errcode << '\n';
    }

    be_bad ("green grass + 21");  // not an assignment
    be_bad ("foo = 123.456");  // does not take decimals
    be_bad ("bar = 0x234");  // does not take hex
    REQUIRE ("red" == be_bad.calc.eval("red=1000"));
    be_bad ("pizza = red + green + 5");  // green not defined
    be_bad.calc.eval(" green= 18  ");
    auto var = be_bad.calc.eval ("pizza = red + green + 5");  // should work now
    REQUIRE (var == "pizza");
    REQUIRE (be_bad.calc.get_value(var) == 1023);
    be_bad ("flarp = red - 17");  // no such operator
    be_bad ("grizzle   =  2345 + + 2");
}


TEST_CASE ("large values") {
    EqEv_t calc;
    auto var= calc.eval ("one = 123456789 + 234567890 + 2345678901");
    calc.eval ("two = one + one + one + 9876543210000");
    calc.eval ("three = two + one + 1");
    calc.eval ("four = three + two + 1000000 + three + one");
    calc.eval ("five = 800000000000000000000 + four");

    show_vars(calc);
}


TEST_CASE ("case-sensitive names") {
    EqEv_t calc;
    calc.eval ("polish = 100");
    calc.eval ("Polish = 2000");
    calc.eval ("abcd  =  polish + Polish");
    calc.eval ("abCD = 17 + abcd");
    show_vars(calc);
    REQUIRE (calc.get_value("polish")==100);
    REQUIRE (calc.get_value("Polish")==2000);
    REQUIRE (calc.get_value("abcd")==2100);
    REQUIRE (calc.get_value("abCD")==2117);
}

TEST_CASE ("zero") {
    EqEv_t calc;
    auto var= calc.eval ("zero = 0");
    REQUIRE (var == "zero");
    REQUIRE (calc.get_value(var)==0);
    var= calc.eval ("zippy = zero + zero + 0 + 42");
    CHECK (var == "zippy");
    CHECK (calc.get_value(var)==42);
}

// use and update

