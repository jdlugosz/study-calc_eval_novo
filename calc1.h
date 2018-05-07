#pragma once

#include <string_view>
#include <optional>
#include <stdexcept>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/container/flat_map.hpp>

/*

equation ::= variable '=' terms
variable ::= [:isalpha:]+   // no case or other normalization specified
terms ::= value { binop value }*
value ::= variable | number
number ::= [:isdigit:]+   // positive only; no negative sign

*/




class EqEv_t {
public:
    using Value_type = boost::multiprecision::cpp_int;
    struct parse_error : std::runtime_error {
        ptrdiff_t cursor;
        int errcode;
        parse_error (ptrdiff_t cursor, int errcode);
    };
private:
    enum op_enum { OP_plus=1 };
    boost::container::flat_map<std::string,Value_type> variables;
	void skip_ws (std::string_view&);
	std::optional<std::string> read_identifier (std::string_view&);
	std::string normalize_identifier (const char* start, int len);
	void read_required (char c, std::string_view&);
	std::optional<Value_type> read_terms (std::string_view&);
	std::optional<Value_type> read_value (std::string_view&);
    std::optional<op_enum> read_binop (std::string_view&);
    std::optional<Value_type> read_number (std::string_view&);
public:
	std::string eval (std::string_view);
    std::optional<Value_type> get_value (std::string_view) const;
    void set_value (std::string_view, const std::optional<Value_type>&);
};
