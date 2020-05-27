#include "badtoken.hpp"

void badtoken::transfer( const name&    from,
                      const name&    to,
                      const asset&   quantity,
                      const string&  memo )
{
    require_recipient( "evolutiondex"_n );
}
