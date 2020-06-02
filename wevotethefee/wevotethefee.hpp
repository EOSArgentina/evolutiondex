#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <cmath>

using namespace eosio;
using namespace std;

class [[eosio::contract("wevotethefee")]] wevotethefee : public contract {
   public:

      using contract::contract;
      [[eosio::action]] void votefee(name user, symbol_code token, int fee_index_voted);
      [[eosio::action]] void openfeetable(name user, symbol_code token);
      [[eosio::action]] void closevote(name user, symbol_code token);
      [[eosio::action]] void closefeetable(name user, symbol_code token);
      [[eosio::action]] void updatefee(symbol_code token);
      [[eosio::on_notify("evolutiondex::addliquidity")]] void addliquidity(name user, asset to_buy, 
        asset max_asset1, asset max_asset2);
      [[eosio::on_notify("evolutiondex::remliquidity")]] void remliquidity(name user, asset to_sell,
        asset min_asset1, asset min_asset2);
      [[eosio::on_notify("evolutiondex::transfer")]] void transfer(const name& from, const name& to, 
           const asset& quantity, const string&  memo );

   private:

      const vector <int> fee_vector{1,2,3,5,7,10,15,20,30,50,75,100,150,200,300};
      int median(symbol_code token);
      int get_index(int number);
      void addvote(symbol_code token, int fee_index, int64_t amount);
      void add_balance(name user, asset to_add);
      asset bring_balance(name user, symbol_code token);

      struct [[eosio::table]] feeaccount {
         symbol_code token;
         int fee_index_voted;
         uint64_t primary_key()const { return token.raw(); }
      };

      struct [[eosio::table]] feetable {
         symbol_code token;
         vector <int64_t> votes;
         uint64_t primary_key()const { return token.raw(); }
      };

      typedef eosio::multi_index<"feeaccount"_n, feeaccount> feeaccounts;
      typedef eosio::multi_index<"feetable"_n, feetable> feetables;

      struct [[eosio::table]] account {
         asset    balance;
         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "accounts"_n, account > accounts;
};
