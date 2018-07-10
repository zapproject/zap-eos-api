#include <eosiolib/eosio.hpp>
#include <string>
#include <vector>

using namespace eosio;

class Bondage: public eosio::contract {
    public:
        using contract::contract;
        
        struct holder {
            
        }

};

EOSIO_ABI(Bondage, ())
