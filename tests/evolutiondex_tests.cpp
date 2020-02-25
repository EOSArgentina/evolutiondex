#include <eosio/chain/abi_serializer.hpp>
#include <eosio/testing/tester.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>

#include <boost/test/unit_test.hpp>

#include <contracts.hpp>
#include <cmath>

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo = fc::mutable_variant_object;

class eosio_token_tester : public tester {
public:

   eosio_token_tester() {
      produce_blocks( 2 );

      create_accounts( { N(alice), N(bob), N(carol), N(eosio.token), N(evolutiondex),
        N(wesetyourfee) } );
      produce_blocks( 2 );

      set_code( N(eosio.token), contracts::token_wasm() );
      set_abi( N(eosio.token), contracts::token_abi().data() );

      set_code( N(evolutiondex), contracts::evolutiondex_wasm() );
      set_abi( N(evolutiondex), contracts::evolutiondex_abi().data() );

      set_code( N(wesetyourfee), contracts::wesetyourfee_wasm() );
      set_abi( N(wesetyourfee), contracts::wesetyourfee_abi().data() );

      produce_blocks();

      const auto& accnt1 = control->db().get<account_object,by_name>( N(eosio.token) );
      abi_def abi1;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt1.abi, abi1), true);
      abi_ser.set_abi(abi1, abi_serializer_max_time);

   }


   fc::variant get_balance( name smartctr, name user, name table, int64_t id, string struc) 
   {
      vector<char> data = get_row_by_account( smartctr, user, table, name(id) );
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant( struc, data, abi_serializer_max_time );
   }

   action_result push_action( const account_name& smartctr, const account_name& signer, const action_name &name, const variant_object &data ) {
      string action_type_name = abi_ser.get_action_type(name);

      action act;
      act.account = smartctr;
      act.name    = name;
      act.data    = abi_ser.variant_to_binary( action_type_name, data, abi_serializer_max_time );

      return base_tester::push_action( std::move(act), signer.to_uint64_t() );
   }


   action_result create( account_name issuer,
                asset        maximum_supply ) {

      return push_action( N(eosio.token), N(eosio.token), N(create), mvo()
           ( "issuer", issuer)
           ( "maximum_supply", maximum_supply)
      );
   }

   action_result issue( account_name issuer, account_name to, asset quantity, string memo ) {
      return push_action( N(eosio.token), issuer, N(issue), mvo()
           ( "to", to)
           ( "quantity", quantity)
           ( "memo", memo)
      );
   }

   action_result transfer( account_name from,
                  account_name to,
                  asset        quantity,
                  string       memo ) {
      return push_action( N(eosio.token), from, N(transfer), mvo()
           ( "from", from)
           ( "to", to)
           ( "quantity", quantity)
           ( "memo", memo)
      );
   }

   action_result open( name owner, symbol sym, name ram_payer ) {
      return push_action( N(evolutiondex), owner, N(open), mvo()
           ( "owner", owner)
           ( "symbol", sym)
           ( "ram_payer", ram_payer)
      );
   }

   action_result openext( name user, name payer, extended_symbol ext_symbol ) {
      return push_action( N(evolutiondex), user, N(openext), mvo()
           ( "user", user)
           ( "payer", payer)
           ( "ext_symbol", ext_symbol)
      );
   }

   action_result closeext ( const name user, const extended_symbol ext_symbol ){ 
      return push_action( N(evolutiondex), user, N(closeext), mvo()
           ( "user", user)
           ( "ext_symbol", ext_symbol)
      );
   }

   action_result withdraw(name user, extended_asset to_withdraw) {
      return push_action( N(evolutiondex), user, N(withdraw), mvo()
         ( "user", user )
         ( "to_withdraw", to_withdraw )
      );
   }

   action_result inittoken( name user, symbol new_symbol, extended_asset ext_asset1,
     extended_asset ext_asset2, int initial_fee, name fee_contract){
      return push_action( N(evolutiondex), user, N(inittoken), mvo()
         ( "user", user)
         ("new_symbol", new_symbol)
         ("ext_asset1", ext_asset1)
         ("ext_asset2", ext_asset2) 
         ("initial_fee", initial_fee)
         ("fee_contract", fee_contract)
      );
   }

   action_result addliquidity(name user, asset to_buy, 
     extended_asset max_ext_asset1, extended_asset max_ext_asset2) {
      return push_action( N(evolutiondex), user, N(addliquidity), mvo()
         ( "user", user )
         ( "to_buy", to_buy )
         ( "max_ext_asset1", max_ext_asset1)
         ( "max_ext_asset2", max_ext_asset2)
      );
   }

   action_result remliquidity(name user, asset to_sell,
     extended_asset min_ext_asset1, extended_asset min_ext_asset2) {
      return push_action( N(evolutiondex), user, N(remliquidity), mvo()
         ( "user", user )
         ( "to_sell", to_sell )
         ( "min_ext_asset1", min_ext_asset1)
         ( "min_ext_asset2", min_ext_asset2)
      );
   }

   action_result exchange( name user, symbol through, extended_asset ext_asset1, extended_asset ext_asset2 ) {
      return push_action( N(evolutiondex), user, N(exchange), mvo()
         ( "user", user )
         ( "through", through)
         ( "ext_asset1", ext_asset1 )
         ( "ext_asset2", ext_asset2 )
      );
   }

   action_result changefee( symbol sym, int newfee ) {
      return push_action( N(wesetyourfee), N(alice), N(changefee), mvo()
         ( "sym", sym )
         ( "newfee", newfee )
      );
   }


   vector <int64_t> alice_balance() {
      auto alice_eos_balance = get_balance(N(evolutiondex), N(alice), N(evodexacnts), 0, "evodexaccount" );
      auto alice_voice_balance = get_balance(N(evolutiondex), N(alice), N(evodexacnts), 1, "evodexaccount");   
      auto alice_evo_balance = get_balance(N(evolutiondex), N(alice), N(accounts), 5199429, "account" );

      auto saldo_eos = to_int(fc::json::to_string(alice_eos_balance["balance"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto saldo_voice = to_int( fc::json::to_string(alice_voice_balance["balance"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto saldo_evo = to_int( fc::json::to_string(alice_evo_balance["balance"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      vector <int64_t> ans = {saldo_eos, saldo_voice, saldo_evo};
      return ans;
   }

   int64_t to_int(string in) {
      auto sub = in.substr(1,in.length()-2);
      return asset::from_string(sub).get_amount();
   }

   vector <int64_t> system_balance(){
      auto sys_balance_json = get_balance(N(evolutiondex), name(5199429), N(stat), 5199429, "currency_stats" );   
      auto saldo_eos = to_int(fc::json::to_string(sys_balance_json["connector1"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto saldo_voice = to_int(fc::json::to_string(sys_balance_json["connector2"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto minted_evo = to_int(fc::json::to_string(sys_balance_json["supply"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      vector <int64_t> ans = {saldo_eos, saldo_voice, minted_evo};
      return ans;
   }

   abi_serializer abi_ser;
};

static symbol EVO = symbol::from_string("4,EVO");
static symbol EOS = symbol::from_string("4,EOS");
static symbol VOICE = symbol::from_string("4,VOICE");


BOOST_AUTO_TEST_SUITE(eosio_token_tests)

BOOST_FIXTURE_TEST_CASE( evo_tests, eosio_token_tester ) try {
   const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
   abi_def abi_evo;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);
   const auto& accnt3 = control->db().get<account_object,by_name>( N(wesetyourfee) );
   abi_def abi_fee;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt3.abi, abi_fee), true);

   create( N(alice), asset::from_string("1000000000000.0000 EOS") );
   create( N(bob), asset::from_string("1000000000000.0000 VOICE") );
   issue( N(alice), N(alice), asset::from_string("100000000.0000 EOS"), "some memo");
   issue( N(bob), N(bob), asset::from_string("1000000000.0000 VOICE"), "");
   transfer( N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

   abi_ser.set_abi(abi_evo, abi_serializer_max_time);

   // eos and voice both live in eosio.token
   openext( N(alice), N(alice), extended_symbol{EOS, N(eosio.token)});
   openext( N(alice), N(alice), extended_symbol{VOICE, N(eosio.token)});

   transfer( N(alice), N(evolutiondex), asset::from_string("10000000.0000 EOS"), "");
   transfer( N(alice), N(evolutiondex), asset::from_string("200000000.0000 VOICE"), "");
   
   inittoken( N(alice), EVO,
     extended_asset{asset{10000000000, EOS}, N(eosio.token)},
     extended_asset{asset{1000000000000, VOICE}, N(eosio.token)},
     10, N(wesetyourfee));

   auto alice_evo_balance = get_balance(N(evolutiondex), N(alice), N(accounts), 5199429, "account");
   auto bal = mvo() ("balance", asset{100000000000, EVO});
   BOOST_REQUIRE_EQUAL( fc::json::to_string(alice_evo_balance, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ), 
   fc::json::to_string(bal, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ) );

   cout << alice_balance().at(0) << " " << alice_balance().at(1) << " " << alice_balance().at(2) << endl;

   addliquidity( N(alice), asset::from_string("50.0000 EVO"), extended_asset{asset{100000000000, EOS}, N(eosio.token)}, extended_asset{asset{100000000000, VOICE}, N(eosio.token)});

   produce_blocks();
   cout << alice_balance().at(0) << " " << alice_balance().at(1) << " " << alice_balance().at(2) << endl;

   remliquidity( N(alice), asset::from_string("17.1872 EVO"),
     extended_asset{asset{1, EOS}, N(eosio.token)},
     extended_asset{asset{1, VOICE}, N(eosio.token)});
     
   cout << alice_balance().at(0) << " " << alice_balance().at(1) << " " << alice_balance().at(2) << endl;

   exchange( N(alice), EVO, 
     extended_asset{asset{40000, EOS}, N(eosio.token)},
     extended_asset{asset{-100000, VOICE}, N(eosio.token)});

   cout << alice_balance().at(0) << " " << alice_balance().at(1) << " " << alice_balance().at(2) << endl;

   exchange( N(alice), EVO, 
     extended_asset{asset{1000, EOS}, N(eosio.token)},
     extended_asset{asset{-48500, VOICE}, N(eosio.token)});

   cout << alice_balance().at(0) << " " << alice_balance().at(1) << " " << alice_balance().at(2) << endl;

   exchange( N(alice), EVO, 
     extended_asset{asset{1, EOS}, N(eosio.token)},
     extended_asset{asset{-9, VOICE}, N(eosio.token)});

   cout << alice_balance().at(0) << " " << alice_balance().at(1) << " " << alice_balance().at(2) << endl;
   // mirar stats de evo, chequear que state_parameter siempre suba.

   abi_ser.set_abi(abi_fee, abi_serializer_max_time); 
   changefee( EVO, 50 );

   abi_ser.set_abi(abi_evo, abi_serializer_max_time);
   addliquidity( N(alice), asset::from_string("50.0000 EVO"),
     extended_asset{asset{100000000000, EOS}, N(eosio.token)},
     extended_asset{asset{100000000000, VOICE}, N(eosio.token)});

   cout << alice_balance().at(0) << " " << alice_balance().at(1) << " " << alice_balance().at(2) << endl;

//   abi_ser.set_abi(abi_fee, abi_serializer_max_time);
//   changefee( EVO, 20 );

   cout << system_balance().at(0) << " " << system_balance().at(1) << " " << system_balance().at(2) << endl;

} FC_LOG_AND_RETHROW()



BOOST_FIXTURE_TEST_CASE( evo_tests_asserts, eosio_token_tester ) try {

   create( N(alice), asset::from_string("1000000000000.0000 EOS") );
   create( N(bob), asset::from_string("1000000000000.0000 VOICE") );
   issue( N(alice), N(alice), asset::from_string("100000000.0000 EOS"), "some memo");
   issue( N(bob), N(bob), asset::from_string("1000000000.0000 VOICE"), "");
   transfer( N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

   const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
   abi_def abi2;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi2), true);
   abi_ser.set_abi(abi2, abi_serializer_max_time);

   // open tok y voice, for alice, both live in eosio.token
   BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
     extended_symbol{EOS, N(eosio.token)}) );
   BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
     extended_symbol{VOICE, N(eosio.token)}) );

   BOOST_REQUIRE_EQUAL( success(), transfer( N(alice), N(evolutiondex), 
     asset::from_string("1000.0000 EOS"), "") );
   BOOST_REQUIRE_EQUAL( success(), transfer( N(alice), N(evolutiondex),
     asset::from_string("20000.0000 VOICE"), "") );

   inittoken( N(alice), EVO, extended_asset{asset{1, EOS}, N(eosio.token)},
     extended_asset{asset{1000, VOICE}, N(eosio.token)}, 10, N(wesetyourfee));

   BOOST_REQUIRE_EQUAL( success(), open( N(alice), EVO, N(alice)) );
   BOOST_REQUIRE_EQUAL( wasm_assert_msg("Cannot close because the balance is not zero."), closeext( N(alice), 
     extended_symbol{EOS, N(eosio.token)}) );
   BOOST_REQUIRE_EQUAL( wasm_assert_msg( "User already has this account" ), openext( N(alice), N(alice), 
      extended_symbol{VOICE, N(eosio.token)}) );
   BOOST_REQUIRE_EQUAL( success(), withdraw( N(alice), 
     extended_asset{asset{9999999, EOS}, N(eosio.token)}) );
   BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), withdraw( N(alice), 
     extended_asset{asset{1, EOS}, N(eosio.token)}) );
   BOOST_REQUIRE_EQUAL( success(), closeext( N(alice),
     extended_symbol{EOS, N(eosio.token)}) );
   // testear close, transfer

} FC_LOG_AND_RETHROW()

// testear operaciones con más de un token, chequear que los productos 
// de los conectores van aumentando.

BOOST_AUTO_TEST_SUITE_END()