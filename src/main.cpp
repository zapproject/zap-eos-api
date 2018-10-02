#include "main.hpp"


void Main::newprovider(account_name provider, std::string title, uint64_t public_key) {
    Main::registry.newprovider(provider, title, public_key);
}

void Main::addendpoint(account_name provider, std::string specifier, std::vector <uint64_t> functions, account_name broker) {
    Main::registry.addendpoint(provider, specifier, functions, broker);
}

void Main::bond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    Main::bondage.bond(subscriber, provider, endpoint, dots);
}

void Main::unbond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    Main::bondage.unbond(subscriber, provider, endpoint, dots);
}

void Main::estimate(account_name provider, std::string endpoint, uint64_t dots) {
    Main::bondage.estimate(provider, endpoint, dots);
}

void Main::query(account_name subscriber, account_name provider, std::string endpoint, std::string query,
                 bool onchain_provider, bool onchain_subscriber) {
    Main::dispatcher.query(subscriber, provider, endpoint, query, onchain_provider, onchain_subscriber);
}

void Main::respond(account_name responder, uint64_t id, std::string params) {
    Main::dispatcher.respond(responder, id, params);
}

void Main::subscribe(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    Main::dispatcher.subscribe(subscriber, provider, endpoint, dots);
}

void Main::unsubscribe(account_name subscriber, account_name provider, std::string endpoint, bool from_sub) {
    Main::dispatcher.unsubscribe(subscriber, provider, endpoint, from_sub);
}

void Main::viewps(uint64_t from, uint64_t to) {
    Main::registry.viewps(from, to);
}

void Main::viewes(account_name provider, uint64_t from, uint64_t to) {
    Main::registry.viewes(provider, from, to);
}

void Main::endpbyhash(account_name provider, std::string specifier) {
    Main::registry.endpbyhash(provider, specifier);
}

void Main::viewhe(account_name holder, account_name provider, std::string endpoint) {
    Main::bondage.viewhe(holder, provider, endpoint);
}

void Main::viewh(account_name holder) {
    Main::bondage.viewh(holder);
}

void Main::viewi(account_name provider, std::string endpoint) {
    Main::bondage.viewi(provider, endpoint);
}

