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
      [[eosio::action]] void newbalance(name user, asset new_balance);
      [[eosio::action]] void votefee(name user, symbol sym, int fee_index_voted);
      [[eosio::action]] void openfeetable(name user, symbol sym);
      [[eosio::action]] void closevote(name user, symbol sym);
      [[eosio::action]] void changefee(symbol sym, int newfee);

   private:

      const vector <int> fee_vector{1,2,4,8,16,32,64,128,256}; 

      int median(symbol sym);
      void addvote(symbol sym, int fee_index, int64_t amount);
      asset bring_balance(name user, symbol sym);

      struct [[eosio::table]] feeaccount {
         asset balance;
         int fee_index_voted;
         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };

      struct [[eosio::table]] feetable {
         symbol sym;
         vector <int64_t> votes;
         uint64_t primary_key()const { return sym.code().raw(); }
      };

      typedef eosio::multi_index<"feeaccount"_n, feeaccount> feeaccounts;
      typedef eosio::multi_index<"feetable"_n, feetable> feetables;

      struct [[eosio::table]] evodexaccount {
         extended_asset   balance;
         uint64_t id;            
         uint64_t primary_key()const { return id; }
         uint128_t secondary_key()const { return 
            make128key(balance.contract.value, balance.quantity.symbol.code().raw() ); }
      };

      typedef eosio::multi_index< "evodexacnts"_n, evodexaccount,
        indexed_by<"extended"_n, const_mem_fun<evodexaccount, uint128_t, 
        &evodexaccount::secondary_key>> > evodexacnts;

      static uint128_t make128key(uint64_t a, uint64_t b);
};
