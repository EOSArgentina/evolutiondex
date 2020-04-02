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
      [[eosio::action]] void votefee(name user, symbol sym, int fee_index_voted);
      [[eosio::action]] void openfeetable(name user, symbol sym);
      [[eosio::action]] void closevote(name user, symbol sym);
      [[eosio::action]] void updatefee(symbol sym);
      [[eosio::on_notify("evolutiondex::addliquidity")]] void addliquidity(name user, asset to_buy, 
        extended_asset max_ext_asset1, extended_asset max_ext_asset2);
      [[eosio::on_notify("evolutiondex::remliquidity")]] void remliquidity(name user, asset to_sell,
        extended_asset min_ext_asset1, extended_asset min_ext_asset2);
      [[eosio::on_notify("evolutiondex::transfer")]] void transfer(const name& from, const name& to, 
           const asset& quantity, const string&  memo );

   private:

      const vector <int> fee_vector{1,2,4,8,16,32,64,128,256}; 

      int median(symbol sym);
      int get_index(int number);
      void addvote(symbol sym, int fee_index, int64_t amount);
      void add_balance(name user, asset to_add);
      asset bring_balance(name user, symbol sym);

      struct [[eosio::table]] feeaccount {
         symbol sym;
         int fee_index_voted;
         uint64_t primary_key()const { return sym.code().raw(); }
      };

      struct [[eosio::table]] feetable {
         symbol sym;
         vector <int64_t> votes;
         uint64_t primary_key()const { return sym.code().raw(); }
      };

      typedef eosio::multi_index<"feeaccount"_n, feeaccount> feeaccounts;
      typedef eosio::multi_index<"feetable"_n, feetable> feetables;

      struct [[eosio::table]] account {
         asset    balance;
         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "accounts"_n, account > accounts;

};
