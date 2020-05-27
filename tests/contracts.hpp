#pragma once
#include <eosio/testing/tester.hpp>

namespace eosio { namespace testing {

struct contracts {
   static std::vector<uint8_t> system_wasm() { return read_wasm("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.system/eosio.system.wasm"); }
   static std::vector<char>    system_abi() { return read_abi("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.system/eosio.system.abi"); }

   static std::vector<uint8_t> token_wasm() { return read_wasm("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.token/eosio.token.wasm"); }
   static std::vector<char>    token_abi() { return read_abi("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.token/eosio.token.abi"); }

   static std::vector<uint8_t> msig_wasm() { return read_wasm("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.msig/eosio.msig.wasm"); }
   static std::vector<char>    msig_abi() { return read_abi("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.msig/eosio.msig.abi"); }

   static std::vector<uint8_t> wrap_wasm() { return read_wasm("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.wrap/eosio.wrap.wasm"); }
   static std::vector<char>    wrap_abi() { return read_abi("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.wrap/eosio.wrap.abi"); }

   static std::vector<uint8_t> bios_wasm() { return read_wasm("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.bios/eosio.bios.wasm"); }
   static std::vector<char>    bios_abi() { return read_abi("/home/sergio/Dropbox/eos/contracts/eosio.contracts/build/contracts/eosio.bios/eosio.bios.abi"); }

   static std::vector<uint8_t> evolutiondex_wasm() { return read_wasm("/home/sergio/Dropbox/eos/contracts/evo/tests/../evolutiondex.wasm"); }
   static std::vector<char>    evolutiondex_abi() { return read_abi("/home/sergio/Dropbox/eos/contracts/evo/tests/../evolutiondex.abi"); }

   static std::vector<uint8_t> badtoken_wasm() { return read_wasm("/home/sergio/Dropbox/eos/contracts/evo/tests/../badtoken.wasm"); }
   static std::vector<char>    badtoken_abi() { return read_abi("/home/sergio/Dropbox/eos/contracts/evo/tests/../badtoken.abi"); }
}; 
}} //ns eosio::testing
