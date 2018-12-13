#include "registry.hpp"

void Registry::newprovider(account_name provider, std::string title, uint64_t key) {
    require_auth(provider);

    db::providerIndex providers(_self, _self);

    print_f("new provider: user = %, title = %, public_key = %", name{provider}, title, key);

    // Save new provider
    // Will throw exception if provider exists
    providers.emplace(provider, [&](auto& newProvider) {
        newProvider.user = provider;
        newProvider.title = title;
        newProvider.key = key;
    });
}

void Registry::addendpoint(account_name provider, std::string specifier, std::vector<int64_t> functions, account_name broker) {
    require_auth(provider);

    db::providerIndex providers(_self, _self);
    db::endpointIndex endpoints(_self, provider);

    // Check that provider exists
    auto iterator = providers.find(provider);
    eosio_assert(iterator != providers.end(), "Provider not found!");

    // Check that provider doesn't have this specifier 
    eosio_assert(Registry::validateEndpoint(endpoints, provider, specifier), "Endpoint already exists!");


    // Save endpoint to multi index storage
    endpoints.emplace(provider, [&](auto& newEndpoint) {
        newEndpoint.id = endpoints.available_primary_key();
	    newEndpoint.provider = provider;
	    newEndpoint.specifier = specifier;
	    newEndpoint.broker = broker;
	    newEndpoint.functions = functions;
    });
}


void Registry::setparams(account_name provider, std::string specifier, std::vector<std::string> params) {
    require_auth(provider);

    db::paramsIndex endpoint_params(_self, provider);

    auto params_index = endpoint_params.get_index<N(byhash)>();
    auto params_iterator = params_index.find(db::hash(provider, specifier));

    if (params_iterator == params_index.end()) {
        endpoint_params.emplace(provider, [&](auto& p) {
            p.id = endpoint_params.available_primary_key();
            p.provider = provider;
            p.endpoint = specifier;
            p.values = params;
        });
    } else {
        params_index.modify(params_iterator, provider, [&](auto &p) {
            p.values = params;
        });
    }
}

