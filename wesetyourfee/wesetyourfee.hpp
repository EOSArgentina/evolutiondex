#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <cmath>

using namespace eosio;
using namespace std;


class [[eosio::contract("wesetyourfee")]] wesetyourfee : public contract {
   public:

      using contract::contract;
      [[eosio::action]] void newbalance(name user, extended_asset new_balance);
      [[eosio::action]] void changefee(extended_symbol sym, int newfee);
};
