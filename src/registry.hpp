#include <eosiolib/eosio.hpp>
#include <string>
#include <vector>

using namespace eosio;

class Registry: public eosio::contract {
    public:
        using contract::contract;

	//@abi table endpoint i64
        struct endpoint {
            uint64_t id;
            account_name provider;
            std::string specifier;
            std::vector<int64_t> constants;
            std::vector<uint64_t> parts;
            std::vector<uint64_t> dividers;

            uint64_t primary_key() const { return id; }
            uint64_t get_provider() const { return provider; }

            EOSLIB_SERIALIZE(endpoint, (id)(provider)(specifier)(constants)(parts)(dividers))
	};

	//@abi table provider i64
        struct provider {
            account_name user;
            std::string title;
            uint64_t key;

            uint64_t primary_key() const { return user; }

            EOSLIB_SERIALIZE(provider, (user)(title)(key))
        };

	
        // REGISTER METHODS

        //@abi action
        void newprovider(account_name provider, std::string title, uint64_t public_key);

        //@abi action
        void addendpoint(account_name provider, std::string specifier, std::vector<int64_t> constants, std::vector<uint64_t> parts, std::vector<uint64_t> dividers);

	
        // VIEW METHODS

        //@abi action
        void viewps(uint64_t from, uint64_t to);

        //@abi action
        void viewes(uint64_t from, uint64_t to);


        typedef multi_index<N(provider), provider> providerIndex;
        typedef multi_index<N(endpoint), endpoint,
				indexed_by<N(byprovider), const_mem_fun<endpoint, uint64_t, &endpoint::get_provider>>> endpointIndex;

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
        bool validateEndpoint(const Registry::endpointIndex &idx, account_name provider, std::string specifier) {
            auto iterator = idx.begin();
            while (iterator != idx.end()) {
                auto item = *iterator;
                if (item.specifier == specifier && item.provider == provider) return false;
                iterator++;
            }
            return true;
        }
	    
};

EOSIO_ABI(Registry, (newprovider)(addendpoint)(viewps)(viewes))
