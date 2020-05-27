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

         [[eosio::action]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );
};