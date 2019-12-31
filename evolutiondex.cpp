#include "evolutiondex.hpp"

using namespace evolution;

void evolutiondex::open( const name& user, const name& payer, const name& smartctr, const asset& asset_to_open)
{
    check( is_account( user ), "user account does not exist" );
    require_auth( user );
    check( asset_to_open.amount == 0, "initial balance must be zero");

    evodexacnts acnts( get_self(), user.value );
    auto acnt_balance = acnts.find( smartctr.value + asset_to_open.symbol.code().raw() );

    if( acnt_balance == acnts.end() ) {
      acnts.emplace( payer, [&]( auto& a ){
          a.balance = extended_asset{asset{0, asset_to_open.symbol}, smartctr};
      });
    }
}  

void evolutiondex::close( const name& user, const name& smartctr, const asset& asset_to_close ) {
   require_auth( user );
   evodexacnts acnts( get_self(), user.value );
   
   auto acnt_balance = acnts.find( smartctr.value + asset_to_close.symbol.code().raw() );
   check( acnt_balance != acnts.end(), "User does not have such token" );

   check( acnt_balance->balance.quantity.amount == 0, "Cannot close because the balance is not zero." );
   acnts.erase( acnt_balance );
}

void evolutiondex::deposit(name from, name to, asset quantity, string memo) {
    check( get_first_receiver() != get_self(), "token already lives in evolutiondex"); // tal vez no es necesario
    check(quantity.amount >= 0, "quantity must be positive");
    if (from == get_self()) return;
    check(to == get_self(), "This transfer is not for evolutiondex"); // hace falta este check?
    extended_asset incoming = extended_asset{quantity, get_first_receiver()};
    add_balance(from, incoming);
}

void evolutiondex::withdraw(name user, name smartctr, asset to_withdraw){
    require_auth( user );
    check(to_withdraw.amount > 0, "quantity must be positive");
    add_balance(user, -extended_asset{to_withdraw, smartctr});
    action(permission_level{ _self, "active"_n },
        smartctr, "transfer"_n,
        std::make_tuple( get_self(), user, to_withdraw, std::string("Withdraw"))
        ).send(); 
}

void evolutiondex::transfer( const name& from, const name& to, const asset& quantity, const string& memo)
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check(quantity.amount > 0, "quantity must be positive"); // qué mira "isvalid"?

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

    extended_asset ext_asset1 = extended_asset{asset1, token->connector1.contract};
    extended_asset ext_asset2 = extended_asset{asset2, token->connector2.contract};
    
    auto A = token-> supply.amount;
    auto E = token-> connector1.quantity.amount;
    auto V = token-> connector2.quantity.amount;;
    auto E_in = asset1.amount;
    auto V_in = asset2.amount;

    int64_t tok_out = 0;
    if (is_exchange_operation) {
        int64_t E_out = E * pow( (double)V / ((double)V + (double)V_in), HALF / HALF) - E;
        check(E_out <= E_in, "available is less than expected");
        ext_asset1.quantity.amount = E_out;
        print("tokens out: ", ext_asset1, "\n");
    } else {
        tok_out = tokens_out(A, E, V, E_in, V_in);
        check(tok_out >= min_expected.amount, "available is less than expected");
        extended_asset ext_asset_add = extended_asset{asset{tok_out, min_expected.symbol}, get_self()};
        print("tokens out: ", ext_asset_add, "\n");
        add_balance(user, ext_asset_add);
    }

    double dfee((double) token->fee / 1000);
    int64_t fee1 = ext_asset1.quantity.amount <= 0 ? 0 : (ext_asset1.quantity.amount + 999) * dfee;
    int64_t fee2 = ext_asset2.quantity.amount <= 0 ? 0 : (ext_asset2.quantity.amount + 999) * dfee;
    print("Fees: ", fee1, " ", fee2, "\n");

    ext_asset1.quantity.amount += fee1;
    ext_asset2.quantity.amount += fee2;
    add_balance(user, -ext_asset1 );
    add_balance(user, -ext_asset2 );

    // modificar statstable
    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.supply.amount += tok_out;
      a.connector1 += ext_asset1;
      a.connector2 += ext_asset2;
      print("Nuevo supply es ", a.supply, ". Connector 1: ", a.connector1, 
        ". Connector 2: ", a.connector2 );
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
// en eosio.token.cpp sub_balance y add_balance, notar la diferencia en las declaraciones de to y from (const, &) 
    check( to != to_acnts.end(), "Symbol not registered for this user");
    check( to->balance.quantity.symbol == to_add.quantity.symbol, "extended_token mismatch"); // evita ataque de colision de keys. De yapa chequea la igualdad entre las 'precision'. Averiguar si ya lo chequea la suma de extended assets.
    to_acnts.modify( to, ""_n, [&]( auto& a ) {  // puede pasar que consuma más ram?
        a.balance += to_add;
        check( a.balance.quantity.amount >= 0, "insufficient funds");
        check( a.balance.quantity.amount <= MAX, "balance cannot be larger than 10^18");
        eosio::print("Nuevo saldo de user: ", a.balance, "\n");
    });
}

int64_t evolutiondex::tokens_out(int64_t A, int64_t E, int64_t V, int64_t E_in, int64_t V_in) {
    int64_t result = A * pow( ((double)E+(double)E_in)/(double)E, HALF ) * pow( ((double)V+(double)V_in)
    / (double)V, HALF )  - A; 
    return result;
}

void evolutiondex::inittoken(name user, name smartctr1, asset asset1, 
    name smartctr2, asset asset2, asset new_token, name fee_contract)
{ 
    require_auth( user );
    check(asset1.symbol != asset2.symbol, "connector symbols must be different");

    extended_asset ext_asset1 = extended_asset{asset1, smartctr1};
    extended_asset ext_asset2 = extended_asset{asset2, smartctr2};
    extended_asset ext_new_token = extended_asset{new_token, get_self()};    

    add_balance(user, ext_new_token);
    add_balance(user, -ext_asset1);
    add_balance(user, -ext_asset2);

    // emplace de stats
    stats statstable( get_self(), get_self().value );
    auto token = statstable.find( new_token.symbol.code().raw() );
    check ( token == statstable.end(), "token already exists" );
    statstable.emplace( user, [&]( auto& a ) {
        a.supply = new_token;
        a.max_supply = asset{MAX,new_token.symbol};
        a.connector1 = ext_asset1;
        a.connector2 = ext_asset2;
        a.fee = 1;
        a.fee_contract = fee_contract;
    } ); 
}

void evolutiondex::changefee(symbol sym, int newfee) {  // función no testeada 
    stats statstable( get_self(), get_self().value );
    auto token = statstable.find( sym.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    check( get_first_receiver() == token->fee_contract, "contract not authorized to change fee." );
    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.fee = newfee;
    } ); 
}