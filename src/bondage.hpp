#pragma once

#include <database.hpp>
#include <math.h>

using namespace eosio;

class Bondage: public eosio::contract {
    public:
        using contract::contract;
        
        // MAIN METHODS

        //@abi action
        void bond(account_name token, account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        void unbond(account_name token, account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        void estimate(account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        void init(account_name registry);

        
        // VIEW METHODS

        //@abi action
        //View specified endpoint dots for specified holder
        void viewhe(account_name holder, account_name provider, std::string endpoint);

        //@abi action
        //View all endpoints for specified holder
        void viewh(account_name holder);
        
        //@abi action
        void registry();

    private:
        const std::string ZAP_TOKEN_SYMBOL = "TST";
        account_name zap_registry = N(zap.registry);

        eosio::asset toAsset(uint64_t tokensAmount) {
            return asset(tokensAmount, symbol_type(string_to_symbol(0, "TST")));
        }

        uint64_t get_dots_price(account_name provider, std::string endpoint, uint64_t total_issued, uint64_t dots_to_buy) {
            uint64_t price = 0;
            for (uint64_t i = 0; i < dots_to_buy; i++) {
                uint64_t current_dot = i + total_issued + 1;
                price += calc_dot_price(provider, endpoint, current_dot);
            }
            
            return price;
        }

        uint64_t calc_dot_price(account_name provider, std::string endpoint_specifier, uint64_t dot) {
            db::endpoint endpoint = get_endpoint(provider, endpoint_specifier);
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

        uint64_t fast_log2(int64_t x) {
            uint64_t ix = (uint64_t&)x;
            uint64_t exp = (ix >> 23) & 0xFF;
            uint64_t log2 = uint64_t(exp) - 127;

            return log2;
        }

        db::endpoint get_endpoint(account_name provider, std::string endpoint_specifier) {
            db::endpointIndex endpoints(Bondage::zap_registry, provider);

            auto idx = endpoints.get_index<N(byhash)>();
            key256 hash = key256(db::hash(provider, endpoint_specifier));
            auto hashItr = idx.find(hash);
            auto item = endpoints.get(hashItr->id);
            return item;
        }

        void make_payment(account_name from, uint64_t amount) {
            action(
                permission_level{ from, N(active) },
                token, N(transfer),
                std::make_tuple(from, _self, Bondage::toAsset(amount), std::string("Zap dots bought."))
            ).send();
        }
};

EOSIO_ABI(Bondage, (bond)(unbond)(estimate)(viewhe)(viewh)(init)(registry))
