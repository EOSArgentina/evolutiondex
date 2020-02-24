#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <cmath>

using namespace eosio;
using namespace std;

namespace evolution {   // necesito el namespace? que hace?

   class [[eosio::contract("evolutiondex")]] evolutiondex : public contract {
      public:
         const int64_t MAX = eosio::asset::max_amount;
         const double HALF = 0.5;

         using contract::contract;
         [[eosio::action]] void inittoken(extended_symbol new_ext_sym, extended_asset ext_asset1, 
           extended_asset ext_asset2, int initial_fee, name fee_contract);
         [[eosio::on_notify("*::transfer")]] void deposit(name from, name to, asset quantity, string memo);
         [[eosio::action]] void open( const name& user, const name& payer, const extended_symbol& ext_symbol, bool is_evotoken);
         [[eosio::action]] void close ( const name& user, const extended_symbol& ext_symbol);
         [[eosio::action]] void withdraw(name user, extended_asset to_withdraw);
         [[eosio::action]] void addliquidity(name user, extended_asset to_buy, extended_asset max_ext_asset1, extended_asset max_ext_asset2);
         [[eosio::action]] void remliquidity(name user, extended_asset to_sell, extended_asset min_ext_asset1, extended_asset min_ext_asset2);
         [[eosio::action]] void exchange( name user, extended_symbol through, asset asset1, asset asset2 );
         [[eosio::on_notify("*::changefee")]] void changefee(symbol sym, int newfee);
         [[eosio::action]] void transfer(const name& from, const name& to, 
           const asset& quantity, const string&  memo );

         
      private:

         struct [[eosio::table]] evodexaccount {
            extended_asset   balance;
            bool is_evotoken;
            uint64_t id;            
            uint64_t primary_key()const { return id; }
            uint128_t secondary_key()const { return 
              make128key(balance.contract.value, balance.quantity.symbol.code().raw() ); }
         };

         static uint128_t make128key(uint64_t a, uint64_t b);

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            extended_asset    connector1;
            extended_asset    connector2;
            int fee;
            name fee_contract;
            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "evodexacnts"_n, evodexaccount,
         indexed_by<"extended"_n, const_mem_fun<evodexaccount, uint128_t, 
           &evodexaccount::secondary_key>> > evodexacnts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         void add_balance( const name& owner, const extended_asset& value, bool is_evotoken );
         int64_t compute(int64_t x, int64_t y, int64_t z, int fee);
         void add_or_remove(name user, extended_asset to_buy, bool is_buying, extended_asset max_ext_asset1, extended_asset max_ext_asset2);
         void notify_fee_contract( name user, extended_asset new_balance);
   };
}
