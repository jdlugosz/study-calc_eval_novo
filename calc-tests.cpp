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


class parse_error_matcher : public Catch::MatcherBase<EqEv_t::parse_error> {
    ptrdiff_t cursor;
    int errcode;
public:
    parse_error_matcher( ptrdiff_t cursor, int errcode ) : cursor { cursor }, errcode { errcode } {}

    static inline ptrdiff_t saved_cursor;
    static inline int saved_errcode;
    // Performs the test for this matcher
    bool match(const EqEv_t::parse_error& err ) const override {
        saved_cursor= err.cursor;
        saved_errcode= err.errcode;
        return cursor==err.cursor && errcode==err.errcode;
    }

    // Produces a string describing what this matcher does. It should
    // include any provided data (the begin/ end in this case) and
    // be written as if it were stating a fact (in the output it will be
    // preceded by the value under test).
    virtual std::string describe() const {
        std::ostringstream ss;
        ss << "position " << cursor << "  error code " << errcode;
        ss << ";  wanted (" << saved_cursor << ", " << saved_errcode << ")";
        return ss.str();
    }
};


// The builder function
inline parse_error_matcher Err ( ptrdiff_t cursor, int errcode ) {
    return { cursor, errcode };
}


TEST_CASE ("parsing errors") {

    EqEv_t calc;

    auto CheckBad = [&](string_view s, ptrdiff_t cursor, int errcode, const char* note_msg = "")
        {
        INFO(s);
        INFO(note_msg);
        CHECK_THROWS_MATCHES (calc.eval(s), EqEv_t::parse_error, Err(cursor,errcode));
        };

    CheckBad (" lady_bug = 17", 5,1);
    CheckBad ("green grass + 21", 6,1, "not an assignment");
    CheckBad ("foo = 123.456", 9,3, "does not take decimals");
    CheckBad ("bar = 0x234", 7,3, "does not take hex");
    REQUIRE ("red" == calc.eval("red=1000"));
    CheckBad ("pizza = red + green + 5", 20,2, "green not defined");
    calc.eval(" green= 18  ");
    auto var = calc.eval ("pizza = red + green + 5");  // should work now
    REQUIRE (var == "pizza");
    REQUIRE (calc.get_value(var) == 1023);
    CheckBad ("flarp = red - 17", 12,3, "no such operator");
    CheckBad ("grizzle   =  2345 + + 2", 20,2, "operator where term expected");
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

