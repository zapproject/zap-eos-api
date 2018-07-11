#pragma once

#include <eosiolib/eosio.hpp>
#include <registry.hpp>
#include <string>
#include <vector>

using namespace eosio;

class Bondage: public eosio::contract {
    public:
        using contract::contract;
        
        struct holder {
            account_name provider;
            std::string endpoint;
            uint256_t sha256hash; // hash to be able to find holder without loop
            uint64_t dots;

            uint64_t primary_key() const { return provider; }
            uint256_t get_hash() const { return sha256hash; }

            EOSLIB_SERIALIZE(endpoint, (provider)(endpoint)(sha256hash)(dots))
        }
        
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
        void viewe(account_name holder, uint256_t hash);

        //@abi action
        //View all endpoints for specified holder
        void viewh(account_name holder);

     private:
         uint64_t calc_dots_price(account_name provider, std::string endpoint_specifier, uint64_t dots) {
             return 0;             
         }

         uint256_t calc_holder_hash(account_name provider, std::string endpoint_specifier) {
             return 0;
         }

         Registry::endpoint get_endpoint(account_name provider, std::string endpoint_specifier) {
             return Registry::endpoint();
         }

};

EOSIO_ABI(Bondage, (bond)(unbond)(estimate)(viewe)(viewh))
