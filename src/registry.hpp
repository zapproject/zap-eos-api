#pragma once

#include <database.hpp>

using namespace eosio;

class Registry {
    public:

        Registry(account_name n): _self(n) { }

        // REGISTER METHODS

        //@abi action
        //Create new provider
        //<provider> param must be valid account and action sender must have permissions for this acc
        void newprovider(account_name provider, std::string title, uint64_t public_key);

        //@abi action
        //Add new endpoint for provider
        //<provider> param must be valid account and action sender must have permissions for this acc
        void addendpoint(account_name provider, std::string specifier, std::vector<int64_t> constants, std::vector<uint64_t> parts, std::vector<uint64_t> dividers);

	
        // VIEW METHODS

        //@abi action
        //View all provider in specified bounds
        //<from> - start index
        //<to> - end index
        void viewps(uint64_t from, uint64_t to);

        //@abi action
        //View all endpoints for specified provider in specified bounds
        //<from> - start index
        //<to> - end index
        void viewes(account_name provider, uint64_t from, uint64_t to);

        //@abi action
        //View params of specified endpoint
        void endpbyhash(account_name provider, std::string specifier);

    private:        
        account_name _self;

	std::string vector_to_string(std::vector<uint64_t> v) {
            std::string str = "[";
            
            uint64_t counter = 0;
            for (auto const& item: v) {
                if (counter > 0) {
                    str += ", ";	
                }
	
                str += std::to_string(item);
                counter++;
            }
            
            str += "]";
            return str;
        }

        std::string vector_to_string(std::vector<int64_t> v) {
            std::string str = "[";
            
            uint64_t counter = 0;
            for (auto const& item: v) {
                if (counter > 0) {
                    str += ", ";	
                }
	
                str += std::to_string(item);
                counter++;
            }
            
            str += "]";
            return str;
        }

        // TODO: should be refactored to optimize search
        bool validateEndpoint(const db::endpointIndex &idx, account_name provider, std::string specifier) {
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
