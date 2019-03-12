#pragma once

#include "database.hpp"

using namespace eosio;

class Registry {
    public:

        Registry(name n): _self(n) { }

        // REGISTER METHODS

        //Create new provider
        //<provider> param must be valid account and action sender must have permissions for this acc
        void newprovider(name provider, std::string title, uint64_t public_key);

        //Add new endpoint for provider
        //<provider> param must be valid account and action sender must have permissions for this acc
        void addendpoint(name provider, std::string specifier, std::vector<int64_t> functions, name broker);

        //Set provider or endpoint params
        void setparams(name provider, std::string specifier, std::vector<std::string> params);

    private:        
        name _self;

        // TODO: should be refactored to optimize search
        bool validateEndpoint(const db::endpointIndex &idx, name provider, std::string specifier) {
            auto iterator = idx.begin();
            while (iterator != idx.end()) {
                auto item = *iterator;
                if (item.specifier == specifier && item.provider == provider) return false;
                iterator++;
            }
            return true;
        }	    
};

//EOSIO_ABI(Registry, (newprovider)(addendpoint)(viewps)(viewes)(endpbyhash))
