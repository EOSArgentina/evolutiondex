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

class evolutiondex_tester : public tester {
public:

   evolutiondex_tester() {
      produce_blocks( 2 );

      create_accounts( { N(alice), N(bob), N(carol), N(eosio.token), N(evolutiondex),
        N(wevotethefee) } );
      produce_blocks( 2 );

      set_code( N(eosio.token), contracts::token_wasm() );
      set_abi( N(eosio.token), contracts::token_abi().data() );

      set_code( N(evolutiondex), contracts::evolutiondex_wasm() );
      set_abi( N(evolutiondex), contracts::evolutiondex_abi().data() );

      set_code( N(carol), contracts::token_wasm() );
      set_abi( N(carol), contracts::token_abi().data() );

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
   action_result create( account_name issuer, asset maximum_supply ) {
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
   action_result closeext ( const name user, const name to, const extended_symbol ext_symbol ){ 
      return push_action( N(evolutiondex), user, N(closeext), mvo()
           ( "user", user )
           ( "to", to )
           ( "ext_symbol", ext_symbol )
           ( "memo", "" )
      );
   }
   action_result withdraw(name user, name to, extended_asset to_withdraw) {
      return push_action( N(evolutiondex), user, N(withdraw), mvo()
         ( "user", user )
         ( "to", to )
         ( "to_withdraw", to_withdraw )
         ( "memo", "" )
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

   action_result addliquidity(name user, asset to_buy, asset max_asset1, asset max_asset2) {
      return push_action( N(evolutiondex), user, N(addliquidity), mvo()
         ( "user", user )
         ( "to_buy", to_buy )
         ( "max_asset1", max_asset1)
         ( "max_asset2", max_asset2)
      );
   }
   action_result remliquidity(name user, asset to_sell,
     asset min_asset1, asset min_asset2) {
      return push_action( N(evolutiondex), user, N(remliquidity), mvo()
         ( "user", user )
         ( "to_sell", to_sell )
         ( "min_asset1", min_asset1)
         ( "min_asset2", min_asset2)
      );
   }
   action_result exchange( name user, symbol_code pair_token, extended_asset ext_asset_in, asset min_expected ) {
      return push_action( N(evolutiondex), user, N(exchange), mvo()
         ( "user", user )
         ( "through", pair_token )
         ( "ext_asset_in", ext_asset_in )
         ( "min_expected", min_expected )
      );
   }
   action_result changefee( symbol_code pair_token, int newfee ) {
      return push_action( N(evolutiondex), N(wevotethefee), N(changefee), mvo()
         ( "pair_token", pair_token )
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
   int64_t token_balance(name contract, name user, int64_t id){
      auto _balance = get_balance(contract, user, N(accounts), id, "account" );
      return to_int(fc::json::to_string(_balance["balance"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
   }
   int64_t to_int(string in) {
      auto sub = in.substr(1,in.length()-2);
      return asset::from_string(sub).get_amount();
   }

   vector <int64_t> system_balance(int64_t id){
      auto sys_balance_json = get_balance(N(evolutiondex), name(id), N(stat), id, "currency_stats" );   
      auto saldo1 = to_int(fc::json::to_string(sys_balance_json["pool1"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto saldo2 = to_int(fc::json::to_string(sys_balance_json["pool2"]["quantity"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      auto minted = to_int(fc::json::to_string(sys_balance_json["supply"], 
        fc::time_point(fc::time_point::now() + abi_serializer_max_time) ));
      vector <int64_t> ans = {saldo1, saldo2, minted};
      return ans;
   }

   bool is_increasing(vector <int64_t> v, vector <int64_t> w){
      int256 x = int256(v.at(0)) * int256(v.at(1)) * int256(w.at(2)) * int256(w.at(2));
      int256 y = int256(w.at(0)) * int256(w.at(1)) * int256(v.at(2)) * int256(v.at(2));
      return x <= y;
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

static symbol EVO4 = symbol::from_string("4,EVO");
static symbol ETUSD4 = symbol::from_string("4,ETUSD");
static symbol EOS4 = symbol::from_string("4,EOS");
static symbol VOICE4 = symbol::from_string("4,VOICE");
static symbol TUSD4 = symbol::from_string("4,TUSD");

static symbol_code EVO = EVO4.to_symbol_code();
static symbol_code ETUSD = ETUSD4.to_symbol_code();
static symbol_code EOS = EOS4.to_symbol_code();
static symbol_code VOICE = VOICE4.to_symbol_code();
static symbol_code TUSD = TUSD4.to_symbol_code();

extended_asset extend(asset to_extend) {
  return extended_asset{to_extend, N(eosio.token)};
}

BOOST_AUTO_TEST_SUITE(evolutiondex_tests)

BOOST_FIXTURE_TEST_CASE( evo_tests, evolutiondex_tester ) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create( N(alice), asset::from_string("1000000000000.0000 EOS") );
    create( N(bob), asset::from_string("1000000000000.0000 VOICE") );
    issue( N(alice), N(alice), asset::from_string("100000000.0000 EOS"), "some memo");
    issue( N(bob), N(bob), asset::from_string("1000000000.0000 VOICE"), "");
    transfer( N(eosio.token), N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    // eos and voice both live in eosio.token
    openext( N(alice), N(alice), extended_symbol{EOS4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{VOICE4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{TUSD4, N(eosio.token)});

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("10000000.0000 EOS"), "");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("200000000.0000 VOICE"), "");
    
    inittoken( N(alice), EVO4,
      extended_asset{asset::from_string("1000000.0000 EOS"), N(eosio.token)},
      extended_asset{asset::from_string("100000000.0000 VOICE"), N(eosio.token)},
      10, N(wevotethefee));

    auto alice_evo_balance = get_balance(N(evolutiondex), N(alice), N(accounts), EVO.value, "account");
    auto bal = mvo() ("balance", asset::from_string("10000000.0000 EVO"));
    BOOST_REQUIRE_EQUAL( fc::json::to_string(alice_evo_balance, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ), 
    fc::json::to_string(bal, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ) );

    addliquidity( N(alice), asset::from_string("50.0000 EVO"), 
      asset::from_string("10000000.0000 EOS"), asset::from_string("10000000.0000 VOICE") );

    produce_blocks();

    remliquidity( N(alice), asset::from_string("17.1872 EVO"),
      asset::from_string("0.0001 EOS"), asset::from_string("0.0001 VOICE") );
      
    exchange( N(alice), EVO, extend(asset::from_string("4.0000 EOS")),asset::from_string("10.0000 VOICE"));
    exchange( N(alice), EVO, extend(asset::from_string("0.1000 EOS")), asset::from_string("4.8500 VOICE"));
    exchange( N(alice), EVO, extend(asset::from_string("0.0001 EOS")), asset::from_string("0.0009 VOICE"));

    auto new_vec = system_balance(EVO.value);
    vector <int64_t> final_system_balance = {10000073864, 999999190299, 100000328128};
    BOOST_REQUIRE_EQUAL(final_system_balance == new_vec, true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0), 89999926136);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1), 1000000809701);

    BOOST_REQUIRE_EQUAL( success(), changefee(EVO, 50) );

    addliquidity( N(alice), asset::from_string("50.0000 EVO"),
      asset::from_string("10000000.0000 EOS"), asset::from_string("10000000.0000 VOICE") );

    new_vec = system_balance(EVO.value);
    final_system_balance = {10000124116, 1000004215279, 100000828128};
    BOOST_REQUIRE_EQUAL(final_system_balance == new_vec, true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0), 89999875884);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1), 999995784721);

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( increasing_parameter, evolutiondex_tester) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create( N(alice), asset::from_string("461168601842738.7903 EOS") );
    create( N(bob), asset::from_string("461168601842738.7903 VOICE") );
    create( N(alice), asset::from_string("461168601842738.7903 TUSD") );
    issue( N(alice), N(alice), asset::from_string("461168601842738.7903 EOS"), "");
    issue( N(bob), N(bob), asset::from_string("461168601842738.7903 VOICE"), "");
    issue( N(alice), N(alice), asset::from_string("461168601842738.7903 TUSD"), "");

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    // eos and voice both live in eosio.token
    openext( N(alice), N(alice), extended_symbol{EOS4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{VOICE4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{TUSD4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{EOS4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{VOICE4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{TUSD4, N(eosio.token)});

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("461168601842738.7903 EOS"), "");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset::from_string("461168601842738.7000 VOICE"), "deposit to: alice");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset::from_string("0.0903 VOICE"), "this goes to Bob");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("450000000000000.0000 TUSD"), "");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("3000000000000.0000 TUSD"), "deposit to: bob");

    inittoken( N(alice), EVO4,
      extended_asset{asset::from_string("23058430092.1369 EOS"), N(eosio.token)},
      extended_asset{asset::from_string("96116860184.2738 VOICE"), N(eosio.token)},
      10, N(wevotethefee));

    inittoken( N(alice), ETUSD4, 
      extended_asset{asset::from_string("10000000000.0000 EOS"), N(eosio.token)},
      extended_asset{asset::from_string("99116860184.2738 TUSD"), N(eosio.token)},
      10, N(wevotethefee));

    auto old_total = total();
    auto old_vec = system_balance(EVO.value);
    auto old_alice_bal_0 = balance(N(alice),0);
    auto old_alice_bal_1 = balance(N(alice),1);

    BOOST_REQUIRE_EQUAL(success(), 
      exchange( N(alice), EVO, extend(asset::from_string("4.0000 EOS")),
      asset::from_string("1.0000 VOICE") ));
    auto new_total = total();
    auto new_vec = system_balance(EVO.value);
    auto new_alice_bal_0 = balance(N(alice),0);
    auto new_alice_bal_1 = balance(N(alice),1);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    BOOST_REQUIRE_EQUAL(new_alice_bal_0 - old_alice_bal_0, -40000);
    BOOST_REQUIRE_EQUAL(new_alice_bal_1 - old_alice_bal_1, 166569);

    old_total = total();
    old_vec = system_balance(EVO.value);
    old_alice_bal_0 = balance(N(alice), 0);
    old_alice_bal_1 = balance(N(alice), 1);
    addliquidity( N(alice), asset::from_string("0.0001 EVO"), 
      asset::from_string("10000000.0000 EOS"), asset::from_string("10000000.0000 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    new_alice_bal_0 = balance(N(alice),0);
    new_alice_bal_1 = balance(N(alice),1);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    BOOST_REQUIRE_EQUAL(new_alice_bal_0 - old_alice_bal_0, -2);
    BOOST_REQUIRE_EQUAL(new_alice_bal_1 - old_alice_bal_1, -4);

    produce_blocks();

    old_total = total();
    old_vec = system_balance(EVO.value);
    old_alice_bal_0 = balance(N(alice), 0);
    old_alice_bal_1 = balance(N(alice), 1);
    remliquidity( N(alice), asset::from_string("0.0001 EVO"),
      asset::from_string("0.0000 EOS"), asset::from_string("0.0000 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    new_alice_bal_0 = balance(N(alice), 0);
    new_alice_bal_1 = balance(N(alice), 1);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    BOOST_REQUIRE_EQUAL(new_alice_bal_0 - old_alice_bal_0, 0);
    BOOST_REQUIRE_EQUAL(new_alice_bal_1 - old_alice_bal_1, 2);

    old_total = total();
    old_vec = system_balance(ETUSD.value);
    BOOST_REQUIRE_EQUAL(success(), 
      exchange( N(bob), ETUSD, 
      extend(asset::from_string("30000000000.0000 TUSD")),
      asset::from_string("40000000.0000 EOS") ) );
    new_total = total();
    new_vec = system_balance(ETUSD.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    old_total = total();    
    old_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(success(), 
      exchange( N(bob), EVO, extend(asset::from_string("4000.0000 EOS")), 
      asset::from_string("1.0000 VOICE") ) );
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    BOOST_REQUIRE_EQUAL( success(), withdraw( N(bob), N(bob), 
      extend(asset::from_string("0.0001 EOS"))) );
    BOOST_REQUIRE_EQUAL(total() == new_total, false);

    old_total = total();
    old_vec = system_balance(EVO.value);
    addliquidity( N(alice), asset::from_string("150000000000000.0000 EVO"), 
       asset::from_string("400000000000000.0000 EOS"), asset::from_string("400000000000000.0000 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    old_total = total(); 
    old_vec = system_balance(EVO.value);
    exchange( N(alice), EVO, extend(asset::from_string("387592687324317.3478 EOS")), 
      asset::from_string("0.0001 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( increasing_parameter_zero_fee, evolutiondex_tester ) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create( N(alice), asset::from_string("461168601842738.7903 EOS") );
    create( N(bob), asset::from_string("461168601842738.7903 VOICE") );
    create( N(alice), asset::from_string("461168601842738.7903 TUSD") );
    issue( N(alice), N(alice), asset::from_string("461168601842738.7903 EOS"), "");
    issue( N(bob), N(bob), asset::from_string("461168601842738.7903 VOICE"), "");
    issue( N(alice), N(alice), asset::from_string("461168601842738.7903 TUSD"), "");

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    // eos and voice both live in eosio.token
    openext( N(alice), N(alice), extended_symbol{EOS4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{VOICE4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{TUSD4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{EOS4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{VOICE4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{TUSD4, N(eosio.token)});

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("461168601842738.7903 EOS"), "");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset::from_string("461168601842738.7000 VOICE"), "deposit to: alice");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset::from_string("0.0903 VOICE"), "this goes to Bob");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("450000000000000.0000 TUSD"), "");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("3000000000000.0000 TUSD"), "deposit to: bob");

    inittoken( N(alice), EVO4,
      extended_asset{asset::from_string("23058430092.1369 EOS"), N(eosio.token)},
      extended_asset{asset::from_string("96116860184.2738 VOICE"), N(eosio.token)},
      0, N(wevotethefee));

    inittoken( N(alice), ETUSD4,
      extended_asset{asset::from_string("10000000000.0000 EOS"), N(eosio.token)},
      extended_asset{asset::from_string("99116860184.2738 TUSD"), N(eosio.token)},
      0, N(wevotethefee));

    auto old_total = total();
    auto old_vec = system_balance(EVO.value);

    exchange( N(alice), EVO, extend(asset::from_string("4.0000 EOS")), 
      asset::from_string("1.0000 VOICE") );
    auto new_total = total();
    auto new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    
    old_total = total();
    old_vec = system_balance(EVO.value);
    addliquidity( N(alice), asset::from_string("0.0001 EVO"), 
      asset::from_string("10000000.0000 EOS"), asset::from_string("10000000.0000 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
    
    produce_blocks();

    old_total = total();
    old_vec = system_balance(EVO.value);
    remliquidity( N(alice), asset::from_string("0.0001 EVO"), 
      asset::from_string("0.0000 EOS"), asset::from_string("0.0000 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    old_total = total();
    old_vec = system_balance(ETUSD.value);
    exchange( N(bob), ETUSD, extend(asset::from_string("30000000000.0000 TUSD")),
      asset::from_string("40000000.0000 EOS") );
    new_total = total();
    new_vec = system_balance(ETUSD.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    old_total = total();    
    old_vec = system_balance(EVO.value);
    exchange( N(bob), EVO, extend(asset::from_string("4000.0000 EOS")), 
      asset::from_string("1.0000 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    withdraw( N(bob), N(bob), extended_asset{asset::from_string("0.0001 EOS"), N(eosio.token)});
    BOOST_REQUIRE_EQUAL(total() == new_total, false);

    old_total = total();
    old_vec = system_balance(EVO.value);
    addliquidity( N(alice), asset::from_string("150000000000000.0000 EVO"), 
      asset::from_string("400000000000000.0000 EOS"), 
      asset::from_string("400000000000000.0000 VOICE") );
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    old_total = total(); 
    old_vec = system_balance(EVO.value);
    exchange( N(alice), EVO, extend(asset::from_string("387592687324317.3478 EOS")), 
      asset::from_string("0.0001 VOICE"));
    new_total = total();
    new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(old_total == new_total, true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( memoexchange_test, evolutiondex_tester ) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create( N(alice), asset::from_string("461168601842738.7903 EOS") );
    create( N(bob), asset::from_string("461168601842738.7903 VOICE") );
    create( N(alice), asset::from_string("461168601842738.7903 TUSD") );
    issue( N(alice), N(alice), asset::from_string("461168601842738.7903 EOS"), "");
    issue( N(bob), N(bob), asset::from_string("461168601842738.7903 VOICE"), "");
    issue( N(alice), N(alice), asset::from_string("461168601842738.7903 TUSD"), "");
    transfer( N(eosio.token), N(bob), N(alice), asset::from_string("0.0001 VOICE"), "");

 // Resumir con helper.
    push_action( N(carol), N(carol), N(create), 
      mvo() ("issuer", N(carol))("maximum_supply", asset::from_string("4.0000 EOS") ));
    push_action( N(carol), N(carol), N(issue), 
      mvo()("to", N(carol))( "quantity", asset::from_string("4.0000 EOS"))("memo", ""));
    push_action( N(carol), N(carol), N(create), 
      mvo() ("issuer", N(carol))("maximum_supply", asset::from_string("1.0000 VOICE") ));
    push_action( N(carol), N(carol), N(issue), 
      mvo()("to", N(carol))( "quantity", asset::from_string("1.0000 VOICE"))("memo", ""));

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    openext( N(alice), N(alice), extended_symbol{EOS4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{VOICE4, N(eosio.token)});
    openext( N(alice), N(alice), extended_symbol{TUSD4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{EOS4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{VOICE4, N(eosio.token)});
    openext( N(bob), N(alice), extended_symbol{TUSD4, N(eosio.token)});

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("461000000000000.0000 EOS"), "");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset::from_string("461168601842738.7000 VOICE"), "deposit to: alice");
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset::from_string("0.0903 VOICE"), "");
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("450000000000000.0000 TUSD"), "");
    transfer( N(eosio.token), N(alice), N(bob), asset::from_string("3000000000000.0000 TUSD"), "");

    BOOST_REQUIRE_EQUAL( success(),
      inittoken( N(alice), EVO4,
        extended_asset{asset::from_string("23058430092.1369 EOS"), N(eosio.token)},
        extended_asset{asset::from_string("96116860184.2738 VOICE"), N(eosio.token)},
        12, N(wevotethefee)) );

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 166536 VOICE") );

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 16.6536 VOICE") );

    // agregar este chequeo para otro método exchange.
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      transfer( N(carol), N(carol), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 16.6535 VOICE") );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      transfer( N(carol), N(carol), N(evolutiondex), asset::from_string("1.0000 VOICE"), 
      "exchange: EVO, 0.0001 EOS") );

    int64_t pre_eos_balance = token_balance(N(eosio.token), N(alice), EOS.value);
    int64_t pre_voice_balance = token_balance(N(eosio.token), N(alice), VOICE.value);
    BOOST_REQUIRE_EQUAL( success(), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 16.6535 VOICE") );
    BOOST_REQUIRE_EQUAL( pre_eos_balance - 40000, token_balance(N(eosio.token), N(alice), EOS.value) );
    BOOST_REQUIRE_EQUAL( pre_voice_balance + 166535, token_balance(N(eosio.token), N(alice), VOICE.value) );

    inittoken( N(alice), ETUSD4,
      extended_asset{asset::from_string("10000000000.0000 EOS"), N(eosio.token)},
      extended_asset{asset::from_string("99116860184.2738 TUSD"), N(eosio.token)},
      0, N(wevotethefee));

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      transfer( N(eosio.token), N(alice), N(evolutiondex),
        asset::from_string("4000.0000 TUSD"), "exchange: ETUSD, 403.5641 EOS") );

    pre_eos_balance = token_balance(N(eosio.token), N(alice), EOS.value);
    int64_t pre_tusd_balance = token_balance(N(eosio.token), N(alice), TUSD.value);
    BOOST_REQUIRE_EQUAL( success(), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4000.0000 TUSD"), 
      "exchange: ETUSD, 403.5639 EOS") );

    BOOST_REQUIRE_EQUAL( pre_tusd_balance - 40000000, 
      token_balance(N(eosio.token), N(alice), TUSD.value) );
    BOOST_REQUIRE_EQUAL( pre_eos_balance + 4035640, 
      token_balance(N(eosio.token), N(alice), EOS.value) );

    auto old_vec = system_balance(EVO.value);
    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 10000 VOICE, nothing to say");
    auto new_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);

    old_vec = system_balance(ETUSD.value);
    transfer( N(eosio.token), N(bob), N(evolutiondex), 
      asset::from_string("300000000000000.0000 TUSD"), "exchange: ETUSD, 40000000000000 EOS,");
    new_vec = system_balance(ETUSD.value);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, new_vec), true);
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( evo_tests_asserts, evolutiondex_tester ) try {

    create( N(alice), asset::from_string("1000000000000.0000 EOS") );
    create( N(bob), asset::from_string("1000000000000.0000 VOICE") );
    issue( N(alice), N(alice), asset::from_string("100000000.0000 EOS"), "some memo");
    issue( N(bob), N(bob), asset::from_string("1000000000.0000 VOICE"), "");
    transfer( N(eosio.token), N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");
    transfer( N(eosio.token), N(alice), N(bob), asset::from_string("0.0001 EOS"), "");

    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol not registered for this user,\
 please run openext action or write exchange details in the memo of your transfer"),
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("0.1000 EOS"), "") );

    // OPENEXT
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{EOS4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{VOICE4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "user account does not exist"), 
      openext( N(cat), N(alice), extended_symbol{VOICE4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{VOICE4, N(eosio.token)}) );

    BOOST_REQUIRE_EQUAL( success(), transfer( N(eosio.token), N(alice), N(evolutiondex), 
      asset::from_string("1000.0000 EOS"), "") );
    BOOST_REQUIRE_EQUAL( success(), transfer( N(eosio.token), N(alice), N(evolutiondex),
      asset::from_string("20000.0000 VOICE"), "") );

    // INITTOKEN
    BOOST_REQUIRE_EQUAL( success(), inittoken( N(alice), EVO4, 
      extended_asset{asset::from_string("0.0001 EOS"), N(eosio.token)},
      extended_asset{asset::from_string("0.1000 VOICE"), N(eosio.token)}, 10, N(foo)) );
    // Agregar chequeos

    // si quiero hacer exchange excediendo los conectores salta "invalid parameters" (compute)

    // CLOSEEXT
    BOOST_REQUIRE_EQUAL( success(), open( N(alice), EVO4, N(alice)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("User does not have such token"), closeext( N(alice), 
      N(bob), extended_symbol{TUSD4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
        extended_symbol{VOICE4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), closeext( N(alice), N(alice), 
      extended_symbol{VOICE4, N(eosio.token)}) );

    BOOST_REQUIRE_EQUAL( success(), withdraw( N(alice), N(bob),
      extend(asset::from_string("999.9998 EOS")) ) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), withdraw( N(alice), N(bob),
      extend(asset::from_string("0.0002 EOS")) ));

    BOOST_REQUIRE_EQUAL( 9999999, token_balance( N(eosio.token), N(bob), EOS.value ));
    BOOST_REQUIRE_EQUAL( success(), closeext( N(alice), N(bob),
      extended_symbol{EOS4, N(eosio.token)} ) );
    BOOST_REQUIRE_EQUAL( 10000000, token_balance( N(eosio.token), N(bob), EOS.value ));

    // testear todos los checks, excepto las funciones de eosio.token.

    BOOST_CHECK( "missing authority of foo" == changefee(EVO, 50));
} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()