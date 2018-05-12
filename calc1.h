#pragma once
#ifndef I_223AFA6C_750C_4D9F_9463_9F1A3262F570
#define I_223AFA6C_750C_4D9F_9463_9F1A3262F570

#include <string_view>
#include <map>
#include <optional>
#include <stdexcept>
#include <boost/multiprecision/cpp_int.hpp>  // https://www.boost.org/doc/libs/1_67_0/libs/multiprecision/doc/html/index.html

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
    std::map<std::string,Value_type,std::less<>> variables;
	static void skip_ws (std::string_view&);
    template <typename Pred>
    static int peek_token (std::string_view& input, Pred p);
    static std::optional<std::string> read_identifier (std::string_view&);
	static std::string normalize_identifier (const char* start, int len);
	static void read_required (char c, std::string_view&);
	std::optional<Value_type> read_terms (std::string_view&) const;
	std::optional<Value_type> read_value (std::string_view&) const;
    static std::optional<op_enum> read_binop (std::string_view&);
    static std::optional<Value_type> read_number (std::string_view&);
    static Value_type numword_to_value (std::string_view&);

public:
	std::string eval (std::string_view);
    std::optional<Value_type> get_value (std::string_view) const;
    void set_value (std::string_view, const std::optional<Value_type>&);
    const auto& get_variables() const { return variables; }
};

#endif
