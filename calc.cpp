#include <iostream>
#include "d3/twostep.h"
#include "d3/overflow.h"
#include "calc1.h"

namespace D3 = Dlugosz::d3;
using namespace D3::twostep;
using D3::sSize;

using std::string_view;
using std::string;
using std::optional;



// ===== variable storage

auto EqEv_t::get_value(string_view name) const -> optional<Value_type>
{
    string key { name };
    auto it= variables.find(key);
    optional<Value_type> retval;
    if (it != End(variables))  retval= it->second;
    return retval;
}


void EqEv_t::set_value (string_view name, const optional<Value_type>& val)
{
    string key { name };
    if (!val)  variables.erase(key);
    variables.insert_or_assign (key, *val);
}


// there was no specification as to being case-insensitive or normalized,
// so take it as strict exact chars.  This function is where the raw
// spelling can be massaged into a canonocal form (e.g. case folded).
string EqEv_t::normalize_identifier (const char* start, int len)
{
	return {start, start+len};
}


// ===== parser

namespace {

struct prim_parse_error {
    const char* pos;
    int errcode;
};

[[noreturn]]
void raise_error (string_view input, int code)
{
    prim_parse_error Err { Data(input), code };
    throw Err;
}

}


EqEv_t::parse_error::parse_error (ptrdiff_t cursor, int errcode)
    : runtime_error{"parse error"}, cursor{cursor}, errcode{errcode}
{ }


void EqEv_t::skip_ws (string_view& s)
{
	while (!s.empty() && isspace(static_cast<unsigned char>(s.front())))  s.remove_prefix(1);
}


void EqEv_t::read_required (char c, string_view& input)
{
	if (Size(input) > 0 && input.front() == c)  {
		input.remove_prefix(1);
        skip_ws (input);
        return;
	}
	raise_error (input, 1);
}



optional<string> EqEv_t::read_identifier (string_view& input)
{
	optional<string> retval;
	int len= 0;
	int Max = sSize(input);
	for (; len < Max; ++len)  if (!isalpha(static_cast<const unsigned char>(input[len]))) break;
	if (len==0)  return retval;
	retval.emplace(normalize_identifier(&(*Begin(input)),len));
	// after all went well, update the input range to take these chars
	input.remove_prefix(len);
    skip_ws(input);
	return retval;
}


auto EqEv_t::read_value (string_view& input) -> optional<Value_type>
{
    ;
    // variable | number
    optional<Value_type> num= read_number(input);
    if (!num) {
        auto var= read_identifier(input);
        if (var)  num= get_value(*var);
    }
    return num;
}


auto EqEv_t::read_number(string_view& input) -> optional<Value_type>
{
    optional<Value_type> retval;
    int len= 0;
    int Max = sSize(input);
    for (; len < Max; ++len)  if (!isdigit(static_cast<const unsigned char>(input[len]))) break;
    if (len==0)  return retval;
    string numword (Begin(input), Begin(input)+len);  // Boost cpp_num doesn't know about string_view
    retval.emplace(numword);
    input.remove_prefix(len);
    skip_ws(input);
    return retval;
}


auto EqEv_t::read_binop (string_view& input) -> optional<op_enum>
{
    optional<op_enum> retval;
    if (Size(input) > 0) {
        char c= input.front();
        // I only have one operator, and it is single-character.
        if (c == '+') {
            retval = OP_plus;
            input.remove_prefix(1);
            skip_ws (input);
        }
    }
    return retval;
}


auto EqEv_t::read_terms (string_view & original_input) -> optional<Value_type>
{
    optional<Value_type> retval;
    string_view input = original_input;

    auto nextval = read_value (input);
    if (!nextval)  return retval;
    Value_type total = *nextval;
    for (;;)
    {
        auto op = read_binop (input);
        if (!op)  break;
        nextval = read_value (input);
        if (!nextval)  raise_error (input, 2);
        // I only have one operator, addition
        total += *nextval;
    }
    retval= std::move(total);
    original_input= input;
	return retval;
}


string EqEv_t::eval (string_view input)
{
    const auto offset_zero= Data(input);
    try {
	    skip_ws (input);
	    auto LHS= read_identifier(input);
	    read_required ('=', input);
	    auto terms_result= read_terms(input);
        if (!input.empty())  raise_error (input, 3);
        set_value (*LHS, terms_result);
        return *LHS;
    }
    catch (const prim_parse_error& err) {
        const auto offset= err.pos - offset_zero;
        throw parse_error {offset, err.errcode};
    }
}


void test (EqEv_t& calc, string_view s)
{
    try {
        string varname= calc.eval (s);
        std::cout << "evaluated: " << varname << " = " << *calc.get_value(varname) << '\n';
    }
    catch (const char* msg) {
        // temporary troubleshooting only!
        std::cout << " <<< error: " << msg << '\n';
    }
}

#if 0
int main()
{
	EqEv_t calc;
	test (calc, "  left = 5");
	test (calc, "right=6  ");
	test (calc, "total =left +right");
}
#endif

