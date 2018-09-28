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
    static uint64_t calc_dot_price(db::endpoint endpoint, uint64_t dot) {
        auto function = find_dot_function(endpoint.functions, dot);
        return calculate_function(function, dot);
    }

    std::string static vector_to_string(std::vector <int64_t> v) {
        std::string str = "[";

        uint64_t counter = 0;
        for (auto const &item: v) {
            if (counter > 0) {
                str += ", ";
            }

            str += std::to_string(item);
            counter++;
        }

        str += "]";
        return str;
    }

    static std::vector <int64_t> find_dot_function(std::vector <int64_t> functions, uint64_t dot) {
        uint64_t i = 0;
        while (i < functions.size()) {
            int64_t length = functions[i];
            int64_t right_bound = functions[i + length + 1];

            if (dot <= right_bound) {
                // fetch function without length parameter
                auto result_vector = std::vector<int64_t>(functions.begin() + i + 1,
                                                          functions.begin() + i + length - 1);
                print_f("found function for dot % is: %", dot, vector_to_string(result_vector));
                return result_vector;
            } else {
                i = i + length + 2;
            }
        }

        return std::vector<int64_t>();
    }

    static uint64_t calculate_function(std::vector <int64_t> function, uint64_t dot) {
        int64_t a = function[0];
        int64_t b = function[1];
        int64_t c = function[2];

        return a * (uint64_t(pow(dot, c)));
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
