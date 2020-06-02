#include "badtoken.hpp"

void badtoken::transfer( const name& from, const name& to,
  const asset& quantity, const string& memo )
{
    require_recipient( "evolutiondex"_n );
}

void badtoken::ontransfer ( const name& from, const name& to, const asset& quantity, const string& memo )
{
    check(false, "notification received");
}

void badtoken::addliquidity(name user, asset to_buy, asset max_asset1, asset max_asset2){
    check(false, "notification received");
}

void badtoken::remliquidity(name user, asset to_sell, asset min_asset1, asset min_asset2){
    check(false, "notification received");
}