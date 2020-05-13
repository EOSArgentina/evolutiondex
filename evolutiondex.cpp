#include "evolutiondex.hpp"

using namespace evolution;

void evolutiondex::openext( const name& user, const name& payer, const extended_symbol& ext_symbol) {
    check( is_account( user ), "user account does not exist" );
    require_auth( payer );
    evodexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( make128key(ext_symbol.get_contract().value, ext_symbol.get_symbol().raw()) );
    if( acnt_balance == index.end() ) {
        acnts.emplace( payer, [&]( auto& a ){
            a.balance = extended_asset{0, ext_symbol};
            a.id = acnts.available_primary_key();
        });
    }
}

void evolutiondex::closeext( const name& user, const name& to, const extended_symbol& ext_symbol, string memo) {
    require_auth( user );
    evodexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( make128key(ext_symbol.get_contract().value, ext_symbol.get_symbol().raw()) );
    check( acnt_balance != index.end(), "User does not have such token" );
    auto ext_balance = acnt_balance->balance;
    if (ext_balance.quantity.amount > 0) {
        action(permission_level{ get_self(), "active"_n }, ext_balance.contract, "transfer"_n,
          std::make_tuple( get_self(), to, ext_balance.quantity, memo) ).send();
    }
    index.erase( acnt_balance );
}

void evolutiondex::deposit(name from, name to, asset quantity, string memo) {
    if (from == get_self()) return;
    check(to == get_self(), "This transfer is not for evolutiondex");
    check(quantity.amount >= 0, "quantity must be positive");
    if ( (memo.substr(0, 9)) == "exchange:") {
      memoexchange(from, quantity, get_first_receiver(), memo.substr(9));
    } else {
      if ( (memo.substr(0, 12)) == "deposit to: ") {
          from = name(memo.substr(12));
          check(from != get_self(), "Donation not accepted");
      }
      extended_asset incoming = extended_asset{quantity, get_first_receiver()};
      add_signed_ext_balance(from, incoming);
    }
}

void evolutiondex::withdraw(name user, name to, extended_asset to_withdraw, string memo){
    require_auth( user );
    check(to_withdraw.quantity.amount > 0, "quantity must be positive");
    add_signed_ext_balance(user, -to_withdraw);
    action(permission_level{ get_self(), "active"_n }, to_withdraw.contract, "transfer"_n,
      std::make_tuple( get_self(), to, to_withdraw.quantity, memo) ).send();
}

void evolutiondex::addliquidity(name user, asset to_buy, 
  asset max_asset1, asset max_asset2) {
    require_auth(user);
    check( (to_buy.amount > 0), "to_buy amount must be positive");
    check( (max_asset1.amount >= 0) && (max_asset2.amount >= 0), "assets must be nonnegative");
    add_signed_liq(user, to_buy, true, max_asset1, max_asset2);
}

void evolutiondex::remliquidity(name user, asset to_sell,
  asset min_asset1, asset min_asset2) {
    require_auth(user);
    check(to_sell.amount > 0, "to_sell amount must be positive");
    check( (min_asset1.amount >= 0) && (min_asset2.amount >= 0), "assets must be nonnegative");
    add_signed_liq(user, -to_sell, false, -min_asset1, -min_asset2);
}

// computes x * y / z plus the fee
int64_t evolutiondex::compute(int64_t x, int64_t y, int64_t z, int fee) {
    check( (x != 0) && (y > 0) && (z > 0), "invalid parameters");
    int128_t prod = int128_t(x) * int128_t(y);
    int128_t tmp = 0;
    int128_t tmp_fee = 0;
    if (x > 0) {
        tmp = 1 + (prod - 1) / int128_t(z);
        check( (tmp <= MAX), "computation overflow" );
        tmp_fee = (tmp * fee + 9999) / 10000;
    } else {
        tmp = prod / int128_t(z);
        check( (tmp >= -MAX), "computation underflow" );
        tmp_fee =  (-tmp * fee + 9999) / 10000;
    }
    tmp += tmp_fee;
    return int64_t(tmp);
}

void evolutiondex::add_signed_liq(name user, asset to_add, bool is_buying,
  asset max_asset1, asset max_asset2){
    check( to_add.is_valid(), "invalid asset");
    stats statstable( get_self(), to_add.symbol.code().raw() );
    const auto& token = statstable.find( to_add.symbol.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    auto A = token-> supply.amount;
    auto P1 = token-> pool1.quantity.amount;
    auto P2 = token-> pool2.quantity.amount;

    int fee = is_buying? token->fee : 0;
    auto to_pay1 = extended_asset{ asset{compute(to_add.amount, P1, A, fee),
      token->pool1.quantity.symbol}, token->pool1.contract};
    auto to_pay2 = extended_asset{ asset{compute(to_add.amount, P2, A, fee),
      token->pool2.quantity.symbol}, token->pool2.contract};

    check( to_pay1.quantity <= max_asset1, "available is less than expected"); // <= operator checks
    check( to_pay2.quantity <= max_asset2, "available is less than expected"); // symbol identity
    add_signed_ext_balance(user, -to_pay1);
    add_signed_ext_balance(user, -to_pay2);

    (to_add.amount > 0)? add_balance(user, to_add, user) : sub_balance(user, -to_add);
    require_recipient(token->fee_contract);
    statstable.modify( token, same_payer, [&]( auto& a ) {
      a.supply += to_add;
      a.pool1 += to_pay1;
      a.pool2 += to_pay2;
    });
    if (token-> supply.amount == 0) statstable.erase(token);
}

void evolutiondex::exchange( name user, symbol_code through, asset asset1, asset asset2) {
    check( ((asset1.amount > 0) && (asset2.amount < 0)) ||
           ((asset1.amount < 0) && (asset2.amount > 0)), "one quantity must be positive and one negative");
    stats statstable( get_self(), through.raw() );
    const auto& token = statstable.find( through.raw() );
    check ( token != statstable.end(), "token does not exist" );
    check ( asset1.symbol == token->pool1.quantity.symbol , "first symbol mismatch");
    check ( asset2.symbol == token->pool2.quantity.symbol , "second symbol mismatch");

    auto P1 = token-> pool1.quantity.amount;
    auto P2 = token-> pool2.quantity.amount;
    auto A1 = asset1.amount;

    int64_t A2 = compute(-A1, P2, P1 + A1, token->fee);
    check(A2 <= asset2.amount, "available is less than expected");
    auto ext_asset1 = extended_asset{A1, token-> pool1.get_extended_symbol()};
    auto ext_asset2 = extended_asset{A2, token-> pool2.get_extended_symbol()};

    add_signed_ext_balance(user, -ext_asset1);
    add_signed_ext_balance(user, -ext_asset2);

    statstable.modify( token, same_payer, [&]( auto& a ) {
      a.pool1 += ext_asset1;
      a.pool2 += ext_asset2;
    });
}

// details has the structure "EVOTOKN,min_expected_asset;memo"
void evolutiondex::memoexchange(name user, asset quantity, name contract, string details){
    int comma = details.find(",");
    check(comma != -1, "there must be a comma between evotoken symbol and min_expected_asset");
    int semicolon = details.find(";");
    check(semicolon > comma, "there must be a semicolon between min_expected_asset and memo");
    string symbol_code_string = details.substr(0, comma);
    string min_expected_asset_string = details.substr(comma + 1, semicolon - comma - 1);
    string memo = details.substr(semicolon + 1);
    
    int last_space = symbol_code_string.rfind(" ");
    symbol_code_string = symbol_code_string.substr(last_space + 1);
    auto through = symbol_code(symbol_code_string);
    auto min_expected_asset = string_to_asset(min_expected_asset_string);

    stats statstable( get_self(), through.raw() );
    const auto& token = statstable.find( through.raw() );
    check ( token != statstable.end(), "token does not exist" );
    bool in_first;
    if ((token->pool1.quantity.symbol == quantity.symbol) && 
        (token->pool2.quantity.symbol == min_expected_asset.symbol)) {
        in_first = true;
        check(token->pool1.contract == contract, "you are transfering from an incorrect contract");
    // testear estos mandando desde otros contratos.
    } else if ((token->pool2.quantity.symbol == quantity.symbol) &&
             (token->pool1.quantity.symbol == min_expected_asset.symbol)) {
        in_first = false;
        check(token->pool2.contract == contract, "you are transfering from an incorrect contract");
    }
    else check(false, "symbol mismatch"); 
    int64_t P_in, P_out;
    if (in_first) { 
      P_in = token-> pool1.quantity.amount;
      P_out = token-> pool2.quantity.amount;
    } else {
      P_in = token-> pool2.quantity.amount;
      P_out = token-> pool1.quantity.amount;
    }
    auto A_in = quantity.amount;
    int64_t A_out = compute(-A_in, P_out, P_in + A_in, token->fee);
    check(min_expected_asset.amount <= -A_out, "available is less than expected");
    extended_asset ext_asset1, ext_asset2, ext_asset_out;
    if (in_first) { 
      ext_asset1 = extended_asset{A_in, token-> pool1.get_extended_symbol()};
      ext_asset2 = extended_asset{A_out, token-> pool2.get_extended_symbol()};
      ext_asset_out = -ext_asset2;
    } else {
      ext_asset1 = extended_asset{A_out, token-> pool1.get_extended_symbol()};
      ext_asset2 = extended_asset{A_in, token-> pool2.get_extended_symbol()};
      ext_asset_out = -ext_asset1;
    }
    statstable.modify( token, same_payer, [&]( auto& a ) {
      a.pool1 += ext_asset1;
      a.pool2 += ext_asset2;
    });
    action(permission_level{ get_self(), "active"_n }, ext_asset_out.contract, "transfer"_n,
      std::make_tuple( get_self(), user, ext_asset_out.quantity, memo) ).send();
}

asset evolutiondex::string_to_asset(string input) {
    int space = input.find(" ");
    check(space == input.rfind(" "), "only one space admitted in min_expected_asset");
    string amount_string = input.substr(0, space);
    string symbol_code_string = input.substr(space + 1);
    int dot = amount_string.find(".");
    check(dot == amount_string.rfind("."), "at most one dot admitted in amount_string");
    int precision; long long amount;
    if (dot == -1) {
      precision = 0;
      amount = stoll(amount_string);
    } else {
      precision = amount_string.length() - dot - 1;
      amount = stoll(amount_string.erase(dot,1));
    }
    print(amount);
    auto sym = symbol(symbol_code(symbol_code_string), precision);
    return asset{amount, sym};
}

void evolutiondex::inittoken(name user, symbol new_symbol, extended_asset ext_asset1,
extended_asset ext_asset2, int initial_fee, name fee_contract)
{
    require_auth( user );
    check((ext_asset1.quantity.amount > 0) && (ext_asset2.quantity.amount > 0), "Both assets must be positive");
    check((ext_asset1.quantity.amount < INIT_MAX) && (ext_asset2.quantity.amount < INIT_MAX), "Initial amounts must be less than 10^15");
    int128_t geometric_mean = sqrt(int128_t(ext_asset1.quantity.amount) * int128_t(ext_asset2.quantity.amount));
    auto new_token = asset{int64_t(geometric_mean), new_symbol};
    check( ext_asset1.get_extended_symbol() != ext_asset2.get_extended_symbol(), "extended symbols must be different");
    stats statstable( get_self(), new_token.symbol.code().raw() );
    const auto& token = statstable.find( new_token.symbol.code().raw() );
    check ( token == statstable.end(), "token symbol already exists" );
    check( (0 <= initial_fee) && (initial_fee <= 500), "initial fee out of reasonable range");

    statstable.emplace( user, [&]( auto& a ) {
        a.supply = new_token;
        a.max_supply = asset{MAX,new_token.symbol};
        a.issuer = get_self();
        a.pool1 = ext_asset1;
        a.pool2 = ext_asset2;
        a.fee = initial_fee;
        a.fee_contract = fee_contract;
    } );

    add_balance(user, new_token, user);
    add_signed_ext_balance(user, -ext_asset1);
    add_signed_ext_balance(user, -ext_asset2);
}

void evolutiondex::changefee(symbol sym, int newfee) {
    check( (0 <= newfee) && (newfee <= 500), "new fee out of reasonable range");
    stats statstable( get_self(), sym.code().raw() );
    const auto& token = statstable.find( sym.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    require_auth(token->fee_contract);
    statstable.modify( token, same_payer, [&]( auto& a ) {
      a.fee = newfee;
    } );
}

uint128_t evolutiondex::make128key(uint64_t a, uint64_t b) {
    uint128_t aa = a;
    uint128_t bb = b;
    return (aa << 64) + bb;
}

void evolutiondex::add_signed_ext_balance( const name& user, const extended_asset& to_add )
{
    check( to_add.quantity.is_valid(), "invalid asset" );
    evodexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( make128key(to_add.contract.value, to_add.quantity.symbol.raw() ) );
    check( acnt_balance != index.end(), "extended_symbol not registered for this user,\
 please run openext action or write exchange details in the memo of your transfer");
    index.modify( acnt_balance, same_payer, [&]( auto& a ) {
        a.balance += to_add;
        check( a.balance.quantity.amount >= 0, "insufficient funds");
    });
}
