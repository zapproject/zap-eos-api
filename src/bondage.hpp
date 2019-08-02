#pragma once

#include "database.hpp"
#include <math.h>

using namespace eosio;

class Bondage {
public:

    Bondage(name n) : _self(n) {}

    // MAIN METHODS

    //Buy dots for specified endpoint
    void bond(name subscriber, name provider, std::string endpoint, uint64_t dots);

    void noauth_bond(name subscriber, name provider, std::string endpoint, uint64_t dots, name dotsPayer);

    //Withdraw dots for specified provider
    void unbond(name subscriber, name provider, std::string endpoint, uint64_t dots);

    void noauth_unbond(name subscriber, name provider, std::string endpoint, uint64_t dots, name dotsPayer);

    //Estimate price of dots (in integral zap tokens) for specified provider
    void estimate(name provider, std::string endpoint, uint64_t dots);


    //Calc price of dots for bonding
    static int64_t get_dots_price(db::endpoint endpoint, uint64_t total_issued, uint64_t dots_to_buy) {
        int64_t price = 0;
        for (uint64_t i = 0; i < dots_to_buy; i++) {
            int64_t current_dot = i + total_issued + 1;
            price += calc_dot_price(endpoint, current_dot);
        }

        return price;
    }

    //Calc price of dots for withdrawing
    static int64_t get_withdraw_price(db::endpoint endpoint, uint64_t total_issued, uint64_t dots_to_withdraw) {
        int64_t price = 0;
        for (uint64_t i = 0; i < dots_to_withdraw; i++) {
            int64_t current_dot = total_issued - i;
            price += calc_dot_price(endpoint, current_dot);
        }

        return price;
    }

private:
    name _self;

    //TODO: must be changed to prod account
    const name zap_token = "eosio.token"_n;

    //Convert specified amount of tokens to <asset> structure
    eosio::asset to_asset(uint64_t tokensAmount) {
        return asset(tokensAmount, symbol(symbol_code(ZAP_TOKEN_SYMBOL), ZAP_TOKEN_DECIMALS));
    }

    void transfer_tokens(name from, name to, uint64_t amount, std::string memo) {
        action(
                permission_level{from, "active"_n},
                zap_token, "transfer"_n,
                std::make_tuple(from, to, to_asset(amount), memo)
        ).send();
    }

    static int64_t get_dots_limit(db::endpoint endpoint) {
        return endpoint.functions[endpoint.functions.size() - 1];
    }

    //Calculate price of specified dot number for specified endpoint
    static int64_t calc_dot_price(db::endpoint endpoint, uint64_t dot) {
        uint64_t index = 0;
        while(index < endpoint.functions.size()){
            int64_t len = endpoint.functions[index];
            int64_t end = endpoint.functions[index + len + 1];

            if(dot > end){
                // move onto the next piece
                index += len + 2;
                continue;
            }

            // calculate at this piece
            int64_t sum = 0;
            for(uint64_t i = 0; i < len; i++){
                int64_t coeff = endpoint.functions[index + i + 1];
                sum += coeff * int64_t(pow(dot, i));
            }
            return sum;
        }
        return -1;
    }

    /**
     * Function update total issued dots for endpoint
     *
     * @param issued table that contains issued dots for endpoint
     * @param payer user that will pay for storage
     * @param endpoint_id id of endpoint
     * @param dots dots to add
     * @return issued dot before update
     */
    uint64_t update_issued(db::issuedIndex &issued, name payer, uint64_t endpoint_id, int64_t dots) {
        auto issued_iterator = issued.find(endpoint_id);
        uint64_t issued_dots_before_update = 0;
        if (issued_iterator != issued.end()) {
            issued_dots_before_update = issued_iterator->dots;
            issued.modify(issued_iterator, payer, [&](auto &i) {
                i.dots = i.dots + dots;
            });
            print_f("Issued updated, added value = %.\n", dots);
        } else {
            issued.emplace(payer, [&](auto &i) {
                i.endpointid = endpoint_id;
                i.dots = dots;
            });
            print_f("New issued created, endpointid = %; dots = %.\n", endpoint_id, dots);
        }
        return issued_dots_before_update;
    }

    auto update_holder(db::holderIndex &holders, name payer, name provider, std::string endpoint,
                       int64_t dots, int64_t escrow) {
        auto holders_index = holders.get_index<"byhash"_n>();
        auto holders_iterator = holders_index.find(db::hash(provider, endpoint));
        if (holders_iterator != holders_index.end()) {
            holders_index.modify(holders_iterator, payer, [&](auto &h) {
                h.dots = h.dots + dots;
                h.escrow = h.escrow + escrow;
            });
            print_f("Holder updated, added value = %.\n", dots);
        } else {
            holders.emplace(payer, [&](auto &h) {
                h.id = holders.available_primary_key();
                h.provider = provider;
                h.endpoint = endpoint;
                h.dots = dots;
                h.escrow = escrow;
            });
            print_f("New holder created, provider = %; endpoint = %; dots = %.\n", name{provider}, endpoint.c_str(), dots);
        }
        return holders_iterator;
    }

};

//EOSIO_ABI(Bondage, (bond)(unbond)(estimate)(escrow)(release)(viewhe)(viewh)(viewi))
