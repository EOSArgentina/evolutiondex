#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <cmath>

using namespace eosio;
using namespace std;

namespace evolution {

   class [[eosio::contract("evolutiondex")]] evolutiondex : public contract {
      public:
         const int64_t MAX = eosio::asset::max_amount;
         const int64_t INIT_MAX = 1000000000000000;  // 10^15 

         using contract::contract;
         [[eosio::action]] void inittoken(name user, symbol new_symbol, 
           extended_asset initial_pool1, extended_asset initial_pool2, 
           int initial_fee, name fee_contract);
         [[eosio::on_notify("*::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);
         [[eosio::action]] void openext( const name& user, const name& payer, const extended_symbol& ext_symbol);
         [[eosio::action]] void closeext ( const name& user, const name& to, const extended_symbol& ext_symbol, string memo);
         [[eosio::action]] void withdraw(name user, name to, extended_asset to_withdraw, string memo);
         [[eosio::action]] void addliquidity(name user, asset to_buy, asset max_asset1, asset max_asset2);
         [[eosio::action]] void remliquidity(name user, asset to_sell, asset min_asset1, asset min_asset2);
         [[eosio::action]] void exchange( name user, symbol_code pair_token, extended_asset ext_asset_in, asset min_expected );
         [[eosio::action]] void changefee(symbol_code pair_token, int newfee);

         [[eosio::action]] void transfer(const name& from, const name& to, 
           const asset& quantity, const string&  memo );
         [[eosio::action]] void open( const name& owner, const symbol& symbol, const name& ram_payer );
         [[eosio::action]] void close( const name& owner, const symbol& symbol );
         
      private:

         struct [[eosio::table]] account {
            asset    balance;
            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] evodexaccount {
            extended_asset   balance;
            uint64_t id;
            uint64_t primary_key()const { return id; }
            uint128_t secondary_key()const { return 
              make128key(balance.contract.value, balance.quantity.symbol.raw() ); }
         };

         static uint128_t make128key(uint64_t a, uint64_t b);

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;
            extended_asset    pool1;
            extended_asset    pool2;
            int fee;
            name fee_contract;
            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "evodexacnts"_n, evodexaccount,
         indexed_by<"extended"_n, const_mem_fun<evodexaccount, uint128_t, 
           &evodexaccount::secondary_key>> > evodexacnts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;
         typedef eosio::multi_index< "accounts"_n, account > accounts;

         void add_signed_ext_balance( const name& owner, const extended_asset& value );
         void add_signed_liq(name user, asset to_buy, bool is_buying, asset max_asset1, asset max_asset2);
         void memoexchange(name user, extended_asset ext_asset_in, string_view details);
         void give(extended_asset ext_asset_in, string_view details);         
         extended_asset process_exch(symbol_code evo_token, extended_asset paying, asset min_expected);
         int64_t compute(int64_t x, int64_t y, int64_t z, int fee);
         asset string_to_asset(string input);

         void add_balance( const name& owner, const asset& value, const name& ram_payer );
         void sub_balance( const name& owner, const asset& value );
   };
}