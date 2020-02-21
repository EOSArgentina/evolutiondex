#include "wesetyourfee.hpp"

// using namespace setfee;

void wesetyourfee::newbalance(name user, extended_asset new_balance){ // se podría hacer addbalance
    require_auth("evolutiondex"_n);
}

void wesetyourfee::changefee(extended_symbol sym, int newfee){
}
