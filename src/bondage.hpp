#pragma once

#include <registry.hpp>
#include <db.hpp>

using namespace eosio;

class Bondage: public eosio::contract {
    public:
        using contract::contract;
        
        // MAIN METHODS

        //@abi action
        void bond(account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        void unbond(account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        void estimate(account_name provider, std::string endpoint, uint64_t dots);

        
        // VIEW METHODS

        //@abi action
        //View specified endpoint dots for specified holder
        void viewhe(account_name holder, uint256_t hash);

        //@abi action
        //View all endpoints for specified holder
        void viewh(account_name holder);

     private:
         uint64_t calc_dots_price(account_name provider, std::string endpoint_specifier, uint64_t dots) {
             return 0;             
         }

         db::endpoint get_endpoint(account_name provider, std::string endpoint_specifier) {
             db::endpointIndex endpoints(_self, provider);

             auto idx = endpoints.get_index<N(byhash)>();
             key256 hash = key256(db::hash(provider, specifier));
             auto hashItr = idx.find(hash);
             auto item = endpoints.get(hashItr->id);
             return item;
         }

};

EOSIO_ABI(Bondage, (bond)(unbond)(estimate)(viewe)(viewh))
