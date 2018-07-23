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

void Registry::addendpoint(account_name provider, std::string specifier, std::vector<int64_t> constants, std::vector<uint64_t> parts, std::vector<uint64_t> dividers) { 
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
	newEndpoint.constants = constants;
	newEndpoint.parts = parts;
        newEndpoint.dividers = dividers;
        newEndpoint.issued = 0;
    });
}

void Registry::viewps(uint64_t from, uint64_t to) {
    eosio_assert(from <= to, "'from' value must be lower or equal than 'to' value.");

    db::providerIndex providers(_self, _self);

    auto iterator = providers.begin();
    uint64_t counter = 0;
    while (iterator != providers.end() && counter <= (to - from)) {
        auto item = *iterator;
        print_f("provider #%: user = %, public_key = %, title = %;", counter, name{item.user}, item.key, item.title);
        iterator++;
        counter++;
    }
}

void Registry::viewes(account_name provider, uint64_t from, uint64_t to) {
    db::endpointIndex endpoints(_self, provider);

    auto endpointsIterator = endpoints.begin();
    uint64_t counter = 0;
    while (endpointsIterator != endpoints.end() && counter <= (to - from)) {
        auto item = *endpointsIterator;
        std::string constants = Registry::vector_to_string(item.constants);
        std::string parts = Registry::vector_to_string(item.parts);
        std::string dividers = Registry::vector_to_string(item.dividers);
        print_f("endpoint #%: provider = %, specifier = %, constants = %, parts = %, dividers = %, issued = %.\n", counter, name{item.provider}, item.specifier, constants, parts, dividers, item.issued);

        counter++;
        endpointsIterator++;
   }  
}

// EXPERIMENTAL FEATURE
// CURRENTLY IT WORKS, BUT IT MUST BE TESTED
void Registry::endpbyhash(account_name provider, std::string specifier) {
     db::endpointIndex endpoints(_self, provider);

     auto idx = endpoints.get_index<N(byhash)>();
     key256 hash = key256(db::hash(provider, specifier));
     auto hashItr = idx.find(hash);
     auto item = endpoints.get(hashItr->id);

     std::string constants = Registry::vector_to_string(item.constants);
     std::string parts = Registry::vector_to_string(item.parts);
     std::string dividers = Registry::vector_to_string(item.dividers);
     print_f("endpoint: provider = %, specifier = %, constants = %, parts = %, dividers = %, issued = %.\n", name{item.provider}, item.specifier, constants, parts, dividers, item.issued);
}



