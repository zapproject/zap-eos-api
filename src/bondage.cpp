#include <bondage.hpp>
#include <eosiolib/action.hpp>

void Bondage::bond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    db::holderIndex* holders = new db::holderIndex(_self, subscriber);
    db::endpointIndex* endpoints = new db::endpointIndex(_self, provider);
    db::issuedIndex* issued = new db::issuedIndex(_self, provider);

    auto endpoint_index = endpoints->get_index<N(byhash)>();
    auto endpoint_iterator = endpoint_index.find(db::hash(provider, endpoint));
    eosio_assert(endpoint_iterator != endpoint_index.end(), "Endpoint not found.");
    print_f("Endpoint item found, id = %.\n", endpoint_iterator->id);
 

    // Update total issued dots for current endpoint
    uint64_t endpoint_id = endpoint_iterator->id;
    uint64_t total_issued_dots = db::update_issued(*issued, subscriber, endpoint_id, dots);

    // Calculate amount of zap tokens that user will pay for dots
    uint64_t price = Bondage::get_dots_price(endpoints->get(endpoint_id), total_issued_dots, dots);

    // Transfer subscriber tokens to zap.bondage address
    db::transfer_tokens(subscriber, _self, price, "bond");
    print_f("Transfer tokens action have sent, price is %.\n", price);

    // Update subsciber dots balance for current endpoint
    db::update_holder(*holders, subscriber, provider, endpoint, dots, 0);

    delete holders;
    delete endpoints;
    delete issued;
}

void Bondage::unbond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    db::holderIndex* holders = new db::holderIndex(_self, subscriber);
    db::endpointIndex* endpoints = new db::endpointIndex(_self, provider);
    db::issuedIndex* issued = new db::issuedIndex(_self, provider);

    auto holders_index = holders->get_index<N(byhash)>();
    auto holders_iterator = holders_index.find(db::hash(provider, endpoint));
    eosio_assert(holders_iterator != holders_index.end(), "Holder doesn't exists.");
    
    // Check that subscriber can unbond dots
    auto endpoint_index = endpoints->get_index<N(byhash)>();
    auto endpoints_iterator = endpoint_index.find(db::hash(provider, endpoint));
    eosio_assert(endpoints_iterator != endpoint_index.end(), "Endpoint doesn't exists.");

    auto issued_iterator = issued->find(endpoints_iterator->id);
    eosio_assert(issued_iterator != issued->end(), "Holder doesn't exists.");
    eosio_assert(issued_iterator->dots >= dots, "Endpoint haven't got enough dots for withdraw.");
    eosio_assert(holders_iterator->dots >= dots, "Holder haven't got enough dots for withdraw.");

    // Update total issued dots for current endpoint
    uint64_t endpoint_id = endpoints_iterator->id;
    uint64_t total_issued_dots = db::update_issued(*issued, subscriber, endpoint_id, dots);
    
    // Update subsciber dots balance for current endpoint
    db::update_holder(*holders, subscriber, provider, endpoint, dots, 0);
 
    // Calculate amount of zap tokens that user will pay for dots
    uint64_t price = Bondage::get_withdraw_price(endpoints->get(endpoint_id), total_issued_dots, dots);

    // Transfer subscriber tokens to zap.bondage address
    db::transfer_tokens(_self, subscriber, price, "unbond");
    print_f("Transfer tokens action have sent, price is %.\n", price);

    delete holders;
    delete endpoints;
    delete issued;
}

void Bondage::estimate(account_name provider, std::string endpoint, uint64_t dots) {
    db::endpointIndex* endpoints = new db::endpointIndex(_self, provider);
    db::issuedIndex* issued = new db::issuedIndex(_self, provider);

    auto endpoint_index = endpoints->get_index<N(byhash)>();
    auto endpoint_iterator = endpoint_index.find(db::hash(provider, endpoint));

    auto issued_iterator = issued->find(endpoint_iterator->id);
    uint64_t total_issued_dots = 0;
    if (issued_iterator != issued->end()) {
        total_issued_dots = issued_iterator->dots;
    }
    
    uint64_t price = Bondage::get_dots_price(endpoints->get(endpoint_iterator->id), total_issued_dots, dots);
    print_f("Estimated price for provider = '%' and endpoint = '%' is % ZAP.\n", name{provider}, endpoint, price);

    delete endpoints;
    delete issued;
}

void Bondage::viewhe(account_name holder, account_name provider, std::string endpoint) {
    db::holderIndex* holders = new db::holderIndex(_self, holder);
    
    auto idx = holders->get_index<N(byhash)>();
    auto hashItr = idx.find(db::hash(provider, endpoint));
    auto item = holders->get(hashItr->provider);

    print_f("Holder %: provider = %; endpoint = %; dots = %; escrow = %.\n", name{holder}, name{item.provider}, item.endpoint, item.dots, item.escrow);

    delete holders;
}

void Bondage::viewh(account_name holder) {
     db::holderIndex* holders = new db::holderIndex(_self, holder);
     
     auto iterator = holders->begin();
     uint64_t counter = 0;
     while (iterator != holders->end()) {
         print_f("#% - provider = %; endpoint = %; dots = %; escrow = %.\n", counter, name{iterator->provider}, iterator->endpoint, iterator->dots, iterator->escrow);
         counter++;
         iterator++;
     }

     delete holders;
}

void Bondage::viewi(account_name provider, std::string endpoint) {
    db::endpointIndex* endpoints = new db::endpointIndex(_self, provider);
    db::issuedIndex* issued = new db::issuedIndex(_self, provider);

    auto endpoint_index = endpoints->get_index<N(byhash)>();
    auto endpoint_iterator = endpoint_index.find(db::hash(provider, endpoint));
    auto issued_iterator = issued->find(endpoint_iterator->id);

    eosio_assert(issued_iterator != issued->end(), "Issued dots not found.");

    print_f("Total dots for %/% = %.\n", name{provider}, endpoint, issued_iterator->dots);

    delete endpoints;
    delete issued;
}







