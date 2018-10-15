#include "bondage.hpp"
#include <eosiolib/action.hpp>

void Bondage::bond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    db::holderIndex holders(_self, subscriber);
    db::endpointIndex endpoints(_self, provider);
    db::issuedIndex issued(_self, provider);

    auto endpoint_index = endpoints.get_index<N(byhash)>();
    auto endpoint_iterator = endpoint_index.find(db::hash(provider, endpoint));
    eosio_assert(endpoint_iterator != endpoint_index.end(), "Endpoint not found.");
    print_f("Endpoint item found, id = %.\n", endpoint_iterator->id);

    if (endpoint_iterator->broker != 0) {
        eosio_assert(endpoint_iterator->broker == subscriber, "Only broker can bond to this endpoint.");
    }
 

    // Update total issued dots for current endpoint
    uint64_t endpoint_id = endpoint_iterator->id;
    uint64_t total_issued_dots = Bondage::update_issued(issued, subscriber, endpoint_id, dots);

    // Calculate amount of zap tokens that user will pay for dots
    uint64_t price = Bondage::get_dots_price(endpoints.get(endpoint_id), total_issued_dots, dots);

    // Transfer subscriber tokens to zap.bondage address
    transfer_tokens(subscriber, _self, price, "bond");
    print_f("Transfer tokens action have sent, price is %.\n", price);

    // Update subsciber dots balance for current endpoint
    Bondage::update_holder(holders, subscriber, provider, endpoint, dots, 0);
}

void Bondage::unbond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    db::holderIndex holders(_self, subscriber);
    db::endpointIndex endpoints(_self, provider);
    db::issuedIndex issued(_self, provider);

    auto holders_index = holders.get_index<N(byhash)>();
    auto holders_iterator = holders_index.find(db::hash(provider, endpoint));
    eosio_assert(holders_iterator != holders_index.end(), "Holder doesn't exists.");
    
    // Check that subscriber can unbond dots
    auto endpoint_index = endpoints.get_index<N(byhash)>();
    auto endpoints_iterator = endpoint_index.find(db::hash(provider, endpoint));
    eosio_assert(endpoints_iterator != endpoint_index.end(), "Endpoint doesn't exists.");

    if (endpoints_iterator->broker != 0) {
        eosio_assert(endpoints_iterator->broker == subscriber, "Only broker can unbond from this endpoint.");
    }

    auto issued_iterator = issued.find(endpoints_iterator->id);
    eosio_assert(issued_iterator != issued.end(), "Holder doesn't exists.");
    eosio_assert(issued_iterator->dots >= dots, "Endpoint haven't got enough dots for withdraw.");
    eosio_assert(holders_iterator->dots >= dots, "Holder haven't got enough dots for withdraw.");

    // Update total issued dots for current endpoint
    uint64_t endpoint_id = endpoints_iterator->id;
    uint64_t total_issued_dots = Bondage::update_issued(issued, subscriber, endpoint_id, -dots);
    
    // Update subsciber dots balance for current endpoint
    Bondage::update_holder(holders, subscriber, provider, endpoint, -dots, 0);
 
    // Calculate amount of zap tokens that user will pay for dots
    uint64_t price = Bondage::get_withdraw_price(endpoints.get(endpoint_id), total_issued_dots, dots);

    // Transfer subscriber tokens to zap.bondage address
    transfer_tokens(_self, subscriber, price, "unbond");
    print_f("Transfer tokens action have sent, price is %.\n", price);
}

void Bondage::estimate(account_name provider, std::string endpoint, uint64_t dots) {
    db::endpointIndex endpoints(_self, provider);
    db::issuedIndex issued(_self, provider);

    auto endpoint_index = endpoints.get_index<N(byhash)>();
    auto endpoint_iterator = endpoint_index.find(db::hash(provider, endpoint));

    auto issued_iterator = issued.find(endpoint_iterator->id);
    uint64_t total_issued_dots = 0;
    if (issued_iterator != issued.end()) {
        total_issued_dots = issued_iterator->dots;
    }
    
    uint64_t price = Bondage::get_dots_price(endpoints.get(endpoint_iterator->id), total_issued_dots, dots);
    print_f("Estimated price for provider = '%' and endpoint = '%' is % ZAP.\n", name{provider}, endpoint, price);
}






