#include "evolutiondex.hpp"

using namespace evolution;

void evolutiondex::open( const name& user, const name& payer,
  const extended_symbol& ext_symbol, bool is_evotoken) {
    check( is_account( user ), "user account does not exist" );
    require_auth( payer );
    evodexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    auto acnt_balance = index.find( make128key(ext_symbol.get_contract().value, ext_symbol.get_symbol().code().raw()) );
    check( acnt_balance == index.end(), "User already has this account" );
    acnts.emplace( payer, [&]( auto& a ){
        a.balance = extended_asset{0, ext_symbol};
        a.is_evotoken = is_evotoken;
        a.id = acnts.available_primary_key();
    });
}  

void evolutiondex::close( const name& user, const extended_symbol& ext_symbol) {
   require_auth( user );
   evodexacnts acnts( get_self(), user.value );   
   auto index = acnts.get_index<"extended"_n>();
   auto acnt_balance = index.find( make128key(ext_symbol.get_contract().value, ext_symbol.get_symbol().code().raw()) );
   check( acnt_balance != index.end(), "User does not have such token" );
   check( acnt_balance->balance.quantity.amount == 0, "Cannot close because the balance is not zero." );
   index.erase( acnt_balance );
}

void evolutiondex::deposit(name from, name to, asset quantity, string memo) {
    if (from == get_self()) return;
    check(to == get_self(), "This transfer is not for evolutiondex");
    check(quantity.amount >= 0, "quantity must be positive");
    if ( (memo.substr(0, 12)) == "deposit to: ") { // muy rígido? ¿tolower? no es grave, withdraw y repetir.
        from = name(memo.substr(12));
        check(from != get_self(), "Donation not accepted");
    }
    extended_asset incoming = extended_asset{quantity, get_first_receiver()};
    add_balance(from, incoming, false);
}

void evolutiondex::withdraw(name user, extended_asset to_withdraw){
    require_auth( user );
    check(to_withdraw.quantity.amount > 0, "quantity must be positive");
    add_balance(user, -to_withdraw, false);
    action(permission_level{ _self, "active"_n }, to_withdraw.contract, "transfer"_n,
      std::make_tuple( get_self(), user, to_withdraw.quantity, std::string("Withdraw")) ).send(); 
}

void evolutiondex::transfer( const name& from, const name& to, const asset& quantity, const string& memo)
{
    check( to != get_self(), "Donation not accepted. To sell that token use remliquidity.");
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check(quantity.amount > 0, "quantity must be positive"); 
    // falta algún chequeo? ver por ejemplo eosio.token
    require_recipient( from );
    require_recipient( to );
    check( memo.size() <= 256, "memo has more than 256 bytes" );
    add_balance( from, extended_asset{-quantity, name(memo)}, true );
    add_balance( to, extended_asset{quantity, name(memo)}, true );
}

void evolutiondex::remliquidity(name user, extended_asset to_sell, 
  extended_asset min_ext_asset1, extended_asset min_ext_asset2) {
    require_auth(user);
    check(to_sell.quantity.amount > 0, "quantity must be positive");
    add_or_remove(user, -to_sell, false, -min_ext_asset1, -min_ext_asset2);
}

void evolutiondex::addliquidity(name user, extended_asset to_buy,
  extended_asset max_ext_asset1, extended_asset max_ext_asset2) {
    require_auth(user);
    check(to_buy.quantity.amount > 0, "quantity must be positive");
    add_or_remove(user, to_buy, true, max_ext_asset1, max_ext_asset2);
}

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
        eosio::check( (tmp >= -MAX), "computation underflow" );
        tmp_fee =  (-tmp * fee + 9999) / 10000;
    }
    tmp += tmp_fee;
    print("fee is: ",tmp_fee, "\n");
    return int64_t(tmp);
}

void evolutiondex::add_or_remove(name user, extended_asset to_buy, bool is_buying,
  extended_asset max_ext_asset1, extended_asset max_ext_asset2){
    stats statstable( get_self(), to_buy.contract.value );
    auto token = statstable.find( to_buy.quantity.symbol.code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    auto A = token-> supply.amount;
    auto C1 = token-> connector1.quantity.amount;
    auto C2 = token-> connector2.quantity.amount;
    
    int fee = is_buying? token->fee : 0;
    auto to_pay1 = extended_asset{ asset{compute(to_buy.quantity.amount, C1, A, fee), 
      token->connector1.quantity.symbol}, token->connector1.contract};
    auto to_pay2 = extended_asset{ asset{compute(to_buy.quantity.amount, C2, A, fee), 
      token->connector2.quantity.symbol}, token->connector2.contract};

    check( to_pay1 <= max_ext_asset1, "available is less than expected");
    check( to_pay2 <= max_ext_asset2, "available is less than expected");
    add_balance(user, -to_pay1, false);  // ver si se puede sacar de tabla para admitir derivados
    add_balance(user, -to_pay2, false);
    add_balance(user, to_buy, true);
    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.supply += to_buy.quantity;
      a.connector1 += to_pay1;
      a.connector2 += to_pay2;
      check( (a.connector1.quantity.amount >= 0) && (a.connector2.quantity.amount >= 0), "overdrawn balance, bug alert"); // no debería fallar nunca, pero protege fondos en caso de bug
      print("Nuevo supply es ", a.supply, ". Connector 1: ", a.connector1, ". Connector 2: ", a.connector2, "\n");
      print("Fee parameter:", a.fee);
    });
    // ¿agregar chequeo de no disminución de state_parameter? (puedo usar compute)
    if (token-> supply.amount == 0) statstable.erase(token);
}

void evolutiondex::exchange( name user, extended_symbol through, asset asset1, asset asset2) {
    check( ((asset1.amount > 0) && (asset2.amount < 0)) || 
      ((asset1.amount < 0) && (asset2.amount > 0)), "one quantity must be positive and one negative");
    stats statstable( get_self(), through.get_contract().value );
    auto token = statstable.find( through.get_symbol().code().raw() );
    check ( token != statstable.end(), "token does not exist" );
    check ( asset1.symbol == token->connector1.quantity.symbol , "first symbol mismatch");
    check ( asset2.symbol == token->connector2.quantity.symbol , "second symbol mismatch");

    auto C1 = token-> connector1.quantity.amount;
    auto C2 = token-> connector2.quantity.amount;
    auto C1_in = asset1.amount;
    auto C2_in = asset2.amount;
    
    int64_t C2_out = compute(-C1_in, C2, C1 + C1_in, token->fee);
    check(C2_out <= C2_in, "available is less than expected");
    asset2.amount = C2_out;
    print("user obtains: ", -asset1, ", ", -asset2, "\n");

    add_balance(user, extended_asset{-asset1, token->connector1.contract}, false);
    add_balance(user, extended_asset{-asset2, token->connector2.contract}, false);

    statstable.modify( token, ""_n, [&]( auto& a ) {
      a.connector1.quantity += asset1;
      a.connector2.quantity += asset2;
      check( (a.connector1.quantity.amount > 0) && (a.connector2.quantity.amount > 0), "overdrawn balance, bug alert"); // no debería fallar nunca, pero protege fondos en caso de bug
      print("Nuevo supply es ", a.supply, ". Connector 1: ", a.connector1, ". Connector 2: ", a.connector2, "\n");
      print("Fee parameter:", a.fee);
    });
    // ¿agregar chequeo de no disminución de state_parameter?
}

void evolutiondex::add_balance( const name& user, const extended_asset& to_add,
  bool is_evotoken )  // cambiar nombre por add_signed_balance
{
    check( to_add.quantity.is_valid(), "invalid quantity" ); // testear esto depositando desde contrato malicioso
    evodexacnts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    auto acnt_balance = index.find( make128key(to_add.contract.value, to_add.quantity.symbol.code().raw() ) );
    check( acnt_balance != index.end(), "Symbol not registered for this user, please run open action");
    check( acnt_balance->balance.quantity.symbol == to_add.quantity.symbol, "precision mismatch"); // Chequea la igualdad entre las 'precision'.
    check( acnt_balance->is_evotoken == is_evotoken, "is_evotoken mismatch");
    index.modify( acnt_balance, ""_n, [&]( auto& a ) {  // puede pasar que consuma más ram?
        a.balance += to_add;
        check( a.balance.quantity.amount >= 0, "insufficient funds");
        check( a.balance.quantity.amount <= MAX, "balance cannot be larger than 2^62-1");
        print("Saldo de ", user,": ", a.balance, "\n");
        if (is_evotoken) notify_fee_contract(user, a.balance);
    });
}

void evolutiondex::inittoken(name user, extended_asset ext_asset1, 
extended_asset ext_asset2, symbol new_symbol, int initial_fee, name fee_contract) // poner new_symbol al lado de user
{ 
    require_auth( user );
    check((ext_asset1.quantity.amount > 0) && (ext_asset2.quantity.amount > 0), "Both assets must be positive");
    int128_t geometric_mean = sqrt(int128_t(ext_asset1.quantity.amount) * int128_t(ext_asset2.quantity.amount));
    // testear a full la sqrt, valores máximos.
    auto new_token = asset{int64_t(geometric_mean), new_symbol};
    extended_asset ext_new_token = extended_asset{new_token, user};
    check( ext_asset1.get_extended_symbol() != ext_asset2.get_extended_symbol(), "extended symbols must be different");
    stats statstable( get_self(), user.value );
    auto token = statstable.find( new_token.symbol.code().raw() );
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

    add_balance(user, ext_new_token, true);
    add_balance(user, -ext_asset1, false);
    add_balance(user, -ext_asset2, false);
}

void evolutiondex::notify_fee_contract( name user, extended_asset new_balance) {
    stats statstable( get_self(), new_balance.contract.value );
    auto token = statstable.find( new_balance.quantity.symbol.code().raw() );
    if (is_account(token->fee_contract)) { // existe is_action?
        action(permission_level{ _self, "active"_n }, token->fee_contract, "newbalance"_n,
            std::make_tuple(user, new_balance)).send(); 
        // si existe cuenta y contrato pero no la acción "newbalance", tira assert, parece.
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

uint128_t evolutiondex::make128key(uint64_t a, uint64_t b) {
    uint128_t aa = a;
    uint128_t bb = b;
    return (aa << 64) + bb;
}
