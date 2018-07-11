#include <registry.hpp>

void Registry::newprovider(account_name provider, std::string title, uint64_t key) {
    require_auth(provider);

    Registry::providerIndex providers(_self, _self);

    print_f("new provider: user = %, title = %, public_key = %", provider, title, key);

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

    Registry::providerIndex providers(_self, _self);
    Registry::endpointIndex* endpoints = new Registry::endpointIndex(_self, provider);

    print_f("endpoint for provider: %\t", provider);
    print_f("endpoint specifier: %\t", specifier);

    // Check that provider exists
    auto iterator = providers.find(provider);
    eosio_assert(iterator != providers.end(), "Provider not found!");

    key256 hash = Registry::hash(provider, specifier);

    print_f("Hash = % %", hash.data()[0], hash.data()[1]);

    // Check that provider doesn't have this specifier 
    eosio_assert(Registry::validateEndpoint(*endpoints, provider, specifier), "Endpoint already exists!");

   
    // Save endpoint to multi index storage
    endpoints->emplace(provider, [&](auto& newEndpoint) {
        newEndpoint.id = endpoints->available_primary_key();
	newEndpoint.provider = provider;
	newEndpoint.specifier = specifier;
	newEndpoint.constants = constants;
	newEndpoint.parts = parts;
        newEndpoint.dividers = dividers;
        newEndpoint.hash = hash;
    });
    
     print_f("Hash after save = % % size = %", hash.data()[0], hash.data()[1], sizeof(hash.data()));

    delete endpoints;
}

void Registry::viewps(uint64_t from, uint64_t to) {
    eosio_assert(from <= to, "'from' value must be lower or equal than 'to' value.");

    Registry::providerIndex providers(_self, _self);

    auto iterator = providers.begin();
    uint64_t counter = 0;
    while (iterator != providers.end() && counter <= (to - from)) {
        auto item = *iterator;
        print_f("provider #%: user = %, public_key = %, title = %;", counter, item.user, item.key, item.title);
        iterator++;
        counter++;
    }
}

void Registry::viewes(account_name provider, uint64_t from, uint64_t to) {
    Registry::endpointIndex endpoints(_self, provider);

    auto endpointsIterator = endpoints.begin();
    uint64_t counter = 0;
    while (endpointsIterator != endpoints.end() && counter <= (to - from)) {
        auto item = *endpointsIterator;
        std::string constants = Registry::vector_to_string(item.constants);
        std::string parts = Registry::vector_to_string(item.parts);
        std::string dividers = Registry::vector_to_string(item.dividers);
        print_f("endpoint #%: provider = %, specifier = %, constants = %, parts = %, dividers = %, hash = % %;\n", counter, item.provider, item.specifier, constants, parts, 
            dividers, item.hash.data()[0], item.hash.data()[1]);
        counter++;
        endpointsIterator++;
   }  
}

void Registry::hashview(account_name provider, std::string specifier) {
     Registry::endpointIndex endpoints(_self, provider);
     auto idx = endpoints.get_index<N(byhash)>();
     key256 hash = Registry::hash(provider, specifier);
     auto hashItr = idx.lower_bound(hash);
     auto item = endpoints.get(hashItr->id);
     std::string constants = Registry::vector_to_string(item.constants);
     std::string parts = Registry::vector_to_string(item.parts);
     std::string dividers = Registry::vector_to_string(item.dividers);
     print_f("endpoint: provider = %, specifier = %, constants = %, parts = %, dividers = %;\n", item.provider, item.specifier, constants, parts, dividers);
}



