#include "wesetyourfee.hpp"

// using namespace setfee;

int wesetyourfee::median(extended_symbol ext_sym){
    feetables tables( get_self(), ext_sym.get_contract().value );
    auto table = tables.find( ext_sym.get_symbol().code().raw());
    check( table != tables.end(), "fee table nonexistent, run openfeetable" );
    auto votes = table->votes;
    int64_t sum = 0, partial_sum = 0;
    int index = 0;
    for (int i = 0; i < 9; i++) {
        sum += votes.at(i);
        print("votes at ", fee_vector.at(i), " es: ", votes.at(i), "\n");
    }
    print("sum es: ", sum," ");
    for (int i = 0; 2 * partial_sum < sum; i++) {
        partial_sum += votes.at(i);
        index = i;
    }
    return fee_vector.at(index);
}

void wesetyourfee::changefee(extended_symbol sym, int newfee_index) {
    check( newfee_index == median(sym), "fee value not allowed" );
    require_recipient( "evolutiondex"_n );
}

void wesetyourfee::newbalance(name user, extended_asset new_balance){ // se podría hacer addbalance
    require_auth("evolutiondex"_n);
    // print("user: ", user, ", new_balance: ", new_balance);

    feeaccounts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    auto acnt = index.find( make128key(new_balance.contract.value, new_balance.quantity.symbol.code().raw() ) );
    if ( acnt == index.end() ) {
        return;
    } else {
        auto old_amount = (acnt->balance).quantity.amount;
        index.modify( acnt, user, [&]( auto& a ){
          a.balance = new_balance;
        });
        addvote( (acnt->balance).get_extended_symbol(), acnt->fee_index_voted, 
          new_balance.quantity.amount - old_amount);
    }    
}

extended_asset wesetyourfee::bring_balance(name user, extended_symbol ext_sym) {
    evodexacnts acnts( "evolutiondex"_n, user.value );
    auto index = acnts.get_index<"extended"_n>();
    auto acnt_balance = index.find( make128key( ext_sym.get_contract().value, 
      ext_sym.get_symbol().code().raw() ) );
    return acnt_balance->balance;
}

uint128_t wesetyourfee::make128key(uint64_t a, uint64_t b) {
    uint128_t aa = a;
    uint128_t bb = b;
    return (aa << 64) + bb;
}

void wesetyourfee::votefee(name user, extended_symbol ext_sym, int fee_index_voted){
    require_auth(user);
    check( (0 <= fee_index_voted) && (fee_index_voted < 9), "index voted must be between 0 and 8");
    feeaccounts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    auto acnt = index.find( make128key(ext_sym.get_contract().value,
      ext_sym.get_symbol().code().raw() ) );
    auto balance = bring_balance(user, ext_sym);
    print("balance es: ", balance, "\n");
    if ( acnt == index.end() ) {
        acnts.emplace( user, [&]( auto& a ){
          a.balance = balance;
          a.fee_index_voted = fee_index_voted;
        });
    } else {
        addvote(ext_sym, acnt->fee_index_voted, -balance.quantity.amount );
        index.modify( acnt, user, [&]( auto& a ){
          a.fee_index_voted = fee_index_voted;
        });
    }
    addvote(ext_sym, fee_index_voted, balance.quantity.amount);
}

void wesetyourfee::closevote(name user, extended_symbol ext_sym) {
    require_auth(user);
    feeaccounts acnts( get_self(), user.value );
    auto index = acnts.get_index<"extended"_n>();
    auto acnt = index.find( make128key(ext_sym.get_contract().value,
      ext_sym.get_symbol().code().raw() ) );
    check( acnt != index.end(), "user is not voting" );
    addvote( ext_sym, acnt->fee_index_voted, -(acnt->balance).quantity.amount );
    index.erase(acnt);
}

void wesetyourfee::openfeetable(name user, extended_symbol ext_sym) {
    feetables tables( get_self(), ext_sym.get_contract().value );
    auto table = tables.find( ext_sym.get_symbol().code().raw() );
    check( table == tables.end(), "already opened" );
    vector<int64_t> zeros(9);
    tables.emplace( user, [&]( auto& a ){
      a.sym = ext_sym.get_symbol();  
      a.votes = zeros;
    });
}

void wesetyourfee::addvote(extended_symbol ext_sym, int fee_index, int64_t amount) {
    feetables tables( get_self(), ext_sym.get_contract().value );
    auto table = tables.find( ext_sym.get_symbol().code().raw());
    check( table != tables.end(), "fee table nonexistent, run openfeetable" );
    tables.modify(table, ""_n, [&]( auto& a ){
      a.votes.at(fee_index) += amount;
      print("Cantidad de votos para fee ", fee_vector.at(fee_index), " es: ", a.votes.at(fee_index), "\n");
    });
    check( (table->votes).at(fee_index) >= 0, "negative number of votes, there is a bug");
}