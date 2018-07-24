#pragma once

#include <database.hpp>

using namespace eosio;

class Registry: public eosio::contract {
    public:
        using contract::contract;

        // REGISTER METHODS

        //@abi action
        void newprovider(account_name provider, std::string title, uint64_t public_key);

        //@abi action
        void addendpoint(account_name provider, std::string specifier, std::vector<int64_t> constants, std::vector<uint64_t> parts, std::vector<uint64_t> dividers);

	
        // VIEW METHODS

        //@abi action
        void viewps(uint64_t from, uint64_t to);

        //@abi action
        void viewes(account_name provider, uint64_t from, uint64_t to);

        //@abi action
        void endpbyhash(account_name provider, std::string specifier);

    private:        
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

EOSIO_ABI(Registry, (newprovider)(addendpoint)(viewps)(viewes)(endpbyhash))
