#pragma once

#include "database.hpp"
#include <math.h>

using namespace eosio;

class Bondage {
public:

    Bondage(account_name n) : _self(n) {}

    // MAIN METHODS

    //Buy dots for specified endpoint
    void bond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

    //Withdraw dots for specified provider
    void unbond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

    //Estimate price of dots (in integral zap tokens) for specified provider
    void estimate(account_name provider, std::string endpoint, uint64_t dots);


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
    account_name _self;

    //TODO: must be changed to prod account
    const account_name zap_token = N(zap.token);

    //Convert specified amount of tokens to <asset> structure
    eosio::asset to_asset(uint64_t tokensAmount) {
        return asset(tokensAmount, symbol_type(string_to_symbol(ZAP_TOKEN_DECIMALS, ZAP_TOKEN_SYMBOL)));
    }

    void transfer_tokens(account_name from, account_name to, uint64_t amount, std::string memo) {
        action(
                permission_level{from, N(active)},
                zap_token, N(transfer),
                std::make_tuple(from, to, to_asset(amount), memo)
        ).send();
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

    uint64_t update_issued(db::issuedIndex &issued, account_name payer, uint64_t endpoint_id, int64_t dots) {
        auto issued_iterator = issued.find(endpoint_id);
        uint64_t total_issued_dots = 0;
        if (issued_iterator != issued.end()) {
            total_issued_dots = issued_iterator->dots;
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
        return total_issued_dots;
    }

    auto update_holder(db::holderIndex &holders, account_name payer, account_name provider, std::string endpoint,
                       int64_t dots, int64_t escrow) {
        auto holders_index = holders.get_index<N(byhash)>();
        auto holders_iterator = holders_index.find(db::hash(provider, endpoint));
        if (holders_iterator != holders_index.end()) {
            holders_index.modify(holders_iterator, payer, [&](auto &h) {
                h.dots = h.dots + dots;
                h.escrow = h.escrow + escrow;
            });
            print_f("Holder updated, added value = %.\n", dots);
        } else {
            holders.emplace(payer, [&](auto &h) {
                h.provider = provider;
                h.endpoint = endpoint;
                h.dots = dots;
                h.escrow = escrow;
            });
            print_f("New holder created, provider = %; endpoint = %; dots = %.\n", name{provider}, endpoint, dots);
        }
        return holders_iterator;
    }
};

//EOSIO_ABI(Bondage, (bond)(unbond)(estimate)(escrow)(release)(viewhe)(viewh)(viewi))
