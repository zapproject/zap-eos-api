#include <bondage.hpp>
#include <eosiolib/action.hpp>

void Bondage::init(account_name registry) {
    require_auth(_self);
    Bondage::zap_registry = registry;
    Bondage::zap_dispatch = dispatch;
}

void Bondage::bond(account_name token, account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    db::holderIndex holders(_self, subscriber);
    db::endpointIndex endpoints(Bondage::zap_registry, provider);
    db::issuedIndex issued(_self, provider);

    auto hidx = holders.get_index<N(byhash)>();
    key256 hash = key256(db::hash(provider, endpoint));
    auto holders_iterator = hidx.find(hash);

    auto eidx = endpoints.get_index<N(byhash)>();
    key256 hash = key256(db::hash(provider, endpoint));
    auto endpoints_iterator = eidx.find(hash);
    
    // Check that endpoint exists
    eosio_assert(endpoints_iterator != eidx.end(), "Endpoint doesn't exists.");
    auto endpoint_item = endpoints.get(endpoints_iterator->id);
    print_f("Bond: enndpoint item found, id = %.", endpoint_item.id);

    // Update total issued dots for current endpoint
    uint64_t endpoint_id = endpoint_item.id;
    auto issued_iterator = issued.find(endpoint_id);
    uint64_t total_issued_dots = 0;
    if (issued_iterator != issued.end()) {
        total_issued_dots = issued_iterator->dots;
        issued.modify(issued_iterator, subscriber, [&](auto& i) {
            i.dots = i.dots + dots;
        });
        print_f("Bond: issued updated, added value = %.\n", dots);
    } else {
        issued.emplace(subscriber, [&](auto& i) {
            i.endpointid = endpoint_id;
            i.dots = dots;
        });
        print_f("Bond: new issued created, endpointid = %; dots = %.\n", endpoint_id, dots);
    }

    // Calculate amount of zap tokens that user will pay for dots
    uint64_t price = Bondage::get_dots_price(provider, endpoint, total_issued_dots, dots);

    // Transfer subscriber tokens to zap.bondage address
    Bondage::take_tokens(token, subscriber, price);
    print_f("Bond: transfer tokens action have sent, price is %.\n", price);

    // Update subsciber dots balance for current endpoint
    if (holders_iterator != hidx.end()) {
        holders.modify(holders_iterator, subscriber, [&](auto& h) {
            h.dots = h.dots + dots;
        });
        print_f("Bond: dots updated, added value = %.\n", dots);
    } else {
        holders.emplace(subscriber, [&](auto& h) {
            h.provider = provider;
            h.endpoint = endpoint;
            h.dots = dots;
            h.escrow = 0;
        });
        print_f("Bond: new holder created, provider = %; endpoint = %; dots = %.\n", name{provider}, endpoint, dots);
    }
}

void Bondage::unbond(account_name token, account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    db::holderIndex holders(_self, subscriber);
    db::endpointIndex endpoints(Bondage::zap_registry, provider);
    db::issuedIndex issued(_self, provider);

    key256 hash = key256(db::hash(provider, endpoint));
    auto hidx = holders.get_index<N(byhash)>();
    auto holders_iterator = hidx.find(hash);

    auto eidx = endpoints.get_index<N(byhash)>();
    auto endpoints_iterator = eidx.find(hash);
    
    
    // Check that subscriber can unbond dots
    eosio_assert(endpoints_iterator != eidx.end(), "Endpoint doesn't exists.");
    eosio_assert(holders_iterator != holders.end(), "Holder doesn't exists.");

    auto issued_iterator = issued.find(endpoints_iterator->id);
    eosio_assert(issued_iterator != issued.end(), "Holder doesn't exists.");
    eosio_assert(issued_iterator->dots >= dots, "Endpoint haven't got enough dots for withdraw.");
    eosio_assert(holders_iterator->dots >= dots, "Holder haven't got enough dots for withdraw.");

    // Update total issued dots for current endpoint
    uint64_t endpoint_id = endpoints_iterator->id;
    uint64_t total_issued_dots = issued_iterator->dots;
    issued.modify(issued_iterator, subscriber, [&](auto& i) {
        i.dots = i.dots - dots;
    });
    print_f("Bond: issued updated, substructed value = %.\n", dots);
    
    // Update subsciber dots balance for current endpoint
    holders.modify(holders_iterator, subscriber, [&](auto& h) {
        h.dots = h.dots - dots;
    });
    print_f("Bond: dots updated, added value = %.\n", dots);
 

    // Calculate amount of zap tokens that user will pay for dots
    uint64_t price = Bondage::get_withdraw_price(provider, endpoint, total_issued_dots, dots);

    // Transfer subscriber tokens to zap.bondage address
    Bondage::withdraw_tokens(token, subscriber, price);
    print_f("Bond: transfer tokens action have sent, price is %.\n", price);
}

void Bondage::estimate(account_name provider, std::string endpoint, uint64_t dots) {
    db::endpointIndex endpoints(Bondage::zap_registry, provider);
    db::issuedIndex issued(_self, provider);

    auto idx = endpoints.get_index<N(byhash)>();
    key256 hash = key256(db::hash(provider, endpoint));
    auto endpoints_iterator = idx.find(hash);

    if (endpoints_iterator == idx.end()) {
        print_f("Estimate: endpoint with provider '%' and specifier '%' not found.\n", name{provider}, endpoint);
        return;
    }

    auto endpoint_item = endpoints.get(endpoints_iterator->id);

    auto issued_iterator = issued.find(endpoint_item.id);
    uint64_t total_issued_dots = 0;
    if (issued_iterator != issued.end()) {
        total_issued_dots = issued_iterator->dots;
    }
    
    uint64_t price = Bondage::get_dots_price(provider, endpoint, total_issued_dots, dots);
    print_f("Estimated price for provider = '%' and endpoint = '%' is % ZAP.", name{provider}, endpoint, price);
}

void Bondage::escrow(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(Bondage::zap_dispatch);

    db::holderIndex holders(_self, subscriber);

    auto idx = holders.get_index<N(byhash)>();
    key256 hash = key256(db::hash(provider, endpoint));
    auto holders_iterator = idx.find(hash);

    eosio_assert(holders_iterator != idx.end(), "Holder not found.");
    eosio_assert(holders_iterator->dots >= dots, "Not enough dots.");
 
    holders.modify(holders_iterator, subscriber, [&] (auto& h) {
        h.dots = h.dots - dots;
        h.escrow = h.escrow + dots;
    });   
    print_f("Escrow updated, escrow dots added = %.", dots);
}

void Bondage::release(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);
}

void Bondage::viewhe(account_name holder, account_name provider, std::string endpoint) {
    db::holderIndex holders(_self, holder);
    
    auto idx = holders.get_index<N(byhash)>();
    key256 hash = key256(db::hash(provider, endpoint));
    auto hashItr = idx.find(hash);
    auto item = holders.get(hashItr->provider);

    print_f("Holder %: provider = %; endpoint = %; dots = %; escrow = %.", name{holder}, name{item.provider}, item.endpoint, item.dots, item.escrow);
}

void Bondage::viewh(account_name holder) {
     db::holderIndex holders(_self, holder);
     
     auto iterator = holders.begin();
     uint64_t counter = 0;
     while (iterator != holders.end()) {
         print_f("#% - provider = %; endpoint = %; dots = %; escrow = %.", counter, name{iterator->provider}, iterator->endpoint, iterator->dots, iterator->escrow);
         counter++;
         iterator++;
     }
}

void Bondage::registry() {
    require_auth(_self);
    print_f("Registry acc name = %.", name{Bondage::zap_registry});
}











