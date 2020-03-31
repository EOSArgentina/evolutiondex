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
      [[eosio::action]] void updatefee(symbol sym, int newfee);
};
