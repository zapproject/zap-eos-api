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
            uint64_t public_key;

            uint64_t primary_key() const { return user; }

            EOSLIB_SERIALIZE(provider, (user)(title)(public_key))
        };

	
	// REGISTER METHODS

        //@abi action
        void newprovider(Registry::provider p);

        //@abi action
        void addendpoint(Registry::endpoint e);


	// GETTERS

	//@abi action
	std::vector<Registry::provider> getproviders(uint64_t from, uint64_t to);

	//@abi action
	std::vector<Registry::endpoint> getendpoints(account_name provider, uint64_t from, uint64_t to);

	//@abi action
	Registry::provider getprovider(account_name provider);

	//@abi action
	Registry::endpoint getendpoint(account_name provider, std::string endpoint_specifier);

        typedef multi_index<N(provider), provider> providerIndex;
	typedef multi_index<N(endpoint), endpoint,
				indexed_by<N(byprovider), const_mem_fun<endpoint, uint64_t, &endpoint::get_provider>>> endpointIndex;
};

EOSIO_ABI(Registry, (newprovider)(addendpoint))
