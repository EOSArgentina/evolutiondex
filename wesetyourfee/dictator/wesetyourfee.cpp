#include "wesetyourfee.hpp"

void wesetyourfee::changefee(symbol sym, int newfee) {
    action(permission_level{ get_self(), "active"_n }, "evolutiondex"_n, "changefee"_n,
        std::make_tuple( sym, newfee)).send(); 
}
