#include <bondage.hpp>

void Bondage::bond(account_name token, account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    db::holderIndex holders(_self, subscriber);
    db::endpointIndex endpoints(_self, provider);

    auto holders_iterator = holders.find(subscriber);

    auto idx = endpoints.get_index<N(byhash)>();
    key256 hash = key256(db::hash(provider, endpoint));
    auto endpoints_iterator = idx.find(hash);
    
    if (endpoints_iterator == idx.end()) {
        print_f("Bond: endpoint with provider '%' and specifier '%' not found.\n", name{provider}, endpoint);
        return;
    }

    auto endpoint_item = endpoints.get(endpoints_iterator->id);
    print_f("Bond: enndpoint item found, id = %.", endpoint_item.id);
    uint64_t price = Bondage::get_dots_price(provider, endpoint, endpoint_item.issued, dots);

    //SEND_INLINE_ACTION(token, transfer, {provider, N(active)}, {provider, Bondage::_self, price, std::string()} );
    print_f("Bond: transfer tokens action have sent, price is %.\n", price);

    if (holders_iterator != holders.end()) {
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

    auto iterator = endpoints.find(endpoint_item.id);
    endpoints.modify(iterator, subscriber, [&] (auto& e) {
        e.issued += dots;
    });
    print("Bond: total issued updated.\n");
}

void Bondage::unbond(account_name token, account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);
    print_f("Not implemented yet.");
}

void Bondage::estimate(account_name provider, std::string endpoint, uint64_t dots) {
    db::endpointIndex endpoints(_self, provider);

    auto idx = endpoints.get_index<N(byhash)>();
    key256 hash = key256(db::hash(provider, endpoint));
    auto endpoints_iterator = idx.find(hash);

    if (endpoints_iterator == idx.end()) {
        print_f("Estimate: endpoint with provider '%' and specifier '%' not found.\n", name{provider}, endpoint);
        return;
    }

    auto endpoint_item = endpoints.get(endpoints_iterator->id);
    uint64_t price = Bondage::get_dots_price(provider, endpoint, endpoint_item.issued, dots);
    print_f("Estimated price for provider = '%' and endpoint = '%' is % ZAP.", name{provider}, endpoint, price);
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











