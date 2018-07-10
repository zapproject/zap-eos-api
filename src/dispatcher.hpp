#include <eosiolib/eosio.hpp>
#include <string>

using namespace eosio;

class Dispatcher: public eosio::contract {
    public:
 	using contract::contract;

        //@abi action
        void query(account_name from, std::string endpoint);

        //@abi action
        void respond(account_name provider, uint64_t id, std::string query);

	//@abi action
	void queries();

        struct user_query {
            uint64_t id;
            account_name user;
            std::string endpoint;
            bool executed;

            uint64_t primary_key() const { return id; }

            EOSLIB_SERIALIZE(user_query, (id)(user)(endpoint)(executed))
        };

        typedef multi_index<N(user_query), user_query> queryIndex;
};

EOSIO_ABI(Dispatcher, (query)(respond)(queries))
