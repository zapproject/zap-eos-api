#include "main.hpp"


void Main::newprovider(name provider, std::string title, uint64_t public_key) {
    Main::registry.newprovider(provider, title, public_key);
}

void Main::addendpoint(name provider, std::string specifier, std::vector <int64_t> functions, name broker) {
    Main::registry.addendpoint(provider, specifier, functions, broker);
}

void Main::bond(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    Main::bondage.bond(subscriber, provider, endpoint, dots);
}

void Main::unbond(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    Main::bondage.unbond(subscriber, provider, endpoint, dots);
}

void Main::estimate(name provider, std::string endpoint, uint64_t dots) {
    Main::bondage.estimate(provider, endpoint, dots);
}

void Main::query(name subscriber, name provider, std::string endpoint, std::string query,
                 bool onchain_provider, bool onchain_subscriber) {
    Main::dispatcher.query(subscriber, provider, endpoint, query, onchain_provider, onchain_subscriber);
}

void Main::respond(name responder, uint64_t id, std::string params) {
    Main::dispatcher.respond(responder, id, params);
}

void Main::subscribe(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    Main::dispatcher.subscribe(subscriber, provider, endpoint, dots);
}

void Main::unsubscribe(name subscriber, name provider, std::string endpoint, bool from_sub) {
    Main::dispatcher.unsubscribe(subscriber, provider, endpoint, from_sub);
}
