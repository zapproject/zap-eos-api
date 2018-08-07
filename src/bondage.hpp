#pragma once

#include <database.hpp>
#include <math.h>
#include <eosiolib/currency.hpp>

using namespace eosio;

class Bondage {
    public:

        Bondage(account_name n): _self(n) { }
 
        // MAIN METHODS

        //Buy dots for specified endpoint
        void bond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        //Withdraw dots for specified provider
        void unbond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        //Estimate price of dots (in integral zap tokens) for specified provider 
        void estimate(account_name provider, std::string endpoint, uint64_t dots);
        
        // VIEW METHODS

        //View specified endpoint dots for specified holder
        void viewhe(account_name holder, account_name provider, std::string endpoint);

        //View all endpoints for specified holder
        void viewh(account_name holder);
        
        //View total issued dots for specified endpoint
        void viewi(account_name provider, std::string endpoint);


        //Calc price of dots for bonding 
        static uint64_t get_dots_price(db::endpoint endpoint, uint64_t total_issued, uint64_t dots_to_buy) {
            uint64_t price = 0;
            for (uint64_t i = 0; i < dots_to_buy; i++) {
                uint64_t current_dot = i + total_issued + 1;
                price += calc_dot_price(endpoint, current_dot);
            }
            
            return price;
        }

        //Calc price of dots for withdrawing
        static uint64_t get_withdraw_price(db::endpoint endpoint, uint64_t total_issued, uint64_t dots_to_withdraw) {
            uint64_t price = 0;
            for (uint64_t i = 0; i < dots_to_withdraw; i++) {
                uint64_t current_dot = total_issued - i;
                price += calc_dot_price(endpoint, current_dot);
            }
            
            return price;
        }

    private:
        account_name _self;

        //Calculate price of specified dot number for specified endpoint
        static uint64_t calc_dot_price(db::endpoint endpoint, uint64_t dot) {
            for (uint64_t i = 0; i < endpoint.dividers.size(); i++) {
                uint64_t start = endpoint.parts[2 * i];
                if (dot < start) continue;
                
                int64_t sum = 0;
                uint64_t p_start = i == 0 ? 0 : endpoint.dividers[i - 1];
                for (uint64_t j = p_start; j < endpoint.dividers[i]; j++) {
                    // get the components
                    int64_t coef = endpoint.constants[(3 * j)];
                    int64_t power = endpoint.constants[(3 * j) + 1];
                    int64_t fn = endpoint.constants[(3 * j) + 2];

                    // calculate function
                    uint64_t function_applyed_x = dot;
                    switch (fn) {
                        case 1: function_applyed_x = fast_log2(dot); break;
                        default: break;
                    }
                    sum += pow(function_applyed_x, power) * coef;
                }
                 
                return sum;
            }
            
            return 0;             
        }

        static uint64_t fast_log2(int64_t x) {
            uint64_t ix = (uint64_t&)x;
            uint64_t exp = (ix >> 23) & 0xFF;
            uint64_t log2 = uint64_t(exp) - 127;

            return log2;
        }

        //Get endpoint object from registry contract
        db::endpoint get_endpoint(account_name provider, std::string endpoint_specifier) {
            db::endpointIndex endpoints(_self, provider);

            auto idx = endpoints.get_index<N(byhash)>();
            key256 hash = key256(db::hash(provider, endpoint_specifier));
            auto hashItr = idx.find(hash);
            auto item = endpoints.get(hashItr->id);
            return item;
        }
};

//EOSIO_ABI(Bondage, (bond)(unbond)(estimate)(escrow)(release)(viewhe)(viewh)(viewi))
