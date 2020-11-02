#include "wevotethefee.hpp"

int wevotethefee::median(symbol_code pair_token){
    feetables tables( get_self(), pair_token.raw() );
    auto table = tables.find( pair_token.raw());
    check( table != tables.end(), "fee table nonexistent, run openfeetable" );
    auto votes = table->votes;
    vector <int64_t> partial_sum_vec(votes.size());
    partial_sum(votes.begin(), votes.end(), partial_sum_vec.begin());
    int64_t sum = partial_sum_vec.back();
    if (sum == 0) return -1;
    auto it = lower_bound(partial_sum_vec.begin(), partial_sum_vec.end(), sum / 2);
    auto index = it - partial_sum_vec.begin();
    return fee_vector.at(index);
}

void wevotethefee::updatefee(symbol_code pair_token) {
    int new_fee = median(pair_token);
    if (new_fee >= 0) action(permission_level{ get_self(), "active"_n },
      "evolutiondex"_n, "changefee"_n,
      make_tuple( pair_token, new_fee )).send();
}

void wevotethefee::onaddliquidity(name user, asset to_buy, asset max_asset1, asset max_asset2){
    add_balance(user, to_buy);
}

void wevotethefee::onremliquidity(name user, asset to_sell, asset min_asset1, asset min_asset2){
    add_balance(user, -to_sell);
}

void wevotethefee::ontransfer(const name& from, const name& to, const asset& quantity, const string&  memo ){
    add_balance(from, -quantity);
    add_balance(to, quantity);
};

asset wevotethefee::bring_balance(name user, symbol_code pair_token) {
    accounts table( "evolutiondex"_n, user.value );
    const auto& user_balance = table.find( pair_token.raw() );
    check ( user_balance != table.end(), "pair_token balance does not exist" );
    return user_balance->balance;
}

void wevotethefee::votefee(name user, symbol_code pair_token, int fee_voted){
    check( (1 <= fee_voted) && (fee_voted <= 300), "only values between 1 and 300 allowed");
    int fee_index_voted = get_index(fee_voted);
    require_auth(user);
    feeaccounts acnts( get_self(), user.value );
    auto acnt = acnts.find( pair_token.raw());
    auto balance = bring_balance(user, pair_token);
    if ( acnt == acnts.end() ) {
        acnts.emplace( user, [&]( auto& a ){
          a.pair_token = pair_token;
          a.fee_index_voted = fee_index_voted;
        });
    } else {
        addvote(pair_token, acnt->fee_index_voted, -balance.amount );
        acnts.modify( acnt, user, [&]( auto& a ){
          a.fee_index_voted = fee_index_voted;
        });
    }
    addvote(pair_token, fee_index_voted, balance.amount);
    updatefee(pair_token);
}

void wevotethefee::closevote(name user, symbol_code pair_token) {
    require_auth(user);
    feeaccounts acnts( get_self(), user.value );
    auto acnt = acnts.find( pair_token.raw());
    check( acnt != acnts.end(), "user is not voting" );
    auto balance = bring_balance(user, pair_token);    
    addvote( pair_token, acnt->fee_index_voted, -balance.amount );
    acnts.erase(acnt);
}

void wevotethefee::closefeetable(symbol_code pair_token) {
    feetables tables( get_self(), pair_token.raw() );
    auto table = tables.find( pair_token.raw());
    check( table != tables.end(), "table does not exist" );
    auto votes = table->votes;
    auto is_empty = all_of(votes.begin(), votes.end(), 
        [](const int & votes_number){return votes_number == 0;});
    check( is_empty, "table of votes is not empty");
    tables.erase(table);
}

void wevotethefee::openfeetable(name user, symbol_code pair_token) {
    feetables tables( get_self(), pair_token.raw() );
    auto table = tables.find( pair_token.raw());
    check( table == tables.end(), "already opened" );
    vector<int64_t> zeros( fee_vector.size());
    tables.emplace( user, [&]( auto& a ){
      a.pair_token = pair_token;  
      a.votes = zeros;
    });
}

void wevotethefee::add_balance(name user, asset to_add) {
    feeaccounts acnts( get_self(), user.value );
    auto acnt = acnts.find( to_add.symbol.code().raw());
    if (acnt != acnts.end()) {
        addvote( acnt->pair_token, acnt->fee_index_voted, to_add.amount);
        updatefee( to_add.symbol.code() );
    }
}

void wevotethefee::addvote(symbol_code pair_token, int fee_index, int64_t amount) {
    feetables tables( get_self(), pair_token.raw() );
    auto table = tables.find( pair_token.raw());
    check( table != tables.end(), "fee table nonexistent, run openfeetable" );
    tables.modify(table, same_payer, [&]( auto& a ){
      a.votes.at(fee_index) += amount;
    });
}

int wevotethefee::get_index(int fee_value){
    auto it = lower_bound(fee_vector.begin(), fee_vector.end(), fee_value);
    return clamp(it - fee_vector.begin(), 5, 11); 
}