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

void evolutiondex::closeext( const name& user, const extended_symbol& ext_symbol) {
    require_auth( user );
    evodexacnts acnts( get_self(), user.value );   
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( make128key(ext_symbol.get_contract().value, ext_symbol.get_symbol().raw()) );
    check( acnt_balance != index.end(), "User does not have such token" );
    auto ext_balance = acnt_balance->balance;
    if (ext_balance.quantity.amount > 0) {
        action(permission_level{ get_self(), "active"_n }, ext_balance.contract, "transfer"_n,
          std::make_tuple( get_self(), user, ext_balance.quantity, std::string("")) ).send(); 
    }
    index.erase( acnt_balance );
}

void evolutiondex::deposit(name from, name to, asset quantity, string memo) {
    if (from == get_self()) return;
    check(to == get_self(), "This transfer is not for evolutiondex");
    check(quantity.amount >= 0, "quantity must be positive");
    if ( (memo.substr(0, 12)) == "deposit to: ") {
        from = name(memo.substr(12));
        check(from != get_self(), "Donation not accepted");
    }
    extended_asset incoming = extended_asset{quantity, get_first_receiver()};
    add_signed_balance(from, incoming);
}

void evolutiondex::withdraw(name user, extended_asset to_withdraw){
    require_auth( user );
    check(to_withdraw.quantity.amount > 0, "quantity must be positive");
    add_signed_balance(user, -to_withdraw);
    action(permission_level{ get_self(), "active"_n }, to_withdraw.contract, "transfer"_n,
      std::make_tuple( get_self(), user, to_withdraw.quantity, std::string("Withdraw")) ).send(); 
}

void evolutiondex::remliquidity(name user, asset to_sell, 
  extended_asset min_ext_asset1, extended_asset min_ext_asset2) {
    require_auth(user);
    check(to_sell.amount > 0, "quantity must be positive");
    add_signed_liq(user, -to_sell, false, -min_ext_asset1, -min_ext_asset2);
}

void evolutiondex::addliquidity(name user, asset to_buy,
  extended_asset max_ext_asset1, extended_asset max_ext_asset2) {
    require_auth(user);
    check(to_buy.amount > 0, "quantity must be positive");
    add_signed_liq(user, to_buy, true, max_ext_asset1, max_ext_asset2);
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
    print("fee is: ",tmp_fee, "\n");
    return int64_t(tmp);
}

void evolutiondex::add_signed_liq(name user, asset to_buy, bool is_buying,
  extended_asset max_ext_asset1, extended_asset max_ext_asset2){
    check( to_buy.is_valid(), "invalid asset");
    stats statstable( get_self(), to_buy.symbol.code().raw() );
    const auto& token = statstable.find( to_buy.symbol.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    auto A = token-> supply.amount;
    auto C1 = token-> connector1.quantity.amount;
    auto C2 = token-> connector2.quantity.amount;
    
    int fee = is_buying? token->fee : 0;
    auto to_pay1 = extended_asset{ asset{compute(to_buy.amount, C1, A, fee), 
      token->connector1.quantity.symbol}, token->connector1.contract};
    auto to_pay2 = extended_asset{ asset{compute(to_buy.amount, C2, A, fee), 
      token->connector2.quantity.symbol}, token->connector2.contract};

    check( to_pay1 <= max_ext_asset1, "available is less than expected");
    check( to_pay2 <= max_ext_asset2, "available is less than expected");
    add_signed_balance(user, -to_pay1);
    add_signed_balance(user, -to_pay2);
    if (to_buy.amount > 0) {
        add_balance(user, to_buy, user);
    } else {
        sub_balance(user, -to_buy);
    }
    require_recipient(token->fee_contract);
    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.supply += to_buy;
      a.connector1 += to_pay1;
      a.connector2 += to_pay2;
      check( (a.connector1.quantity.amount >= 0) && (a.connector2.quantity.amount >= 0), "overdrawn balance, bug alert"); // no debería fallar nunca, pero protege fondos en caso de bug
      print("Nuevo supply es ", a.supply, ". Connector 1: ", a.connector1, ". Connector 2: ", a.connector2, "\n");
      print("Fee parameter:", a.fee);
    });
    if (token-> supply.amount == 0) statstable.erase(token);
}

void evolutiondex::exchange( name user, symbol through, extended_asset ext_asset1, extended_asset ext_asset2) {
    check( ((ext_asset1.quantity.amount > 0) && (ext_asset2.quantity.amount < 0)) || 
           ((ext_asset1.quantity.amount < 0) && (ext_asset2.quantity.amount > 0)), "one quantity must be positive and one negative");
    stats statstable( get_self(), through.code().raw() );
    const auto& token = statstable.find( through.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    check ( ext_asset1.get_extended_symbol() == token->connector1.get_extended_symbol() , "first extended_symbol mismatch");
    check ( ext_asset2.get_extended_symbol() == token->connector2.get_extended_symbol() , "second extended_symbol mismatch");

    auto C1 = token-> connector1.quantity.amount;
    auto C2 = token-> connector2.quantity.amount;
    auto C1_in = ext_asset1.quantity.amount;
    auto C2_in = ext_asset2.quantity.amount;
    
    int64_t C2_out = compute(-C1_in, C2, C1 + C1_in, token->fee);
    check(C2_out <= C2_in, "available is less than expected");
    ext_asset2.quantity.amount = C2_out;
    print("user obtains: ", -ext_asset1, ", ", -ext_asset2, "\n");

    add_signed_balance(user, -ext_asset1);
    add_signed_balance(user, -ext_asset2);

    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.connector1 += ext_asset1;
      a.connector2 += ext_asset2;
      check( (a.connector1.quantity.amount > 0) && (a.connector2.quantity.amount > 0), "overdrawn balance, bug alert"); // no debería fallar nunca, pero protege fondos en caso de bug
      print("Nuevo supply es ", a.supply, ". Connector 1: ", a.connector1, ". Connector 2: ", a.connector2, "\n");
      print("Fee parameter:", a.fee);
    });
}

void evolutiondex::inittoken(name user, symbol new_symbol, extended_asset ext_asset1, 
extended_asset ext_asset2, int initial_fee, name fee_contract)
{ 
    require_auth( user );
    check((ext_asset1.quantity.amount > 0) && (ext_asset2.quantity.amount > 0), "Both assets must be positive");
    check((ext_asset1.quantity.amount < INIT_MAX) && (ext_asset2.quantity.amount < INIT_MAX), "Initial balances must be less than 10^15");
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
        a.connector1 = ext_asset1;
        a.connector2 = ext_asset2;
        a.fee = initial_fee;
        a.fee_contract = fee_contract;
    } ); 

    add_balance(user, new_token, user);
    add_signed_balance(user, -ext_asset1);
    add_signed_balance(user, -ext_asset2);
}

void evolutiondex::updatefee(symbol sym, int newfee) {
    check( (0 <= newfee) && (newfee <= 500), "new fee out of reasonable range");
    stats statstable( get_self(), sym.code().raw() );
    const auto& token = statstable.find( sym.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    check( get_first_receiver() == token->fee_contract, "contract not authorized to change fee." );
    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.fee = newfee;
    } ); 
}

uint128_t evolutiondex::make128key(uint64_t a, uint64_t b) {
    uint128_t aa = a;
    uint128_t bb = b;
    return (aa << 64) + bb;
}

void evolutiondex::add_signed_balance( const name& user, const extended_asset& to_add )
{
    check( to_add.quantity.is_valid(), "invalid asset" );
    evodexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    const auto& acnt_balance = index.find( make128key(to_add.contract.value, to_add.quantity.symbol.raw() ) );
    check( acnt_balance != index.end(), "Extended_symbol not registered for this user, please run openext action");
    index.modify( acnt_balance, ""_n, [&]( auto& a ) {  // puede pasar que consuma más ram?
        a.balance += to_add;
        check( a.balance.quantity.amount >= 0, "insufficient funds");
        print("Saldo de ", user,": ", a.balance, "\n");
    });
}
