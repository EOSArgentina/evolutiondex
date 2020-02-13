#include "evolutiondex.hpp"

using namespace evolution;

void evolutiondex::open( const name& user, const name& payer, const name& smartctr, const symbol& sym)
{
    check( is_account( user ), "user account does not exist" );
    require_auth( payer );

    evodexacnts acnts( get_self(), user.value );
    auto acnt_balance = acnts.find( smartctr.value + sym.code().raw() );

    check( acnt_balance == acnts.end(), "User already has this account" );
    acnts.emplace( payer, [&]( auto& a ){
        a.balance = extended_asset{asset{0, sym}, smartctr};
    });
}  

void evolutiondex::close( const name& user, const name& smartctr, const symbol& sym ) {
   require_auth( user );
   evodexacnts acnts( get_self(), user.value );
   
   auto acnt_balance = acnts.find( smartctr.value + sym.code().raw() );
   check( acnt_balance != acnts.end(), "User does not have such token" );
   check( acnt_balance->balance.quantity.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( acnt_balance );
}

void evolutiondex::deposit(name from, name to, asset quantity, string memo) {
    if (from == get_self()) return;
    check(to == get_self(), "This transfer is not for evolutiondex");
    check( get_first_receiver() != get_self(), "token already lives in evolutiondex");
    check(quantity.amount >= 0, "quantity must be positive");
    // si el memo dice "deposit to: ..." cambiar from para acreditar, no admitir from==get_self()
    extended_asset incoming = extended_asset{quantity, get_first_receiver()};
    add_balance(from, incoming);
}

void evolutiondex::withdraw(name user, name smartctr, asset to_withdraw){
    require_auth( user );
    check(to_withdraw.amount > 0, "quantity must be positive");
    add_balance(user, -extended_asset{to_withdraw, smartctr});
    action(permission_level{ _self, "active"_n }, smartctr, "transfer"_n,
        std::make_tuple( get_self(), user, to_withdraw, std::string("Withdraw")) ).send(); 
}

void evolutiondex::transfer( const name& from, const name& to, const asset& quantity, const string& memo)
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check(quantity.amount > 0, "quantity must be positive"); // comparar con chequeos en eosio.token

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    add_balance( from, extended_asset{-quantity, get_self()} );
    add_balance( to, extended_asset{quantity, get_self()} );
}

void evolutiondex::selltoken(name user, asset asset1, asset asset2, asset max_expected)
{ 
    require_auth(user);
    check(asset1.amount >= 0, "quantity must be positive");
    check(asset2.amount >= 0, "quantity must be positive");
    check(max_expected.amount >= 0, "quantity must be positive");
    operate_token(user, -asset1, -asset2, -max_expected, false);
}

void evolutiondex::buytoken(name user, asset asset1, asset asset2, asset min_expected)
{ 
    require_auth(user);
    check(asset1.amount >= 0, "quantity must be positive");
    check(asset2.amount >= 0, "quantity must be positive");
    check(min_expected.amount >= 0, "quantity must be positive");
    operate_token(user, asset1, asset2, min_expected, false);
}

void evolutiondex::operate_token(name user, asset asset1, asset asset2, 
asset min_expected, bool is_exchange_operation){   
    stats statstable( get_self(), get_self().value );
    auto token = statstable.find( min_expected.symbol.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    check ( asset1.symbol == token->connector1.quantity.symbol , "first symbol mismatch");
    check ( asset2.symbol == token->connector2.quantity.symbol , "second symbol mismatch");

    auto A = token-> supply.amount;
    auto C1 = token-> connector1.quantity.amount;
    auto C2 = token-> connector2.quantity.amount;
    auto C1_in = asset1.amount;
    auto C2_in = asset2.amount;

    double W1 = ((double) (token -> weight1_int) / 10);  // ¿está bien usar double?
    double W2 = 1 - W1;

    if (C1_in > 0) C1_in = (C1_in * (10000 - token->fee)) / 10000;
    if (C2_in > 0) C2_in = (C2_in * (10000 - token->fee)) / 10000;

    int64_t tok_out = 0;
    if (is_exchange_operation) {
        auto final = pow( C1 + C1_in, W1) * pow( C2 + C2_in, W2);
        auto initial = pow(C1, W1) * pow(C2, W2);
        check(final > initial, "available is less than expected");
        print("user obtains: ", -asset1, ", ", -asset2, "\n");
    } else {
        tok_out = A * pow( ((double)C1+(double)C1_in)/(double)C1, W1 ) * 
      pow( ((double)C2+(double)C2_in) / (double)C2, W2 ) - A - 1;
        check(tok_out >= min_expected.amount, "available is less than expected");
        extended_asset ext_asset_add = extended_asset{asset{tok_out, min_expected.symbol}, get_self()};
        add_balance(user, ext_asset_add);
        print("tokens out: ", ext_asset_add, "\n");
    }

    add_balance(user, extended_asset{-asset1, token->connector1.contract});
    add_balance(user, extended_asset{-asset2, token->connector2.contract});

    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.supply.amount += tok_out;
      a.connector1.quantity += asset1;
      a.connector2.quantity += asset2;
      print("Nuevo supply es ", a.supply, ". Connector 1: ", a.connector1, ". Connector 2: ", a.connector2, "\n");
      print("Fee parameter:", a.fee);
    });
}

void evolutiondex::exchange( name user, symbol through, asset asset1, asset asset2) {
    check( ((asset1.amount > 0) && (asset2.amount < 0)) || 
      ((asset1.amount < 0) && (asset2.amount > 0)), "one quantity must be positive and one negative");
    operate_token(user, asset1, asset2, asset{0, through}, true);
}

void evolutiondex::add_balance( const name& user, const extended_asset& to_add )  // cambiar nombre por add_signed_balance
{
    evodexacnts to_acnts( get_self(), user.value );
    auto to = to_acnts.find( to_add.contract.value + to_add.quantity.symbol.code().raw() );
    print( to_add.contract, " le corresponde el valor ", to_add.contract.value, " ");
    print( to_add.quantity.symbol.code(), " le va el valor: ", to_add.quantity.symbol.code().raw() );
    print( " y la suma es: ", to_add.contract.value + to_add.quantity.symbol.code().raw(), " " );
// en eosio.token.cpp sub_balance y add_balance, notar la diferencia en las declaraciones de to y from (const, &) 
    check( to != to_acnts.end(), "Symbol not registered for this user");
    check( to->balance.quantity.symbol == to_add.quantity.symbol, "extended_token mismatch"); // evita ataque de colision de keys. Además chequea la igualdad entre las 'precision'.
    to_acnts.modify( to, ""_n, [&]( auto& a ) {  // puede pasar que consuma más ram?
        a.balance += to_add;
        check( a.balance.quantity.amount >= 0, "insufficient funds");
        check( a.balance.quantity.amount <= MAX, "balance cannot be larger than 10^18");
        print("Saldo de ", user,": ", a.balance, "\n");
        if (a.balance.contract == get_self()) notify_fee_contract(user, a.balance.quantity);
    });
}

void evolutiondex::inittoken(name user, name smartctr1, asset asset1, 
    name smartctr2, asset asset2, asset new_token, int weight1_int, int initial_fee, name fee_contract)
{ 
    require_auth( user );
    extended_asset ext_asset1 = extended_asset{asset1, smartctr1};
    extended_asset ext_asset2 = extended_asset{asset2, smartctr2};
    extended_asset ext_new_token = extended_asset{new_token, get_self()};    
    check( ext_asset1.get_extended_symbol() != ext_asset2.get_extended_symbol(), "extended symbols must be different");
    stats statstable( get_self(), get_self().value );
    auto token = statstable.find( new_token.symbol.code().raw() );
    check ( token == statstable.end(), "token symbol already exists" );
    check ( (0 < weight1_int) && ( weight1_int < 10), "weight1_int must be between 1 and 9");
    check( (0 <= initial_fee) && (initial_fee < 500), "initial fee out of reasonable range");

    statstable.emplace( user, [&]( auto& a ) {
        a.supply = new_token;
        a.max_supply = asset{MAX,new_token.symbol};
        a.connector1 = ext_asset1;
        a.connector2 = ext_asset2;
        a.weight1_int = weight1_int;
        a.fee = initial_fee;
        a.fee_contract = fee_contract;
    } ); 

    add_balance(user, ext_new_token);
    add_balance(user, -ext_asset1);
    add_balance(user, -ext_asset2);
}

void evolutiondex::notify_fee_contract( name user, asset new_balance) {
    stats statstable( get_self(), get_self().value );
    auto token = statstable.find( new_balance.symbol.code().raw() );
    if (is_account(token->fee_contract)) { // existe is_action?
        action(permission_level{ _self, "active"_n }, token->fee_contract, "newbalance"_n,
            std::make_tuple(user, new_balance)).send();
    }
}

void evolutiondex::changefee(symbol sym, int newfee) {
    check( (0 <= newfee) && (newfee < 500), "new fee out of reasonable range");
    stats statstable( get_self(), get_self().value );
    auto token = statstable.find( sym.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    check( get_first_receiver() == token->fee_contract, "contract not authorized to change fee." );
    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.fee = newfee;
    } ); 
}