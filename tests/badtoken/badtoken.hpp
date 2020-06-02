#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("badtoken")]] badtoken : public contract {
   public:
      using contract::contract;

      [[eosio::action]] void transfer( const name& from, const name& to, 
         const asset& quantity, const string& memo );
      [[eosio::on_notify("evolutiondex::transfer")]] void ontransfer( const name& from, const name& to, 
         const asset& quantity, const string& memo );
      [[eosio::on_notify("evolutiondex::addliquidity")]] void addliquidity(name user, asset to_buy, 
      asset max_asset1, asset max_asset2);
      [[eosio::on_notify("evolutiondex::remliquidity")]] void remliquidity(name user, asset to_sell,
      asset min_asset1, asset min_asset2);
};