#include "main.hpp"


void main::newprovider(name provider, std::string title, uint64_t public_key) {
    main::registry.newprovider(provider, title, public_key);
}

void main::addendpoint(name provider, std::string specifier, std::vector <int64_t> functions, name broker) {
    main::registry.addendpoint(provider, specifier, functions, broker);
}

void main::bond(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    main::bondage.bond(subscriber, provider, endpoint, dots);
}

void main::unbond(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    main::bondage.unbond(subscriber, provider, endpoint, dots);
}

void main::estimate(name provider, std::string endpoint, uint64_t dots) {
    main::bondage.estimate(provider, endpoint, dots);
}

void main::query(name subscriber, name provider, std::string endpoint, std::string query,
                 bool onchain_provider, bool onchain_subscriber) {
    main::dispatcher.query(subscriber, provider, endpoint, query, onchain_provider, onchain_subscriber);
}

void main::respond(name responder, uint64_t id, std::string params) {
    main::dispatcher.respond(responder, id, params);
}

void main::subscribe(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    main::dispatcher.subscribe(subscriber, provider, endpoint, dots);
}

void main::unsubscribe(name subscriber, name provider, std::string endpoint, bool from_sub) {
    main::dispatcher.unsubscribe(subscriber, provider, endpoint, from_sub);
}

EOSIO_DISPATCH(main, (newprovider)(addendpoint)(bond)(unbond)(estimate)(query)(respond)(subscribe)(unsubscribe))
