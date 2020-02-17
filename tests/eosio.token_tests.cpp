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

      create_accounts( { N(alice), N(bob), N(carol), N(eosio.token), N(evolutiondex) } );
      produce_blocks( 2 );

      set_code( N(eosio.token), contracts::eosio_token_wasm() );
      set_abi( N(eosio.token), contracts::eosio_token_abi().data() );

      set_code( N(evolutiondex), contracts::evolutiondex_wasm() );
      set_abi( N(evolutiondex), contracts::evolutiondex_abi().data() );

      produce_blocks();
   }

/* int128_t make128key(uint64_t a, uint64_t b) {
    uint128_t aa = a;
    uint128_t bb = b;
    return (aa << 64) + bb;
} */

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

   action_result open( name user, name payer, name smartctr, symbol sym ) {
      return push_action( N(evolutiondex), user, N(open), mvo()
           ( "user", user)
           ( "payer", payer)
           ( "contract", smartctr)
           ( "sym", sym)
      );
   }

   action_result close ( const name user, const name smartctr, const symbol sym ){ 
      return push_action( N(evolutiondex), user, N(close), mvo()
           ( "user", user)
           ( "smartctr", smartctr)
           ( "sym", sym)      
      );
   }

   action_result withdraw(name user, name smartctr, asset to_withdraw) {
      return push_action( N(evolutiondex), user, N(withdraw), mvo()
         ( "user", user )
         ( "smartctr", smartctr )
         ( "to_withdraw", to_withdraw )
      );
   }

   action_result inittoken( name user, name smartctr1, asset asset1, 
     name smartctr2, asset asset2, symbol new_symbol, int initial_fee, name fee_contract){
      return push_action( N(evolutiondex), user, N(inittoken), mvo()
         ( "user", user)
         ("smartctr1", smartctr1)
         ("asset1", asset1)
         ("smartctr2", smartctr2)
         ("asset2", asset2)
         ("new_symbol", new_symbol)
         ("initial_fee", initial_fee)
         ("fee_contract", fee_contract)
      );
   }

   action_result addliquidity(name user, asset to_buy) {
      return push_action( N(evolutiondex), user, N(addliquidity), mvo()
         ( "user", user )
         ( "to_buy", to_buy )
      );
   }

   action_result remliquidity(name user, asset to_sell) {
      return push_action( N(evolutiondex), user, N(remliquidity), mvo()
         ( "user", user )
         ( "to_sell", to_sell )
      );
   }

   action_result exchange( name user, symbol through, asset asset1, asset asset2 ) {
      return push_action( N(evolutiondex), user, N(exchange), mvo()
         ( "user", user )
         ( "through", through)
         ( "asset1", asset1 )
         ( "asset2", asset2 )
      );
   }

/*   const long EVONUM = 6262569080031172048;
   const long EOSNUM = 6138663591592764928;
   auto symb_evo = eosio::chain::symbol::from_string("4,EVO");
   auto symbol_code_evo = symb_evo.to_symbol_code().value;
*/
   void alice_balance(int i) {
      auto alice_eos_balance = get_balance(N(evolutiondex), N(alice), N(evodexacnts), 0, "evodexaccount" );
      auto alice_voice_balance = get_balance(N(evolutiondex), N(alice), N(evodexacnts), 1, "evodexaccount");   
      auto alice_evo_balance = get_balance(N(evolutiondex), N(alice), N(evodexacnts), 2, "evodexaccount" );

      cout << i << " alice: " << alice_eos_balance;
      cout << i << " alice: " << alice_voice_balance;
      cout << i << " alice: " << alice_evo_balance;
   }

   abi_serializer abi_ser;
};


BOOST_AUTO_TEST_SUITE(eosio_token_tests)

BOOST_FIXTURE_TEST_CASE( evo_tests, eosio_token_tester ) try {
   const auto& accnt1 = control->db().get<account_object,by_name>( N(eosio.token) );
   abi_def abi1;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt1.abi, abi1), true);
   abi_ser.set_abi(abi1, abi_serializer_max_time);

   create( N(alice), asset::from_string("1000000000000.0000 EOS") );
   create( N(bob), asset::from_string("1000000000000.0000 VOICE") );
   issue( N(alice), N(alice), asset::from_string("100000000.0000 EOS"), "some memo");
   issue( N(bob), N(bob), asset::from_string("1000000000.0000 VOICE"), "");
   transfer( N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");
   
   auto symb = eosio::chain::symbol::from_string("4,EOS");
   auto symbol_code = symb.to_symbol_code().value;

   const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
   abi_def abi2;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi2), true);
   abi_ser.set_abi(abi2, abi_serializer_max_time);

   // eos and voice both live in eosio.token
   open( N(alice), N(alice), N(eosio.token), symbol::from_string("4,EOS"));
   open( N(alice), N(alice), N(eosio.token), symbol::from_string("4,VOICE"));
   open( N(alice), N(alice), N(evolutiondex), symbol::from_string("4,EVO"));

   transfer( N(alice), N(evolutiondex), asset::from_string("10000000.0000 EOS"), "");
   transfer( N(alice), N(evolutiondex), asset::from_string("200000000.0000 VOICE"), "");
   
   inittoken( N(alice), 
     N(eosio.token), asset::from_string("1000000.0000 EOS"),
     N(eosio.token), asset::from_string("100000000.0000 VOICE"), 
     symbol::from_string("4,EVO"), 10, N(wesetyourfee));

   auto alice_evo_balance = get_balance(N(evolutiondex), N(alice), N(evodexacnts), 2, "evodexaccount");
   auto bal = mvo()
      ("balance", extended_asset{asset{100000000000, symbol::from_string("4,EVO")}, N(evolutiondex)})
      ("id", 2);
   BOOST_REQUIRE_EQUAL( fc::json::to_string(alice_evo_balance, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ), 
fc::json::to_string(bal, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ) );

   alice_balance(0);

   addliquidity( N(alice), asset::from_string("50.0000 EVO") );

   produce_blocks();
   alice_balance(1);

   remliquidity( N(alice), asset::from_string("17.1872 EVO") ) ;
   alice_balance(2);


   exchange( N(alice), symbol::from_string("4,EVO"), 
     asset::from_string("4.0000 EOS"), asset::from_string("-10.0000 VOICE") );
   alice_balance(3);

   exchange( N(alice), symbol::from_string("4,EVO"), 
     asset::from_string("0.1000 EOS"), asset::from_string("-4.8500 VOICE") );
   alice_balance(4);

   exchange( N(alice), symbol::from_string("4,EVO"), 
     asset::from_string("0.0001 EOS"), asset::from_string("-0.0009 VOICE") );
   alice_balance(5);
   // mirar stats de evo, chequear que state_parameter siempre suba.

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( evo_tests_asserts, eosio_token_tester ) try {
   const auto& accnt1 = control->db().get<account_object,by_name>( N(eosio.token) );
   abi_def abi1;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt1.abi, abi1), true);
   abi_ser.set_abi(abi1, abi_serializer_max_time);

   BOOST_REQUIRE_EQUAL( success(), create( N(alice), asset::from_string("1000000000000.0000 EOS") ) );
   BOOST_REQUIRE_EQUAL( success(), create( N(bob), asset::from_string("1000000000000.0000 VOICE") ) );
   BOOST_REQUIRE_EQUAL( success(), issue( N(alice), N(alice), asset::from_string("10000.0000 EOS"), "some memo") );
   BOOST_REQUIRE_EQUAL( success(), issue( N(bob), N(bob), asset::from_string("100000.0000 VOICE"), "") );
   BOOST_REQUIRE_EQUAL( success(), transfer( N(bob), N(alice), asset::from_string("50000.0000 VOICE"), "") );
   
   auto symb = eosio::chain::symbol::from_string("4,EOS");
   auto symbol_code = symb.to_symbol_code().value;

   const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
   abi_def abi2;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi2), true);
   abi_ser.set_abi(abi2, abi_serializer_max_time);

   // open tok y voice, for alice, both live in eosio.token
   BOOST_REQUIRE_EQUAL( success(), open( N(alice), N(alice), N(eosio.token), symbol::from_string("4,EOS")));
   BOOST_REQUIRE_EQUAL( success(), open( N(alice), N(alice), N(eosio.token), symbol::from_string("4,VOICE")));

   BOOST_REQUIRE_EQUAL( success(), transfer( N(alice), N(evolutiondex), 
     asset::from_string("1000.0000 EOS"), "") );
   BOOST_REQUIRE_EQUAL( success(), transfer( N(alice), N(evolutiondex),
     asset::from_string("20000.0000 VOICE"), "") );

   BOOST_REQUIRE_EQUAL( success(), open( N(alice), N(alice), N(evolutiondex), symbol::from_string("4,EVO")));

   BOOST_REQUIRE_EQUAL( wasm_assert_msg("Cannot close because the balance is not zero."), close( N(alice), N(eosio.token), 
     symbol::from_string("4,EOS")) );

   BOOST_REQUIRE_EQUAL( wasm_assert_msg( "User already has this account" ),
      open( N(alice), N(alice), N(eosio.token), symbol::from_string("4,VOICE"))
   );
} FC_LOG_AND_RETHROW()


BOOST_AUTO_TEST_SUITE_END()


/*   BOOST_REQUIRE_EQUAL( success(), withdraw( N(alice), N(eosio.token),
     asset::from_string("989.0000 EOS")) );

   BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), withdraw( N(alice), N(eosio.token),
     asset::from_string("0.0001 EOS")) );*

   BOOST_REQUIRE_EQUAL( success(), close( N(alice), N(eosio.token), 
     symbol::from_string("4,EOS")) ); */
