#include "database.hpp"

using namespace eosio;

class Dispatcher {
    public:
 	Dispatcher(name n): _self(n), queries(n, n.value) { }

        //Query provider data
        void query(name subscriber, name provider, std::string endpoint, std::string query, bool onchain_provider, bool onchain_subscriber);

        //Respond to query
        void respond(name responder, uint64_t id, std::string params);

        //Buy subscription to provider endpoint
        void subscribe(name subscriber, name provider, std::string endpoint, uint64_t dots);

        //Remove subscription
        void unsubscribe(name subscriber, name provider, std::string endpoint, bool from_sub);

    private:
        name _self;   
        db::queryIndex queries;

        //TODO: must be changed to prod account
        const name zap_token = "zap.token"_n;

        //Convert specified amount of tokens to <asset> structure
        eosio::asset to_asset(uint64_t tokensAmount) {
            return asset(tokensAmount, symbol(symbol_code(ZAP_TOKEN_SYMBOL), ZAP_TOKEN_DECIMALS));
        }
    
        void transfer_tokens(name from, name to, uint64_t amount, std::string memo) {
            action(
                permission_level{ from, "active"_n },
                zap_token, "transfer"_n,
                std::make_tuple(from, to, to_asset(amount), memo)
            ).send();
        }


        bool delete_query(db::queryIndex &queries, uint64_t id) {       
            auto iterator = queries.find(id);
            if (iterator != queries.end()) {
                queries.erase(iterator);
                return true;
            } else {
                return false;
            }
        }

        uint64_t get_bound_dots(name subscriber, name provider, std::string endpoint) {
            db::holderIndex holders(_self, subscriber.value);
            
            auto hidx = holders.get_index<"byhash"_n>();
            auto holders_iterator = hidx.find(db::hash(provider, endpoint));
            
            if (holders_iterator != hidx.end()) {
                return holders_iterator->dots;
            } else {
                return 0;
            }
        }

        uint64_t update_issued(db::issuedIndex &issued, name payer, uint64_t endpoint_id, int64_t dots) {
            auto issued_iterator = issued.find(endpoint_id);
            uint64_t total_issued_dots = 0;
            if (issued_iterator != issued.end()) {
                total_issued_dots = issued_iterator->dots;
                issued.modify(issued_iterator, payer, [&](auto& i) {
                    i.dots = i.dots + dots;
                });
                print_f("Issued updated, added value = %.\n", dots);
            } else {
                issued.emplace(payer, [&](auto& i) {
                    i.endpointid = endpoint_id;
                    i.dots = dots;
                });
                print_f("New issued created, endpointid = %; dots = %.\n", endpoint_id, dots);
            }
            return total_issued_dots;
        }
    
        auto update_holder(db::holderIndex &holders, name payer, name provider, std::string endpoint, int64_t dots, int64_t escrow) {
            auto holders_index = holders.get_index<"byhash"_n>();
            auto holders_iterator = holders_index.find(db::hash(provider, endpoint));
            if (holders_iterator != holders_index.end()) {
                holders_index.modify(holders_iterator, payer, [&](auto& h) {
                    h.dots = h.dots + dots;
                    h.escrow = h.escrow + escrow;
                });
                print_f("Holder updated, added value = %.\n", dots);
            } else {
                holders.emplace(payer, [&](auto& h) {
                    h.provider = provider;
                    h.endpoint = endpoint;
                    h.dots = dots;
                    h.escrow = escrow;
                });
                print_f("New holder created, provider = %; endpoint = %; dots = %.\n", name{provider}, endpoint.c_str(), dots);
            }
            return holders_iterator;
        }

        //Escrow dots to specified endpoint
        //Can be called only by dispatch provider
        //User can not withdraw dots from escrow
        void escrow(name subscriber, name provider, std::string endpoint, uint64_t dots) {
            db::holderIndex holders(_self, subscriber.value);

            auto idx = holders.get_index<"byhash"_n>();
            auto holders_iterator = idx.find(db::hash(provider, endpoint));
        
            eosio_assert(holders_iterator != idx.end(), "Holder not found.");
            eosio_assert(holders_iterator->dots >= dots, "Not enough dots.");
         
            // Remove specified amount of dots and add them to subscriber escrow
            update_holder(holders, subscriber, provider, endpoint, -dots, dots);
            print_f("Escrow updated, escrow dots added = %.\n", dots);
        }    

        //Convert escrowed dots of subscriber to zap tokens and send tokens to provider
        //Escrow dots will be removed
        //Provider will receive zap tokens for escrowed dots
        //Dots will be removed from total issued of endpoint
        //Dots will be removed from subscriber holder
        void release(name subscriber, name provider, std::string endpoint, uint64_t dots) {
            db::holderIndex holders(_self, subscriber.value);
            db::endpointIndex endpoints(_self, provider.value);
            db::issuedIndex issued(_self, provider.value);
        
            auto endpoints_index = endpoints.get_index<"byhash"_n>();
            auto endpoints_iterator = endpoints_index.find(db::hash(provider, endpoint));
            auto issued_iterator = issued.find(endpoints_iterator->id);

            auto holders_index = holders.get_index<"byhash"_n>();
            auto holders_iterator = holders_index.find(db::hash(provider, endpoint));

            // Check that subscriber have bonded dots and his escrow dots are bigger or equal to dots for release
            eosio_assert(issued_iterator != issued.end(), "Issued dots not found.");
            eosio_assert(holders_iterator != holders_index.end(), "Holder not found.");
            eosio_assert(holders_iterator->escrow >= dots, "Not enough escrow dots.");
 
            // Remove specified amount of dots from subscriber escrow
            update_holder(holders, subscriber, provider, endpoint, 0, -dots);
            update_holder(holders, provider, provider, endpoint, 0, dots);
            update_issued(issued, subscriber, issued_iterator->endpointid, -dots);
        }    

};

