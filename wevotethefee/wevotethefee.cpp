#include "wevotethefee.hpp"

int wevotethefee::median(symbol_code pair_token){
    feetables tables( get_self(), pair_token.raw() );
    auto table = tables.find( pair_token.raw());
    check( table != tables.end(), "fee table nonexistent, run openfeetable" );
    auto votes = table->votes;
    vector <int64_t> partial_sum_vec(votes.size());
    partial_sum(votes.begin(), votes.end(), partial_sum_vec.begin());
    int64_t sum = partial_sum_vec.back();
    if (sum == 0) return FEE_VECTOR.at(DEFAULT_FEE_INDEX);
    auto it = lower_bound(partial_sum_vec.begin(), partial_sum_vec.end(), sum / 2);
    auto index = it - partial_sum_vec.begin();
    return FEE_VECTOR.at(index);
}

void wevotethefee::updatefee(symbol_code pair_token) {
    int new_fee = median(pair_token);
    action(permission_level{ get_self(), "active"_n },
      "evolutiondex"_n, "changefee"_n,
      make_tuple( pair_token, new_fee )).send();
}

void wevotethefee::onaddliquidity(name user, asset to_buy, asset max_asset1, asset max_asset2){
    add_balance(user, to_buy, true);
}

void wevotethefee::onremliquidity(name user, asset to_sell, asset min_asset1, asset min_asset2){
    add_balance(user, -to_sell, true);
}

void wevotethefee::ontransfer(const name& from, const name& to, const asset& quantity, const string&  memo ){
    add_balance(from, -quantity, false);
    add_balance(to, quantity, true);
};

asset wevotethefee::bring_balance(name user, symbol_code pair_token) {
    accounts table( "evolutiondex"_n, user.value );
    const auto& user_balance = table.find( pair_token.raw() );
    check ( user_balance != table.end(), "pair_token balance does not exist" );
    return user_balance->balance;
}

void wevotethefee::votefee(name user, symbol_code pair_token, int fee_voted){
    check( (FEE_VECTOR.at(MIN_FEE_INDEX) <= fee_voted) && 
           (fee_voted <= FEE_VECTOR.at(MAX_FEE_INDEX)), "only values between 10 and 100 are allowed");
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
        addvote(pair_token, acnt->fee_index_voted, -balance.amount, false);
        acnts.modify( acnt, user, [&]( auto& a ){
          a.fee_index_voted = fee_index_voted;
        });
    }
    addvote(pair_token, fee_index_voted, balance.amount, true);
}

void wevotethefee::closevote(name user, symbol_code pair_token) {
    require_auth(user);
    feeaccounts acnts( get_self(), user.value );
    auto acnt = acnts.find( pair_token.raw());
    check( acnt != acnts.end(), "user is not voting" );
    auto balance = bring_balance(user, pair_token);    
    addvote( pair_token, acnt->fee_index_voted, -balance.amount, true );
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
    vector<int64_t> zeros( FEE_VECTOR.size());
    tables.emplace( user, [&]( auto& a ){
      a.pair_token = pair_token;  
      a.votes = zeros;
    });
}

void wevotethefee::add_balance(name user, asset to_add, bool need_update) {
    feeaccounts acnts( get_self(), user.value );
    auto acnt = acnts.find( to_add.symbol.code().raw());
    if (acnt != acnts.end()) {
        addvote( acnt->pair_token, acnt->fee_index_voted, to_add.amount, need_update);
    }
}

void wevotethefee::addvote(symbol_code pair_token, int fee_index, int64_t amount,
  bool need_update) {
    feetables tables( get_self(), pair_token.raw() );
    auto table = tables.find( pair_token.raw());
    check( table != tables.end(), "fee table nonexistent, run openfeetable" );
    tables.modify(table, same_payer, [&]( auto& a ){
      a.votes.at(fee_index) += amount;
    });
    if (need_update) updatefee( pair_token );
}

int wevotethefee::get_index(int fee_value){
    auto it = lower_bound(FEE_VECTOR.begin(), FEE_VECTOR.end(), fee_value);
    return it - FEE_VECTOR.begin();
}