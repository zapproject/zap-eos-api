#include <registry.hpp>

void Registry::newprovider(Registry::provider p) {
    require_auth(p.user);

    Registry::providerIndex providers(_self, _self);

    print_f("new provider: user = %, title = %, public_key = %", p.user, p.title, p.public_key);

    providers.emplace(p.user, [&](auto& newProvider) {
        newProvider.user = p.user;
        newProvider.title = p.title;
        newProvider.public_key = p.public_key;
    });
}

void Registry::addendpoint(Registry::endpoint e) { 
    require_auth(e.provider);

    Registry::providerIndex providers(_self, _self);
    Registry::endpointIndex endpoints(_self, _self);

    print_f("endpoint for provider: %", e.provider);
    print_f("endpoint specifier: %", e.specifier);

    auto iterator = providers.find(e.provider);
    eosio_assert(iterator != providers.end(), "Provider not found!");

    auto providerEndpoints = endpoints.get_index<N(byprovider)>();
    for(const auto& item : providerEndpoints) {
        eosio_assert(item.specifier != e.specifier, "Endpoint is already exists!");
    }

    endpoints.emplace(e.provider, [&](auto& newEndpoint) {
        newEndpoint.id = endpoints.available_primary_key();
	newEndpoint.provider = e.provider;
	newEndpoint.specifier = e.specifier;
	newEndpoint.constants = e.constants;
	newEndpoint.parts = e.parts;
        newEndpoint.dividers = e.dividers;
    });
}

std::vector<Registry::provider> Registry::getproviders(uint64_t from, uint64_t to) {
    return std::vector<Registry::provider>();
}

std::vector<Registry::endpoint> Registry::getendpoints(account_name provider, uint64_t from, uint64_t to) {
    return std::vector<Registry::endpoint>();
}

Registry::provider Registry::getprovider(account_name provider) {
    return Registry::provider();
}

Registry::endpoint Registry::getendpoint(account_name provider, std::string endpoint_specifier) {
    return Registry::endpoint();
}


