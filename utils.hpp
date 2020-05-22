#pragma once
//#include <charconv>
#include <string>
#include <algorithm>
#include <iterator>

#include <eosio/asset.hpp>
#include "safe.hpp"

string_view trim(string_view sv) {
    sv.remove_prefix(std::min(sv.find_first_not_of(" "), sv.size())); // left trim
    sv.remove_suffix(std::min(sv.size()-sv.find_last_not_of(" ")-1, sv.size())); // right trim    
    return sv;
}
 
vector<string_view> split(string_view str, string_view delims = " ")
{
    vector<string_view> res;
    std::size_t current, previous = 0;
    current = str.find_first_of(delims);
    while (current != std::string::npos) {
        res.push_back(trim(str.substr(previous, current - previous)));
        previous = current + 1;
        current = str.find_first_of(delims, previous);
    }
    res.push_back(trim(str.substr(previous, current - previous)));
    return res;
}

bool starts_with(string_view sv, string_view s) {
    return sv.size() >= s.size() && sv.compare(0, s.size(), s) == 0;
}

template <class T>
void to_int(string_view sv, T& res) {
    res = 0;
    T p = 1;
    for( auto itr = sv.rbegin(); itr != sv.rend(); ++itr ) {
        check( *itr <= '9' && *itr >= '0', "invalid character");
        res += p * T(*itr-'0');
        p   *= T(10);
    }
}
template <class T>
void precision_from_decimals(int8_t decimals, T& p10)
{
    check(decimals <= 18, "precision should be <= 18");
    p10 = 1;
    T p = decimals;
    while( p > 0  ) {
        p10 *= 10; --p;
    }
}

asset asset_from_string(string_view from)
{
    string_view s = trim(from);

    // Find space in order to split amount and symbol
    auto space_pos = s.find(' ');
    check(space_pos != string::npos, "Asset's amount and symbol should be separated with space");
    auto symbol_str = trim(s.substr(space_pos + 1));
    auto amount_str = s.substr(0, space_pos);

    // Ensure that if decimal point is used (.), decimal fraction is specified
    auto dot_pos = amount_str.find('.');
    if (dot_pos != string::npos) {
        check(dot_pos != amount_str.size() - 1, "Missing decimal fraction after decimal point");
    }

    // Parse symbol
    uint8_t precision_digit = 0;
    if (dot_pos != string::npos) {
        precision_digit = amount_str.size() - dot_pos - 1;
    }

    symbol sym = symbol(symbol_str, precision_digit);

    // Parse amount
    safe<int64_t> int_part, fract_part;
    if (dot_pos != string::npos) {
        to_int(amount_str.substr(0, dot_pos), int_part);
        to_int(amount_str.substr(dot_pos + 1), fract_part);
        if (amount_str[0] == '-') fract_part *= -1;
    } else {
        to_int(amount_str, int_part);
    }

    safe<int64_t> amount = int_part;
    safe<int64_t> precision; precision_from_decimals(sym.precision(), precision);
    amount *= precision;
    amount += fract_part;

    return asset(amount.value, sym);
}
