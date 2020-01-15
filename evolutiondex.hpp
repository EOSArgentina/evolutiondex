// El frontend o el usuario deben verificar bien quiénes son los connectors
// en cada caso. Puede haber connectors fantasma o bien que cobren fees en la transfer 
// y entonces no coincidan los saldos. Después no se podría hacer withdraw.

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
         const int64_t MAX = 1000000000000000000;
         const double HALF = 0.5;

         using contract::contract;
         [[eosio::action]] void inittoken(name user, name smartctr1, asset asset1, name smartctr2, asset asset2, asset new_token, int initial_fee, name fee_contract);
         [[eosio::on_notify("*::transfer")]] void deposit(name from, name to, asset quantity, string memo);
         [[eosio::action]] void open( const name& user, const name& payer, const name& contract, const asset& asset_to_open );
         [[eosio::action]] void close ( const name& user, const name& smartctr, const asset& asset_to_open );
         [[eosio::action]] void withdraw(name account, const name smartctr, asset to_withdraw);
         [[eosio::action]] void buytoken(name user, asset asset1, asset asset2, asset min_expected);
         [[eosio::action]] void selltoken(name user, asset asset1, asset asset2, asset max_expected);
         [[eosio::action]] void exchange( name user, symbol through, asset asset1, asset asset2 );
         [[eosio::on_notify("*::changefee")]] void changefee(symbol sym, int newfee);
         [[eosio::action]] void transfer(const name& from, const name& to, 
           const asset& quantity, const string&  memo );

      private:

         struct [[eosio::table]] evodexaccount {
            extended_asset   balance;
            uint64_t primary_key()const { return balance.contract.value + balance.quantity.symbol.code().raw(); }
            // elegir una operación que tenga ínfimas chances de colapso
         };

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            extended_asset    connector1;
            extended_asset    connector2;
/*          float weight1;
            float weight2;*/
            int fee;
            name fee_contract;
            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "evodexacnts"_n, evodexaccount > evodexacnts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         void add_balance( const name& owner, const extended_asset& value );
         void operate_token(name user, asset asset1, asset asset2,
           asset min_expected, bool is_exchange_operation);
         asset chargefee(name user, asset quantity, name smartctr, int fee);
         void notify_fee_contract( name user, asset new_balance);
   };
}
