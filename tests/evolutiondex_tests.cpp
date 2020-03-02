#include <eosio/chain/abi_serializer.hpp>
#include <eosio/testing/tester.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <contracts.hpp>
#include <cmath>

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;
using namespace boost::multiprecision;

using int128 = boost::multiprecision::int128_t;
using int256 = boost::multiprecision::int256_t;
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

   action_result transfer( name contract, account_name from,
                  account_name to,
                  asset        quantity,
                  string       memo ) {
      return push_action( contract, from, N(transfer), mvo()
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
      return push_action( N(evolutiondex), payer, N(openext), mvo()
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

   action_result changefee( name from_contract, symbol sym, int newfee ) {
      return push_action( from_contract, N(alice), N(changefee), mvo()
         ( "sym", sym )
         ( "newfee", newfee )
      );
   }

   int64_t balance(name user, int64_t id) {
      auto _balance = get_balance(N(evolutiondex), user, N(evodexacnts), id, "evodexaccount" );
      return to_int(fc::json::to_string(_balance["balance"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
   }

   int64_t tok_balance(name user, int64_t id){
      auto _balance = get_balance(N(evolutiondex), user, N(accounts), id, "account" );
      return to_int(fc::json::to_string(_balance["balance"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
   }

   int64_t to_int(string in) {
      auto sub = in.substr(1,in.length()-2);
      return asset::from_string(sub).get_amount();
   }

   vector <int64_t> system_balance(int64_t id){
      auto sys_balance_json = get_balance(N(evolutiondex), name(id), N(stat), id, "currency_stats" );   
      auto saldo1 = to_int(fc::json::to_string(sys_balance_json["connector1"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto saldo2 = to_int(fc::json::to_string(sys_balance_json["connector2"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto minted = to_int(fc::json::to_string(sys_balance_json["supply"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      vector <int64_t> ans = {saldo1, saldo2, minted};
      return ans;
   }

   bool is_increasing(vector <int64_t> v, vector <int64_t> w){
      int256 x = int256(v.at(0)) * int256(v.at(1)) * int256(w.at(2)) * int256(w.at(2));
      int256 y = int256(w.at(0)) * int256(w.at(1)) * int256(v.at(2)) * int256(v.at(2));
      // cout << "x " << x << endl << "y " << y << endl;
      return x < y;
   }

   vector <int64_t> total(){
      int64_t total_eos =  balance(N(alice), 0) + balance(N(bob), 0) 
        + system_balance(5199429).at(0) + system_balance(293455877189).at(0);
      int64_t total_voice = balance(N(alice), 1) + balance(N(bob), 1) 
        + system_balance(5199429).at(1);
      int64_t total_tusd = balance(N(alice), 2) + balance(N(bob), 2)
        + system_balance(293455877189).at(1);
      vector <int64_t> ans = {total_eos, total_voice, total_tusd};
      return ans;
   }

   abi_serializer abi_ser;
};

static symbol EVO = symbol::from_string("4,EVO");
static symbol ETUSD = symbol::from_string("4,ETUSD");
static symbol EOS = symbol::from_string("4,EOS");
static symbol VOICE = symbol::from_string("4,VOICE");
static symbol TUSD = symbol::from_string("4,TUSD");

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
    transfer( N(eosio.token), N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    // eos and voice both live in eosio.token
    openext( N(alice), N(alice), extended_symbol{EOS, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{VOICE, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{TUSD, N(eosio.token)});

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("10000000.0000 EOS"), "");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("200000000.0000 VOICE"), "");
    
    inittoken( N(alice), EVO,
      extended_asset{asset{10000000000, EOS}, N(eosio.token)},
      extended_asset{asset{1000000000000, VOICE}, N(eosio.token)},
      10, N(wesetyourfee));

    auto alice_evo_balance = get_balance(N(evolutiondex), N(alice), N(accounts), 5199429, "account");
    auto bal = mvo() ("balance", asset{100000000000, EVO});
    BOOST_REQUIRE_EQUAL( fc::json::to_string(alice_evo_balance, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ), 
    fc::json::to_string(bal, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ) );

    addliquidity( N(alice), asset::from_string("50.0000 EVO"), extended_asset{asset{100000000000, EOS}, N(eosio.token)}, extended_asset{asset{100000000000, VOICE}, N(eosio.token)});

    produce_blocks();

    remliquidity( N(alice), asset::from_string("17.1872 EVO"),
      extended_asset{asset{1, EOS}, N(eosio.token)},
      extended_asset{asset{1, VOICE}, N(eosio.token)});
      
    exchange( N(alice), EVO, 
      extended_asset{asset{40000, EOS}, N(eosio.token)},
      extended_asset{asset{-100000, VOICE}, N(eosio.token)});

    exchange( N(alice), EVO, 
      extended_asset{asset{1000, EOS}, N(eosio.token)},
      extended_asset{asset{-48500, VOICE}, N(eosio.token)});

    exchange( N(alice), EVO, 
      extended_asset{asset{1, EOS}, N(eosio.token)},
      extended_asset{asset{-9, VOICE}, N(eosio.token)});

    auto new_vec = system_balance(5199429);
    vector <int64_t> final_system_balance = {10000073864, 999999190299, 100000328128};
    BOOST_REQUIRE_EQUAL(final_system_balance == new_vec, true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0), 89999926136);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1), 1000000809701);

    abi_ser.set_abi(abi_fee, abi_serializer_max_time); 
    BOOST_REQUIRE_EQUAL( success(), changefee( N(wesetyourfee), EVO, 50) );
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    addliquidity( N(alice), asset::from_string("50.0000 EVO"),
      extended_asset{asset{100000000000, EOS}, N(eosio.token)},
      extended_asset{asset{100000000000, VOICE}, N(eosio.token)});

    new_vec = system_balance(5199429);
    final_system_balance = {10000124116, 1000004215279, 100000828128};
    BOOST_REQUIRE_EQUAL(final_system_balance == new_vec, true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0), 89999875884);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1), 999995784721);

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( increasing_parameter, eosio_token_tester) try {
    cout << endl << "increasing_parameter test begins" << endl;
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);
    const auto& accnt3 = control->db().get<account_object,by_name>( N(wesetyourfee) );
    abi_def abi_fee;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt3.abi, abi_fee), true);

    create( N(alice), asset{4611686018427387903, EOS});
    create( N(bob), asset{4611686018427387903, VOICE} );
    create( N(alice), asset{4611686018427387903, TUSD});
    issue( N(alice), N(alice), asset{4611686018427387903, EOS}, "");
    issue( N(bob), N(bob), asset{4611686018427387903, VOICE}, "");
    issue( N(alice), N(alice), asset{4611686018427387903, TUSD}, "");

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    // eos and voice both live in eosio.token
    openext( N(alice), N(alice), extended_symbol{EOS, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{VOICE, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{TUSD, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{EOS, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{VOICE, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{TUSD, N(eosio.token)});

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset{4611686018427387903, EOS}, "");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset{4611686018427387000, VOICE}, "deposit to: alice");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset{903, VOICE}, "this goes to Bob");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset{4500000000000000000, TUSD}, "");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset{30000000000000000, TUSD}, "deposit to: bob");

    inittoken( N(alice), EVO,
      extended_asset{asset{230584300921369, EOS}, N(eosio.token)},
      extended_asset{asset{961168601842738, VOICE}, N(eosio.token)},
      10, N(wesetyourfee));

    inittoken( N(alice), ETUSD,
      extended_asset{asset{100000000000000, EOS}, N(eosio.token)},
      extended_asset{asset{991168601842738, TUSD}, N(eosio.token)},
      10, N(wesetyourfee));

    cout << "Alice: " << balance(N(alice), 0) << " " << balance(N(alice), 1) << " " << balance(N(alice), 2) << endl;

    auto old_total = total();
    auto old_vec = system_balance(5199429);
    auto old_alice_bal_0 = balance(N(alice),0);
    auto old_alice_bal_1 = balance(N(alice),1);
    cout << old_vec.at(0) << " " << old_vec.at(1) << " " << old_vec.at(2) << endl << endl;

    exchange( N(alice), EVO, 
      extended_asset{asset{40000, EOS}, N(eosio.token)},
      extended_asset{asset{-10000, VOICE}, N(eosio.token)});
    auto new_total = total();
    auto new_vec = system_balance(5199429);
    auto new_alice_bal_0 = balance(N(alice),0);
    auto new_alice_bal_1 = balance(N(alice),1);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    BOOST_REQUIRE_EQUAL(new_alice_bal_0 - old_alice_bal_0, -40000);
    BOOST_REQUIRE_EQUAL(new_alice_bal_1 - old_alice_bal_1, 166569);

    cout << "Alice: " << balance(N(alice), 0) << " " << balance(N(alice), 1) << " " << balance(N(alice), 2) << endl;
    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl << endl;
    
    old_total = total();
    old_vec = system_balance(5199429);
    old_alice_bal_0 = balance(N(alice), 0);
    old_alice_bal_1 = balance(N(alice), 1);
    addliquidity( N(alice), asset::from_string("0.0001 EVO"), extended_asset{asset{100000000000, EOS}, N(eosio.token)}, extended_asset{asset{100000000000, VOICE}, N(eosio.token)});
    new_total = total();
    new_vec = system_balance(5199429);
    new_alice_bal_0 = balance(N(alice),0);
    new_alice_bal_1 = balance(N(alice),1);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    BOOST_REQUIRE_EQUAL(new_alice_bal_0 - old_alice_bal_0, -2);
    BOOST_REQUIRE_EQUAL(new_alice_bal_1 - old_alice_bal_1, -4);

    cout << "Alice: " << balance(N(alice), 0) << " " << balance(N(alice), 1) << " " << balance(N(alice), 2) << endl;
    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl << endl;
    
    produce_blocks();

    old_total = total();
    old_vec = system_balance(5199429);
    old_alice_bal_0 = balance(N(alice), 0);
    old_alice_bal_1 = balance(N(alice), 1);
    remliquidity( N(alice), asset::from_string("0.0001 EVO"),
      extended_asset{asset{0, EOS}, N(eosio.token)},
      extended_asset{asset{0, VOICE}, N(eosio.token)});
    new_total = total();
    new_vec = system_balance(5199429);
    new_alice_bal_0 = balance(N(alice), 0);
    new_alice_bal_1 = balance(N(alice), 1);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    BOOST_REQUIRE_EQUAL(new_alice_bal_0 - old_alice_bal_0, 0);
    BOOST_REQUIRE_EQUAL(new_alice_bal_1 - old_alice_bal_1, 2);

    cout << "Alice: " << balance(N(alice), 0) << " " << balance(N(alice), 1) << " " << balance(N(alice), 2) << endl;
    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl << endl;

    cout << "Bob: " << balance(N(bob), 0) << " " << balance(N(bob), 1) << " " << balance(N(bob), 2) << endl;

    old_total = total();
    old_vec = system_balance(293455877189);
    exchange( N(bob), ETUSD, 
      extended_asset{asset{-40000000000000, EOS}, N(eosio.token)},
      extended_asset{asset{3000000000000000000, TUSD}, N(eosio.token)});
    new_total = total();
    new_vec = system_balance(293455877189);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    cout << "Bob: " << balance(N(bob), 0) << " " << balance(N(bob), 1) << " " << balance(N(bob), 2) << endl;
    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl << endl;

    old_total = total();    
    old_vec = system_balance(5199429);
    exchange( N(bob), EVO, 
      extended_asset{asset{40000000, EOS}, N(eosio.token)},
      extended_asset{asset{-10000, VOICE}, N(eosio.token)});
    new_total = total();
    new_vec = system_balance(5199429);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    cout << "Bob: " << balance(N(bob), 0) << " " << balance(N(bob), 1) << " " << balance(N(bob), 2) << endl;
    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl << endl;

    withdraw( N(bob), extended_asset{asset{1, EOS}, N(eosio.token)});
    BOOST_REQUIRE_EQUAL(total() == new_total, false);

    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl;
    cout << "Alice: " << balance(N(alice), 0) << " " << balance(N(alice), 1) << " " << balance(N(alice), 2) << endl;

    old_total = total();
    old_vec = system_balance(5199429);
    addliquidity( N(alice), asset{1500000000000000000, EVO}, 
       extended_asset{asset{4000000000000000000, EOS}, N(eosio.token)}, extended_asset{asset{4000000000000000000, VOICE}, N(eosio.token)});
    new_total = total();
    new_vec = system_balance(5199429);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    cout << "alice: " << balance(N(alice), 0) << " " << balance(N(alice), 1) << " " << balance(N(alice), 2) << endl;
    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl << endl;

    old_total = total(); 
    old_vec = system_balance(5199429);
    exchange( N(alice), EVO, 
      extended_asset{asset{3875926873243173478, EOS}, N(eosio.token)},
      extended_asset{asset{-1, VOICE}, N(eosio.token)});
    new_total = total();
    new_vec = system_balance(5199429);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    cout << "alice: " << balance(N(alice), 0) << " " << balance(N(alice), 1) << " " << balance(N(alice), 2) << endl;
    cout << new_vec.at(0) << " " << new_vec.at(1) << " " << new_vec.at(2) << endl;
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( evo_tests_asserts, eosio_token_tester ) try {

    create( N(alice), asset::from_string("1000000000000.0000 EOS") );
    create( N(bob), asset::from_string("1000000000000.0000 VOICE") );
    issue( N(alice), N(alice), asset::from_string("100000000.0000 EOS"), "some memo");
    issue( N(bob), N(bob), asset::from_string("1000000000.0000 VOICE"), "");
    transfer( N(eosio.token), N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);
    const auto& accnt3 = control->db().get<account_object,by_name>( N(wesetyourfee) );
    abi_def abi_fee;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt3.abi, abi_fee), true);

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("Extended_symbol not registered for this user, please run openext action"),
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset{1000,EOS}, "") );

    // OPENEXT
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{EOS, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{VOICE, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "user account does not exist"), 
      openext( N(cat), N(alice), extended_symbol{VOICE, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("User already has this account"), 
      openext( N(alice), N(alice), extended_symbol{VOICE, N(eosio.token)}) );


    BOOST_REQUIRE_EQUAL( success(), transfer( N(eosio.token), N(alice), N(evolutiondex), 
      asset::from_string("1000.0000 EOS"), "") );
    BOOST_REQUIRE_EQUAL( success(), transfer( N(eosio.token), N(alice), N(evolutiondex),
      asset::from_string("20000.0000 VOICE"), "") );

    // INITTOKEN
    BOOST_REQUIRE_EQUAL( success(), inittoken( N(alice), EVO, extended_asset{asset{1, EOS}, N(eosio.token)},
      extended_asset{asset{1000, VOICE}, N(eosio.token)}, 10, N(another)) );
    // Agregar chequeos

    // si quiero hacer exchange excediendo los conectores salta "invalid parameters" (compute)

    // CLOSEEXT
    BOOST_REQUIRE_EQUAL( success(), open( N(alice), EVO, N(alice)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("User does not have such token"), closeext( N(alice), 
      extended_symbol{TUSD, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "User already has this account" ), openext( N(alice), N(alice), 
        extended_symbol{VOICE, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), closeext( N(alice), extended_symbol{VOICE, N(eosio.token)}) );
    // habría que testear que le devolvió el token al cerrar.
    BOOST_REQUIRE_EQUAL( success(), withdraw( N(alice), 
      extended_asset{asset{9999999, EOS}, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), withdraw( N(alice), 
      extended_asset{asset{1, EOS}, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), closeext( N(alice),
      extended_symbol{EOS, N(eosio.token)}) );
    // testear todos los checks. Las funciones de token acaso no hace falta testearlas.

    abi_ser.set_abi(abi_fee, abi_serializer_max_time); 
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("contract not authorized to change fee."), 
      changefee( N(wesetyourfee), EVO, 50) );
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
} FC_LOG_AND_RETHROW()


BOOST_AUTO_TEST_SUITE_END()