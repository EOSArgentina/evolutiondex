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

        create_accounts( { N(alice), N(bob), N(carol), N(eosio.token), N(anothertoken), N(evolutiondex),
          N(wevotethefee), N(badtoken) } );
        produce_blocks( 2 );

        set_code( N(eosio.token), contracts::token_wasm() );
        set_abi( N(eosio.token), contracts::token_abi().data() );

        set_code( N(anothertoken), contracts::token_wasm() );
        set_abi( N(anothertoken), contracts::token_abi().data() );

        set_code( N(evolutiondex), contracts::evolutiondex_wasm() );
        set_abi( N(evolutiondex), contracts::evolutiondex_abi().data() );

        set_code( N(carol), contracts::token_wasm() );
        set_abi( N(carol), contracts::token_abi().data() );

        set_code( N(badtoken), contracts::badtoken_wasm() );
        set_abi( N(badtoken), contracts::badtoken_abi().data() );

        set_code( N(wevotethefee), contracts::wevotethefee_wasm() );
        set_abi( N(wevotethefee), contracts::wevotethefee_abi().data() );
        
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
    action_result create( name contract, account_name issuer, asset maximum_supply ) {
        return push_action( contract, contract, N(create), mvo()
            ( "issuer", issuer)
            ( "maximum_supply", maximum_supply)
        );
    }
    action_result issue( name contract, account_name issuer, account_name to, asset quantity, string memo ) {
        return push_action( contract, issuer, N(issue), mvo()
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
    action_result inittoken( name user, symbol new_symbol, extended_asset initial_pool1,
      extended_asset initial_pool2, int initial_fee, name fee_contract){
        return push_action( N(evolutiondex), user, N(inittoken), mvo()
          ( "user", user)
          ("new_symbol", new_symbol)
          ("initial_pool1", initial_pool1)
          ("initial_pool2", initial_pool2) 
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
    action_result onesideremli(name user, asset to_sell, 
      extended_asset min_expected, string memo) {
        return push_action( N(evolutiondex), user, N(onesideremli), mvo()
          ( "user", user )
          ( "to_sell", to_sell )
          ( "min_expected", min_expected)
          ( "memo", memo)
        );
    }
    action_result exchange( name user, symbol_code pair_token, extended_asset ext_asset_in, asset min_expected ) {
        return push_action( N(evolutiondex), user, N(exchange), mvo()
          ( "user", user )
          ( "pair_token", pair_token )
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

    action_result openfeetable( name user, symbol_code pair_token ) {
        return push_action( N(wevotethefee), user, N(openfeetable), mvo()
          ( "user", user )
          ( "pair_token", pair_token )
        );
    }
    action_result closefeetable( symbol_code pair_token ) {
        return push_action( N(wevotethefee), N(wevotethefee), N(closefeetable), mvo()
          ( "pair_token", pair_token )
        );
    }
    action_result votefee( name user, symbol_code pair_token, int fee_voted ) {
        return push_action( N(wevotethefee), user, N(votefee), mvo()
          ( "user", user )
          ( "pair_token", pair_token )
          ( "fee_voted", fee_voted )
        );
    }
    action_result closevote( name user, symbol_code pair_token ) {
        return push_action( N(wevotethefee), user, N(closevote), mvo()
          ( "user", user )
          ( "pair_token", pair_token )
        );
    }
    action_result updatefee( name user, symbol_code pair_token ) {
      return push_action( N(wevotethefee), user, N(updatefee), mvo()
        ( "pair_token", pair_token )
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
        auto EVO_value = symbol::from_string("4,EVO").to_symbol_code().value;
        auto ETUSD_value = symbol::from_string("4,ETUSD").to_symbol_code().value;
        int64_t total_eos =  balance(N(alice), 0) + balance(N(bob), 0) 
          + system_balance(EVO_value).at(0) + system_balance(ETUSD_value).at(0);
        int64_t total_voice = balance(N(alice), 1) + balance(N(bob), 1) 
          + system_balance(EVO_value).at(1);
        int64_t total_tusd = balance(N(alice), 2) + balance(N(bob), 2)
          + system_balance(ETUSD_value).at(1);
        vector <int64_t> ans = {total_eos, total_voice, total_tusd};
        return ans;
    }
    void create_tokens_and_issue() {
        create( N(eosio.token), N(alice), asset::from_string("461168601842738.7903 EOS") );
        create( N(anothertoken), N(bob), asset::from_string("461168601842738.7903 VOICE") );
//        create( N(eosio.token), N(bob), asset::from_string("461168601842738.7903 VOICE") );
        create( N(eosio.token), N(alice), asset::from_string("461168601842738.7903 TUSD") );
        issue( N(eosio.token), N(alice), N(alice), asset::from_string("461168601842738.7903 EOS"), "");
        issue( N(anothertoken), N(bob), N(bob), asset::from_string("461168601842738.7903 VOICE"), "");
//        issue( N(eosio.token), N(bob), N(bob), asset::from_string("461168601842738.7903 VOICE"), "");
        issue( N(eosio.token), N(alice), N(alice), asset::from_string("461168601842738.7903 TUSD"), "");
    }
    void many_openext() {
        openext( N(alice), N(alice), extended_symbol{symbol::from_string("4,EOS"), N(eosio.token)});
        openext( N(alice), N(alice), extended_symbol{symbol::from_string("4,VOICE"), N(anothertoken)});
//        openext( N(alice), N(alice), extended_symbol{symbol::from_string("4,VOICE"), N(eosio.token)});
        openext( N(alice), N(alice), extended_symbol{symbol::from_string("4,TUSD"), N(eosio.token)});
        openext( N(bob), N(alice), extended_symbol{symbol::from_string("4,EOS"), N(eosio.token)});
        openext( N(bob), N(alice), extended_symbol{symbol::from_string("4,VOICE"), N(anothertoken)});
//        openext( N(bob), N(alice), extended_symbol{symbol::from_string("4,VOICE"), N(eosio.token)});
        openext( N(bob), N(alice), extended_symbol{symbol::from_string("4,TUSD"), N(eosio.token)});
    }
    void many_transfer() {
        transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("461000000000000.0000 EOS"), "");
        transfer( N(anothertoken), N(bob), N(evolutiondex), asset::from_string("461168601842738.7000 VOICE"), "deposit to: alice");
        transfer( N(anothertoken), N(bob), N(evolutiondex), asset::from_string("0.0903 VOICE"), "this goes to Bob");
        transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("450000000000000.0000 TUSD"), "");
        transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("3000000000000.0000 TUSD"), "deposit to: bob");
    }

    void prepare_carol_token() {
        push_action( N(carol), N(carol), N(create), 
          mvo() ("issuer", N(carol))("maximum_supply", asset::from_string("4.0000 EOS") ));
        push_action( N(carol), N(carol), N(issue), 
          mvo()("to", N(carol))( "quantity", asset::from_string("4.0000 EOS"))("memo", ""));
        push_action( N(carol), N(carol), N(create), 
          mvo() ("issuer", N(carol))("maximum_supply", asset::from_string("1.0000 VOICE") ));
        push_action( N(carol), N(carol), N(issue), 
          mvo()("to", N(carol))( "quantity", asset::from_string("1.0000 VOICE"))("memo", ""));
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
  if (to_extend.symbol_name() == "VOICE") {
    return extended_asset{to_extend, N(anothertoken)};  
  } else {
    return extended_asset{to_extend, N(eosio.token)};
  }
}

BOOST_AUTO_TEST_SUITE(evolutiondex_tests)

BOOST_FIXTURE_TEST_CASE( add_rem_exchange, evolutiondex_tester ) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create_tokens_and_issue();
    transfer( N(anothertoken), N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    many_openext();

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("10000000.0000 EOS"), "");
    transfer( N(anothertoken), N(alice), N(evolutiondex), asset::from_string("200000000.0000 VOICE"), "");
    
    inittoken( N(alice), EVO4,
      extend(asset::from_string("1000000.0000 EOS")), 
      extend(asset::from_string("100000000.0000 VOICE")), 10, N(wevotethefee));

    auto alice_evo_balance = get_balance(N(evolutiondex), N(alice), N(accounts), EVO.value, "account");
    auto bal = mvo() ("balance", asset::from_string("10000000.0000 EVO"));
    BOOST_REQUIRE_EQUAL( fc::json::to_string(alice_evo_balance, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ), 
    fc::json::to_string(bal, fc::time_point(fc::time_point::now() + abi_serializer_max_time) ) );

// ADDLIQUIDITY
    BOOST_REQUIRE_EQUAL( error("missing authority of alice"), 
      push_action( N(evolutiondex), N(bob), N(addliquidity), mvo()
          ( "user", N(alice))( "to_buy", asset::from_string("1 EVO"))
          ( "max_asset1", asset::from_string("1 EOS") )
          ( "max_asset2", asset::from_string("1 VOICE")) )
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("to_buy amount must be positive"), 
      addliquidity( N(alice), asset::from_string("-5.0000 EVO"), 
      asset::from_string("0.5000 EOS"), asset::from_string("5000.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("assets must be nonnegative"), 
      addliquidity( N(alice), asset::from_string("2.0000 EVO"), 
      asset::from_string("-0.3000 EOS"), asset::from_string("30.0000 NOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("assets must be nonnegative"), 
      addliquidity( N(alice), asset::from_string("2.0000 EVO"), 
      asset::from_string("0.3000 EOS"), asset::from_string("-30.0000 NOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      addliquidity( N(alice), asset::from_string("5.0000 EVO"), 
      asset::from_string("0.5000 EOS"), asset::from_string("5000.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      addliquidity( N(alice), asset::from_string("2.0000 EVO"), 
      asset::from_string("0.0000 EOS"), asset::from_string("20.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("pair token does not exist"), 
      addliquidity( N(alice), asset::from_string("2.0000 EMMO"), 
      asset::from_string("0.0000 EOS"), asset::from_string("20.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("incorrect symbol"), 
      addliquidity( N(alice), asset::from_string("3.0000 EVO"), 
      asset::from_string("0.3000 ECOS"), asset::from_string("30.0001 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), 
      addliquidity( N(alice), asset::from_string("1000000000000.0000 EVO"), 
      asset::from_string("1000000000000.0000 EOS"), asset::from_string("20000000000000.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( success(), 
      addliquidity( N(alice), asset::from_string("50.0000 EVO"), 
      asset::from_string("5.0050 EOS"), asset::from_string("500.5000 VOICE") )
    );
    produce_blocks();

// REMLIQUIDITY
    BOOST_REQUIRE_EQUAL( error("missing authority of alice"), 
      push_action( N(evolutiondex), N(bob), N(remliquidity), mvo()
          ( "user", N(alice))( "to_sell", asset::from_string("1 EVO"))
          ( "min_asset1", asset::from_string("1 EOS") )
          ( "min_asset2", asset::from_string("1 VOICE")) )
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("to_sell amount must be positive"), 
      remliquidity( N(alice), asset::from_string("-5.0000 EVO"), 
      asset::from_string("0.5000 EOS"), asset::from_string("5000.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("assets must be nonnegative"), 
      remliquidity( N(alice), asset::from_string("3.0000 EVO"), 
      asset::from_string("-0.3000 EOS"), asset::from_string("30.0001 NOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("assets must be nonnegative"), 
      remliquidity( N(alice), asset::from_string("3.0000 EVO"), 
      asset::from_string("0.3000 EOS"), asset::from_string("-30.0001 NOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      remliquidity( N(alice), asset::from_string("1.0000 EVO"), 
      asset::from_string("0.1001 EOS"), asset::from_string("10.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      remliquidity( N(alice), asset::from_string("3.0000 EVO"), 
      asset::from_string("0.3000 EOS"), asset::from_string("30.0001 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("incorrect symbol"), 
      remliquidity( N(alice), asset::from_string("3.0000 EVO"), 
      asset::from_string("0.3000 EOS"), asset::from_string("30.0001 NOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("overdrawn balance"), 
      remliquidity( N(alice), asset::from_string("1000000000000.0000 EVO"), 
      asset::from_string("0.0000 EOS"), asset::from_string("0.0000 VOICE")));

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("computation overflow"), 
      addliquidity( N(alice), asset::from_string("46116860184273.8791 EVO"), 
      asset::from_string("1.0000 EOS"), asset::from_string("1.0000 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("computation underflow"), 
      remliquidity( N(alice), asset::from_string("46116860184273.8791 EVO"), 
      asset::from_string("1.0000 EOS"), asset::from_string("1.0000 VOICE")));

    BOOST_REQUIRE_EQUAL( success(), 
      remliquidity( N(alice), asset::from_string("10000050.0000 EVO"),
      asset::from_string("0.0001 EOS"), asset::from_string("0.0001 VOICE") )
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("pair token does not exist"), 
      addliquidity( N(alice), asset::from_string("2.0000 EVO"), 
      asset::from_string("0.0000 EOS"), asset::from_string("20.0000 VOICE"))
    );

// set pools
    inittoken( N(alice), EVO4,
      extend(asset::from_string("1000000.0000 EOS")),
      extend(asset::from_string("100000000.0000 VOICE")), 10, N(wevotethefee));
    addliquidity( N(alice), asset::from_string("50.0000 EVO"), 
      asset::from_string("5.0050 EOS"), asset::from_string("500.5000 VOICE") );
    remliquidity( N(alice), asset::from_string("17.1872 EVO"),
      asset::from_string("0.0000 EOS"), asset::from_string("0.0000 VOICE") );

// EXCHANGE
    BOOST_REQUIRE_EQUAL( error("missing authority of alice"), 
      push_action( N(evolutiondex), N(bob), N(exchange), mvo()
          ( "user", N(alice))( "pair_token", EVO )
          ( "ext_asset_in", extend(asset::from_string("1 EOS")) )
          ( "min_expected", asset::from_string("1 VOICE")) )
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg(
      "ext_asset_in must be nonzero and min_expected must have same sign or be zero"), 
      exchange( N(alice), EVO, extend(asset::from_string("2.0000 VOICE")), 
      asset::from_string("-0.1000 EOS")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg(
      "ext_asset_in must be nonzero and min_expected must have same sign or be zero"), 
      exchange( N(alice), EVO, extend(asset::from_string("-2.0000 RICE")), 
      asset::from_string("0.1000 REOS")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg(
      "ext_asset_in must be nonzero and min_expected must have same sign or be zero"), 
      exchange( N(alice), EVO, extend(asset::from_string("0.0000 EOS")), 
      asset::from_string("-0.1000 VOICE")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("invalid parameters"), 
      exchange( N(alice), EVO, extend(asset::from_string("-1000004.0000 EOS")),
      asset::from_string("-0.0001 VOICE")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("invalid parameters"), 
      exchange( N(alice), EVO, extend(asset::from_string("-100000328.6280 VOICE")), 
      asset::from_string("0.0000 EOS")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("pair token does not exist"), 
      exchange( N(alice), TUSD, extend(asset::from_string("-8.0000 VOICE")), 
      asset::from_string("0.0000 EOS")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      exchange( N(alice), EVO, extend(asset::from_string("4.000 EOS")), 
      asset::from_string("10.0000 VOICE")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      exchange( N(alice), EVO, extended_asset{asset::from_string("4.0000 EOS"), N(another)}, 
      asset::from_string("10.0000 VOICE")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      exchange( N(alice), EVO, extended_asset{asset::from_string("1.0000 VOICE"), N(another)}, 
      asset::from_string("1.0000 EOS")) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"),
      exchange( N(alice), EVO, extend(asset::from_string("4.0000 EOS")),
      asset::from_string("400.0000 VOICE")) );

    exchange( N(alice), EVO, extend(asset::from_string("4.0000 EOS")), asset::from_string("10.0000 VOICE"));
    exchange( N(alice), EVO, extend(asset::from_string("0.1000 EOS")), asset::from_string("4.8500 VOICE"));
    exchange( N(alice), EVO, extend(asset::from_string("0.0001 EOS")), asset::from_string("0.0009 VOICE"));

    vector <int64_t> expected_system_balance = {10000073864, 999999190299, 100000328128};
    BOOST_REQUIRE_EQUAL(expected_system_balance == system_balance(EVO.value), true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0), 89999926136);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1), 1000000809701);

    BOOST_REQUIRE_EQUAL( success(), changefee(EVO, 50) );

    addliquidity( N(alice), asset::from_string("50.0000 EVO"),
      asset::from_string("10000000.0000 EOS"), asset::from_string("10000000.0000 VOICE") );

    expected_system_balance = {10000124116, 1000004215279, 100000828128};
    BOOST_REQUIRE_EQUAL(expected_system_balance == system_balance(EVO.value), true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0), 89999875884);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1), 999995784721);

// one_side_addliquidity and onesiremli, assert tests
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("pair token does not exist"), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), 
        asset::from_string("10000.0000 EOS"), "add liquidity: 10.0000 NOT")
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("token to provide must match one side of the pool"), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), 
        asset::from_string("1.0000 TUSD"), "add liquidity: 10.0000 EVO")
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"),
      transfer( N(anothertoken), N(alice), N(evolutiondex), 
        asset::from_string("1.0000 VOICE"), "add liquidity: 10.0000 EVO")
    );
//  unreachable assert in one_side_addliquidity: wasm_assert_msg("to_buy amount must be positive")
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("to_sell amount must be positive"), 
        onesideremli( N(alice), asset::from_string("-5.6000 EVO"), 
        extend(asset::from_string("0.1000 EOS")), "")
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("min_expected must be nonnegative"), 
        onesideremli( N(alice), asset::from_string("5.6000 EVO"), 
        extend(asset::from_string("-0.1000 EOS")), "")
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
        onesideremli( N(alice), asset::from_string("5.6000 EVO"), 
        extend(asset::from_string("100.0000 EOS")), "")
    );

// first side add    
    auto old_system_balance = expected_system_balance;
    auto old_alice_eos_balance = token_balance(N(eosio.token), N(alice), EOS.value);
    BOOST_REQUIRE_EQUAL( success(), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), 
        asset::from_string("10000.0000 EOS"), "add liquidity: 10.0000 EVO")
    );
    expected_system_balance = {10000144218, 1000004215279, 100000928128};
    BOOST_REQUIRE_EQUAL(expected_system_balance == system_balance(EVO.value), true);
    auto new_alice_eos_balance = token_balance(N(eosio.token), N(alice), EOS.value);
    BOOST_REQUIRE_EQUAL( true, old_alice_eos_balance == 20102 + new_alice_eos_balance);
    BOOST_REQUIRE_EQUAL( true, is_increasing( old_system_balance, expected_system_balance ) );
// test Alice evotoken balance, and check overdraw case.

// second side add
    old_system_balance = expected_system_balance;
    auto old_alice_voice_balance = token_balance(N(anothertoken), N(alice), VOICE.value);
    BOOST_REQUIRE_EQUAL( success(), 
      transfer( N(anothertoken), N(alice), N(evolutiondex), 
        asset::from_string("10000.0000 VOICE"), "add liquidity: 10.0000 EVO")
    );
    expected_system_balance = {10000144218, 1000006225270, 100001028128};
    BOOST_REQUIRE_EQUAL(expected_system_balance == system_balance(EVO.value), true);
    auto new_alice_voice_balance = token_balance(N(anothertoken), N(alice), VOICE.value);
    BOOST_REQUIRE_EQUAL( true, old_alice_voice_balance == 2009991 + new_alice_voice_balance);
    BOOST_REQUIRE_EQUAL( true, is_increasing( old_system_balance, system_balance(EVO.value) ) );

// first side rem li
    old_system_balance = expected_system_balance;
    old_alice_eos_balance = token_balance(N(eosio.token), N(alice), EOS.value);
    BOOST_REQUIRE_EQUAL( success(), 
        onesideremli( N(alice), asset::from_string("5.6000 EVO"), 
        extend(asset::from_string("0.1000 EOS")), "deliver to your cousin")
    );
    expected_system_balance = {10000133018, 1000006225270, 100000972128};
    BOOST_REQUIRE_EQUAL(expected_system_balance == system_balance(EVO.value), true);
    new_alice_eos_balance = token_balance(N(eosio.token), N(alice), EOS.value);
    BOOST_REQUIRE_EQUAL( true, old_alice_eos_balance == -11200 + new_alice_eos_balance);
    BOOST_REQUIRE_EQUAL( true, is_increasing( old_system_balance, expected_system_balance ) );

// second side rem li
    old_system_balance = expected_system_balance;
    old_alice_voice_balance = token_balance(N(anothertoken), N(alice), VOICE.value);
    BOOST_REQUIRE_EQUAL( success(),
        onesideremli( N(alice), asset::from_string("0.0100 EVO"), 
        extend(asset::from_string("0.0001 VOICE")), "deliver to your cousin")
    );
    expected_system_balance = {10000133018, 1000006223271, 100000972028};
    BOOST_REQUIRE_EQUAL(expected_system_balance == system_balance(EVO.value), true);
    new_alice_voice_balance = token_balance(N(anothertoken), N(alice), VOICE.value);
    BOOST_REQUIRE_EQUAL( true, old_alice_voice_balance == -1999 + new_alice_voice_balance);
    BOOST_REQUIRE_EQUAL( true, is_increasing( old_system_balance, expected_system_balance ) );

//    cout << system_balance(EVO.value).at(0) << " " << system_balance(EVO.value).at(1) << " "
//      << system_balance(EVO.value).at(2) << " ";
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( increasing_parameter, evolutiondex_tester) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create_tokens_and_issue();
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    many_openext(); 
    many_transfer();
    transfer( N(eosio.token), N(alice), N(evolutiondex), 
    asset::from_string("168601842738.7903 EOS"), "");

    inittoken( N(alice), EVO4, 
      extend(asset::from_string("23058430092.1369 EOS")),
      extend(asset::from_string("96116860184.2738 VOICE")), 10, N(wevotethefee));

    inittoken( N(alice), ETUSD4, 
      extend(asset::from_string("10000000000.0000 EOS")),
      extend(asset::from_string("99116860184.2738 TUSD")), 10, N(wevotethefee));

    auto old_total = total();
    auto old_vec = system_balance(EVO.value);
    auto old_alice_bal_0 = balance(N(alice),0);
    auto old_alice_bal_1 = balance(N(alice),1);

    BOOST_REQUIRE_EQUAL(success(), 
      exchange( N(alice), EVO, extend(asset::from_string("4.0000 EOS")),
      asset::from_string("1.0000 VOICE") ));
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0) - old_alice_bal_0, -40000);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1) - old_alice_bal_1, 166569);

    old_total = total();
    old_vec = system_balance(EVO.value);
    old_alice_bal_0 = balance(N(alice), 0);
    old_alice_bal_1 = balance(N(alice), 1);
    addliquidity( N(alice), asset::from_string("0.0001 EVO"), 
      asset::from_string("10000000.0000 EOS"), asset::from_string("10000000.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
    BOOST_REQUIRE_EQUAL(balance(N(alice),0) - old_alice_bal_0, -2);
    BOOST_REQUIRE_EQUAL(balance(N(alice),1) - old_alice_bal_1, -4);

    produce_blocks();

    old_total = total();
    old_vec = system_balance(EVO.value);
    old_alice_bal_0 = balance(N(alice), 0);
    old_alice_bal_1 = balance(N(alice), 1);
    remliquidity( N(alice), asset::from_string("0.0001 EVO"),
      asset::from_string("0.0000 EOS"), asset::from_string("0.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
    BOOST_REQUIRE_EQUAL(balance(N(alice), 0) - old_alice_bal_0, 0);
    BOOST_REQUIRE_EQUAL(balance(N(alice), 1) - old_alice_bal_1, 2);

    old_total = total();
    old_vec = system_balance(ETUSD.value);
    BOOST_REQUIRE_EQUAL(success(), 
      exchange( N(bob), ETUSD, 
      extend(asset::from_string("30000000000.0000 TUSD")),
      asset::from_string("40000000.0000 EOS") ) );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(ETUSD.value)), true);

    old_total = total();    
    old_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL(success(), 
      exchange( N(bob), EVO, extend(asset::from_string("4000.0000 EOS")), 
      asset::from_string("1.0000 VOICE") ) );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);

    old_total = total();
    BOOST_REQUIRE_EQUAL( success(), withdraw( N(bob), N(bob), 
      extend(asset::from_string("0.0001 EOS"))) );
    BOOST_REQUIRE_EQUAL(old_total == total(), false);

    old_total = total();
    old_vec = system_balance(EVO.value);
    addliquidity( N(alice), asset::from_string("150000000000000.0000 EVO"), 
       asset::from_string("400000000000000.0000 EOS"), asset::from_string("400000000000000.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);

    old_total = total(); 
    old_vec = system_balance(EVO.value);
    exchange( N(alice), EVO, extend(asset::from_string("387592687324317.3478 EOS")), 
      asset::from_string("0.0001 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
    
    old_total = total(); 
    old_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      exchange( N(alice), EVO, extend(asset::from_string("-1.0000 EOS")), 
      asset::from_string("-0.1068 VOICE")) );
    BOOST_REQUIRE_EQUAL( success(), exchange( N(alice), EVO, 
      extend(asset::from_string("-1.0000 EOS")), asset::from_string("-0.1069 VOICE")) );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);

    old_total = total(); 
    old_vec = system_balance(EVO.value);
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      exchange( N(bob), EVO, extend(asset::from_string("-13.0001 VOICE")), 
      asset::from_string("-122.0328 EOS")) );
    BOOST_REQUIRE_EQUAL( success(), exchange( N(bob), EVO, 
      extend(asset::from_string("-13.0001 VOICE")), asset::from_string("-122.0329 EOS")) );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( increasing_parameter_zero_fee, evolutiondex_tester ) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create_tokens_and_issue();

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    many_openext();
    many_transfer();
    transfer( N(eosio.token), N(alice), N(evolutiondex), 
      asset::from_string("168601842738.7903 EOS"), "");

    inittoken( N(alice), EVO4,
      extend(asset::from_string("23058430092.1369 EOS")),
      extend(asset::from_string("96116860184.2738 VOICE")), 0, N(wevotethefee));

    inittoken( N(alice), ETUSD4,
      extend(asset::from_string("10000000000.0000 EOS")),
      extend(asset::from_string("99116860184.2738 TUSD")), 0, N(wevotethefee));

    auto old_total = total();
    auto old_vec = system_balance(EVO.value);
    exchange( N(alice), EVO, extend(asset::from_string("4.0000 EOS")), 
      asset::from_string("1.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
    
    old_total = total();
    old_vec = system_balance(EVO.value);
    addliquidity( N(alice), asset::from_string("0.0001 EVO"), 
      asset::from_string("10000000.0000 EOS"), asset::from_string("10000000.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
    
    produce_blocks();

    old_total = total();
    old_vec = system_balance(EVO.value);
    remliquidity( N(alice), asset::from_string("0.0001 EVO"), 
      asset::from_string("0.0000 EOS"), asset::from_string("0.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);

    old_total = total();
    old_vec = system_balance(ETUSD.value);
    exchange( N(bob), ETUSD, extend(asset::from_string("30000000000.0000 TUSD")),
      asset::from_string("40000000.0000 EOS") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(ETUSD.value)), true);

    old_total = total();    
    old_vec = system_balance(EVO.value);
    exchange( N(bob), EVO, extend(asset::from_string("4000.0000 EOS")), 
      asset::from_string("1.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);

    old_total = total();    
    withdraw( N(bob), N(bob), extend(asset::from_string("0.0001 EOS")) );
    BOOST_REQUIRE_EQUAL(old_total == total(), false);

    old_total = total();
    old_vec = system_balance(EVO.value);
    addliquidity( N(alice), asset::from_string("150000000000000.0000 EVO"), 
      asset::from_string("400000000000000.0000 EOS"), 
      asset::from_string("400000000000000.0000 VOICE") );
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);

    old_total = total(); 
    old_vec = system_balance(EVO.value);
    exchange( N(alice), EVO, extend(asset::from_string("387592687324317.3478 EOS")), 
      asset::from_string("0.0001 VOICE"));
    BOOST_REQUIRE_EQUAL(old_total == total(), true);
    BOOST_REQUIRE_EQUAL(is_increasing(old_vec, system_balance(EVO.value)), true);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( memoexchange_test, evolutiondex_tester ) try {
    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);

    create_tokens_and_issue();
    prepare_carol_token();
    transfer( N(anothertoken), N(bob), N(alice), asset::from_string("0.0001 VOICE"), "");

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    many_openext();
    many_transfer();

    BOOST_REQUIRE_EQUAL( success(),
      inittoken( N(alice), EVO4,
        extend(asset::from_string("23058430092.1369 EOS")),
        extend(asset::from_string("96116860184.2738 VOICE")), 12, N(wevotethefee)) );

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 166536 VOICE") );

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("available is less than expected"), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 16.6536 VOICE") );

    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      transfer( N(carol), N(carol), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 16.6535 VOICE") );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol mismatch"), 
      transfer( N(carol), N(carol), N(evolutiondex), asset::from_string("1.0000 VOICE"), 
      "exchange: EVO, 0.0001 EOS") );

    int64_t pre_eos_balance = token_balance(N(eosio.token), N(alice), EOS.value);
    int64_t pre_voice_balance = token_balance(N(anothertoken), N(alice), VOICE.value);
    BOOST_REQUIRE_EQUAL( success(), 
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("4.0000 EOS"), 
      "exchange: EVO, 16.6535 VOICE") );
    BOOST_REQUIRE_EQUAL( pre_eos_balance - 40000, token_balance(N(eosio.token), N(alice), EOS.value) );
    BOOST_REQUIRE_EQUAL( pre_voice_balance + 166535, token_balance(N(anothertoken), N(alice), VOICE.value) );

    inittoken( N(alice), ETUSD4,
      extend(asset::from_string("10000000000.0000 EOS")),
      extend(asset::from_string("99116860184.2738 TUSD")), 0, N(wevotethefee));

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


BOOST_FIXTURE_TEST_CASE( the_other_actions, evolutiondex_tester ) try {

    create_tokens_and_issue();
    transfer( N(anothertoken), N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    // add_signed_ext_balance
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol not registered for this user,\
 please run openext action or write exchange details in the memo of your transfer"),
      transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("0.1000 EOS"), "") );

    // OPENEXT
    BOOST_REQUIRE_EQUAL( error("missing authority of natalia"), 
      push_action( N(evolutiondex), N(bob), N(openext), mvo()
        ( "user", N(bob))( "payer", N(natalia) )
        ( "ext_symbol", extended_symbol{VOICE4, N(eosio.token)} ))
    );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{EOS4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{VOICE4, N(anothertoken)}) );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{VOICE4, N(anothertoken)}) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg( "user account does not exist"), 
      openext( N(cat), N(alice), extended_symbol{VOICE4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), openext( N(alice), N(alice), 
      extended_symbol{VOICE4, N(eosio.token)}) );

    // ONTRANSFER
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("This transfer is not for evolutiondex"), 
      transfer( N(badtoken), N(alice), N(bob), asset::from_string("1000.0000 EOS"), ""));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("quantity must be positive"), 
      transfer( N(badtoken), N(alice), N(evolutiondex), asset::from_string("-1000.0000 EOS"), ""));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("Donation not accepted"), transfer( N(eosio.token), N(alice), N(evolutiondex), 
      asset::from_string("1000.0000 EOS"), "deposit to: evolutiondex") );
    BOOST_REQUIRE_EQUAL( success(), transfer( N(eosio.token), N(alice), N(evolutiondex), 
      asset::from_string("1000.0000 EOS"), "") );
    BOOST_REQUIRE_EQUAL( success(), transfer( N(anothertoken), N(alice), N(evolutiondex),
      asset::from_string("20000.0000 VOICE"), "") );

    // WITHDRAW
    BOOST_REQUIRE_EQUAL( error("missing authority of alice"), 
      push_action( N(evolutiondex), N(bob), N(withdraw), mvo()
        ("user", N(alice)) ("to", N(natalia))
        ("to_withdraw", extend(asset::from_string("1.0000 EOS"))) ("memo", "") )
    ); 
    BOOST_REQUIRE_EQUAL( success(), withdraw( N(alice), N(bob),
      extend(asset::from_string("999.9998 EOS")) ) );
    BOOST_REQUIRE_EQUAL( 9999998, token_balance( N(eosio.token), N(bob), EOS.value ));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("quantity must be positive"), withdraw( N(alice), N(bob),
      extend(asset::from_string("-0.0001 EOS")) ));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), withdraw( N(alice), N(bob),
      extend(asset::from_string("0.0003 EOS")) ));

    // INITTOKEN
    BOOST_REQUIRE_EQUAL( error("missing authority of alice"), 
      push_action( N(evolutiondex), N(bob), N(inittoken), mvo()
        ("user", N(alice)) ("new_symbol", EVO4)
        ("initial_pool1", extend(asset::from_string("1.0000 EOS")))
        ("initial_pool2", extend(asset::from_string("1.0000 ECO")))
        ("initial_fee", 1) ("fee_contract", N(carol)) )
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("Both assets must be positive"), inittoken( N(alice), EVO4, 
      extend(asset::from_string("-0.0001 EOS")),
      extend(asset::from_string("0.1000 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("Both assets must be positive"), inittoken( N(alice), EVO4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("-0.1000 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("Initial amounts must be less than 10^15"), inittoken( N(alice), EVO4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("100000000000.0001 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("Initial amounts must be less than 10^15"), inittoken( N(alice), EVO4, 
      extend(asset::from_string("100000000000.0001 EOS")),
      extend(asset::from_string("1.0001 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended symbols must be different"), inittoken( N(alice), EVO4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.1000 EOS")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), 
      inittoken( N(alice), ETUSD4, 
      extend(asset::from_string("0.0003 EOS")), 
      extend(asset::from_string("0.1000 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("insufficient funds"), 
      inittoken( N(alice), ETUSD4, 
      extend(asset::from_string("0.0002 EOS")), 
      extend(asset::from_string("100000000.0000 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( success(), inittoken( N(alice), EVO4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.1000 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("token symbol already exists"), inittoken( N(alice), EVO4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.1000 VOICE")), 10, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("initial fee out of reasonable range"), inittoken( N(alice), ETUSD4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.1000 VOICE")), 501, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("initial fee out of reasonable range"), inittoken( N(alice), ETUSD4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.1000 VOICE")), -1, N(wevotethefee)) );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("fee_contract account must exist or be empty"), inittoken( N(alice), ETUSD4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.1000 VOICE")), 1, N(nonexistent)) );
    BOOST_REQUIRE_EQUAL( success(), inittoken( N(alice), ETUSD4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.0001 VOICE")), 1, N( )) );
    
    // behaviour when fee_contract is empty 
    BOOST_REQUIRE_EQUAL( success(), 
      transfer( N(evolutiondex), N(alice), N(bob), asset::from_string("0.0001 ETUSD"), "") );
    BOOST_REQUIRE_EQUAL( success(), 
      transfer( N(evolutiondex), N(bob), N(alice), asset::from_string("0.0001 ETUSD"), "") );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("this pair token has fixed fee parameter"),
      changefee(ETUSD, 20) );
    BOOST_REQUIRE_EQUAL( success(), remliquidity( N(alice), asset::from_string("0.0001 ETUSD"),
      asset::from_string("0.0001 EOS"), asset::from_string("0.0001 VOICE")));

  // TRANSFER
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("extended_symbol not registered for this user,\
 please run openext action or write exchange details in the memo of your transfer"),
      transfer(N(evolutiondex), N(alice), N(evolutiondex), asset::from_string("0.0010 EVO"), "")
    );

  // CLOSEEXT
    BOOST_REQUIRE_EQUAL( error("missing authority of natalia"), 
      push_action( N(evolutiondex), N(bob), N(closeext), mvo()
        ("user", N(natalia))("ext_symbol", extended_symbol{EVO4, N(eosio.token)})
        ("to", N(alice))("memo", "") )
    );
    BOOST_REQUIRE_EQUAL( success(), 
      closeext( N(alice), N(alice), extended_symbol{VOICE4, N(eosio.token)}) );
    BOOST_REQUIRE_EQUAL( success(), 
      closeext( N(alice), N(bob), extended_symbol{EOS4, N(eosio.token)} ) );
    BOOST_REQUIRE_EQUAL( 9999999, token_balance( N(eosio.token), N(bob), EOS.value ));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("User does not have such token"), 
      closeext( N(alice), N(bob), extended_symbol{EOS4, N(eosio.token)} ) );

  // CHANGEFEE
    BOOST_REQUIRE_EQUAL( error("missing authority of wevotethefee"), 
      push_action( N(evolutiondex), N(bob), N(changefee), 
        mvo() ("pair_token", EVO) ("newfee", 50) )
    );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("pair token does not exist"), changefee(EOS, 500));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("new fee out of reasonable range"), changefee(EVO, 501));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("new fee out of reasonable range"), changefee(EVO, -5));
    BOOST_REQUIRE_EQUAL( success(), changefee(EVO, 50) );

  // Notifications to fee_contract 
    many_openext();
    transfer( N(eosio.token), N(bob), N(evolutiondex), asset::from_string("0.0002 EOS"), "");
    transfer( N(anothertoken), N(bob), N(evolutiondex), asset::from_string("0.0002 VOICE"), "");
    BOOST_REQUIRE_EQUAL( success(), inittoken( N(bob), ETUSD4, 
      extend(asset::from_string("0.0001 EOS")),
      extend(asset::from_string("0.0001 VOICE")), 0, N(badtoken) ) ); 
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("notification received"), 
      transfer( N(evolutiondex), N(bob), N(alice), asset::from_string("0.0001 ETUSD"), "") );
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("notification received"), 
      addliquidity( N(bob), asset::from_string("0.0001 ETUSD"), 
      asset::from_string("0.0001 EOS"), asset::from_string("0.0001 VOICE")));
    BOOST_REQUIRE_EQUAL( wasm_assert_msg("notification received"), 
      remliquidity( N(bob), asset::from_string("0.0001 ETUSD"), 
      asset::from_string("0.0001 EOS"), asset::from_string("0.0001 VOICE")));

} FC_LOG_AND_RETHROW()



BOOST_FIXTURE_TEST_CASE( we_vote_the_fee, evolutiondex_tester ) try {

    create_tokens_and_issue();
    transfer( N(anothertoken), N(bob), N(alice), asset::from_string("500000000.0000 VOICE"), "");

    const auto& accnt2 = control->db().get<account_object,by_name>( N(evolutiondex) );
    abi_def abi_evo;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt2.abi, abi_evo), true);
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);

    many_openext();

    transfer( N(eosio.token), N(alice), N(evolutiondex), asset::from_string("10000000.0000 EOS"), "");
    transfer( N(anothertoken), N(alice), N(evolutiondex), asset::from_string("200000000.0000 VOICE"), "");
    
    inittoken( N(alice), EVO4,
      extend(asset::from_string("1000000.0000 EOS")), 
      extend(asset::from_string("100000000.0000 VOICE")), 10, N(wevotethefee));

    const auto& accnt3 = control->db().get<account_object,by_name>( N(wevotethefee) );
    abi_def abi_wevote;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt3.abi, abi_wevote), true);

// Start wevotethefee actions. Testing checks and basic functions.
    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    auto ISNT = symbol::from_string("4,ISNT").to_symbol_code();
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("pair_token balance does not exist"), 
      votefee(N(alice), ISNT, 30) );
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("pair_token balance does not exist"), 
      votefee(N(bob), EVO, 30) );
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("fee table nonexistent, run openfeetable"), votefee(N(alice), EVO, 30));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("fee table nonexistent, run openfeetable"), updatefee(N(alice), EVO));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("table does not exist"), closefeetable(EVO));
    BOOST_REQUIRE_EQUAL(success(), openfeetable(N(alice), EVO));
    BOOST_REQUIRE_EQUAL(success(), closefeetable(EVO));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("fee table nonexistent, run openfeetable"), votefee(N(alice), EVO, 30));
    BOOST_REQUIRE_EQUAL(success(), openfeetable(N(alice), EVO));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("already opened"), openfeetable(N(alice), EVO));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("there are no votes"), updatefee(N(alice), EVO));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("user is not voting"), closevote(N(alice), EVO));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("only values between 1 and 300 allowed"), votefee(N(alice), EVO, 301));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("only values between 1 and 300 allowed"), votefee(N(alice), EVO, -10));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("only values between 1 and 300 allowed"), votefee(N(alice), EVO, 0));

    BOOST_REQUIRE_EQUAL(success(), votefee(N(alice), EVO, 30));
    BOOST_REQUIRE_EQUAL(success(), updatefee(N(alice), EVO));
    BOOST_REQUIRE_EQUAL(wasm_assert_msg("table of votes is not empty"), closefeetable(EVO));

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    auto evo_stats = get_balance(N(evolutiondex), name(EVO.value), N(stat), EVO.value, "currency_stats" );
    BOOST_REQUIRE_EQUAL(30, evo_stats["fee"]);

    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    auto evo_votes = get_balance(N(wevotethefee), name(EVO.value), N(feetable), EVO.value,
      "feetable" )["votes"].get_array();
    BOOST_REQUIRE_EQUAL(100000000000, evo_votes[8]);

    // Test authorizations
    BOOST_REQUIRE_EQUAL( error("missing authority of bob"),
      push_action( N(wevotethefee), N(alice), N(votefee), 
        mvo()( "user", N(bob) )( "pair_token", EVO )( "fee_voted", "10" ) )
    );
    BOOST_REQUIRE_EQUAL( error("missing authority of bob"),
      push_action( N(wevotethefee), N(alice), N(closevote), 
        mvo()( "user", N(bob) )( "pair_token", EVO ) )
    );

    // Vote balance change after transfer
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    BOOST_REQUIRE_EQUAL(success(), 
      transfer( N(evolutiondex), N(alice), N(bob), asset::from_string("100.0000 EVO"), ""));

    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    BOOST_REQUIRE_EQUAL(success(), votefee(N(bob), EVO, 300));
    evo_votes = get_balance(N(wevotethefee), name(EVO.value), N(feetable), EVO.value,
      "feetable" )["votes"].get_array();
    BOOST_REQUIRE_EQUAL(99999000000, evo_votes[8]);
    BOOST_REQUIRE_EQUAL(1000000, evo_votes[14]);

    // Scenarios with many votes
    create_accounts( { N(dan), N(eva), N(fran)});
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);    
    BOOST_REQUIRE_EQUAL(success(), transfer( N(evolutiondex), N(alice), N(carol), asset::from_string("2000.0000 EVO"), ""));
    transfer( N(evolutiondex), N(alice), N(dan), asset::from_string("400000.0000 EVO"), "");
    transfer( N(evolutiondex), N(alice), N(eva), asset::from_string("4240000.0000 EVO"), "");
    transfer( N(evolutiondex), N(alice), N(fran), asset::from_string("2500000.0000 EVO"), "");

    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    BOOST_REQUIRE_EQUAL(success(), votefee(N(carol), EVO, 1));
    votefee(N(dan), EVO, 2);
    votefee(N(eva), EVO, 99);
    votefee(N(fran), EVO, 108);

    updatefee(N(alice), EVO);
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    evo_stats = get_balance(N(evolutiondex), name(EVO.value), N(stat), EVO.value, "currency_stats" );
    BOOST_REQUIRE_EQUAL(100, evo_stats["fee"]);

    openext( N(eva), N(eva), extended_symbol{symbol::from_string("4,EOS"), N(eosio.token)});
    openext( N(eva), N(eva), extended_symbol{symbol::from_string("4,VOICE"), N(anothertoken)});
    push_action( N(evolutiondex), N(eva), N(remliquidity), mvo()
          ( "user", N(eva))( "to_sell", asset::from_string("4000000.0000 EVO"))
          ( "min_asset1", asset::from_string("1.0000 EOS") )
          ( "min_asset2", asset::from_string("1.0000 VOICE")) );

    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    updatefee(N(alice), EVO);
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    evo_stats = get_balance(N(evolutiondex), name(EVO.value), N(stat), EVO.value, "currency_stats" );
    BOOST_REQUIRE_EQUAL(30, evo_stats["fee"]);

    BOOST_REQUIRE_EQUAL(success(), push_action( N(evolutiondex), N(eva), N(addliquidity), mvo()
          ( "user", N(eva))( "to_buy", asset::from_string("3900000.0000 EVO"))
          ( "max_asset1", asset::from_string("100000000000.0000 EOS") )
          ( "max_asset2", asset::from_string("100000000000.0000 VOICE")) )   );
    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    updatefee(N(alice), EVO);
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    evo_stats = get_balance(N(evolutiondex), name(EVO.value), N(stat), EVO.value, "currency_stats" );
    BOOST_REQUIRE_EQUAL(100, evo_stats["fee"]);

    // median 1
    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    votefee(N(alice), EVO, 1);
    votefee(N(dan), EVO, 1);
    votefee(N(eva), EVO, 1);
    votefee(N(fran), EVO, 1);
    updatefee(N(alice), EVO);
    evo_votes = get_balance(N(wevotethefee), name(EVO.value), N(feetable), EVO.value,
      "feetable" )["votes"].get_array();
    BOOST_REQUIRE_EQUAL(98999000000, evo_votes[0]);
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    evo_stats = get_balance(N(evolutiondex), name(EVO.value), N(stat), EVO.value, "currency_stats" );
    BOOST_REQUIRE_EQUAL(1, evo_stats["fee"]);

    // check votes after transfer, remliquidity and addliquidity
    BOOST_REQUIRE_EQUAL(success(), 
      transfer( N(evolutiondex), N(alice), N(bob), asset::from_string("100.0000 EVO"), ""));
    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
      evo_votes = get_balance(N(wevotethefee), name(EVO.value), N(feetable), EVO.value,
      "feetable" )["votes"].get_array();
    BOOST_REQUIRE_EQUAL(98998000000, evo_votes[0]);
    BOOST_REQUIRE_EQUAL(2000000, evo_votes[14]);

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    push_action( N(evolutiondex), N(eva), N(remliquidity), mvo()
          ( "user", N(eva))( "to_sell", asset::from_string("10000.0000 EVO"))
          ( "min_asset1", asset::from_string("1.0000 EOS") )
          ( "min_asset2", asset::from_string("1.0000 VOICE")) );
    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    evo_votes = get_balance(N(wevotethefee), name(EVO.value), N(feetable), EVO.value,
      "feetable" )["votes"].get_array();
    BOOST_REQUIRE_EQUAL(98998000000 - 100000000, evo_votes[0]);

    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    push_action( N(evolutiondex), N(eva), N(addliquidity), mvo()
          ( "user", N(eva))( "to_buy", asset::from_string("100.0000 EVO"))
          ( "max_asset1", asset::from_string("10000000.0000 EOS") )
          ( "max_asset2", asset::from_string("10000000.0000 VOICE")) );
    abi_ser.set_abi(abi_wevote, abi_serializer_max_time);
    evo_votes = get_balance(N(wevotethefee), name(EVO.value), N(feetable), EVO.value,
      "feetable" )["votes"].get_array();
    BOOST_REQUIRE_EQUAL(98998000000 - 100000000 + 1000000, evo_votes[0]);

    // median 300
    votefee(N(dan), EVO, 300);
    votefee(N(eva), EVO, 300);
    votefee(N(fran), EVO, 300);
    updatefee(N(alice), EVO);
    evo_votes = get_balance(N(wevotethefee), name(EVO.value), N(feetable), EVO.value,
      "feetable" )["votes"].get_array();
    abi_ser.set_abi(abi_evo, abi_serializer_max_time);
    evo_stats = get_balance(N(evolutiondex), name(EVO.value), N(stat), EVO.value, "currency_stats" );
    BOOST_REQUIRE_EQUAL(300, evo_stats["fee"]);

} FC_LOG_AND_RETHROW()


BOOST_AUTO_TEST_SUITE_END()