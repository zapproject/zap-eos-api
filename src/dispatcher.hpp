#include <database.hpp>

using namespace eosio;

class Dispatcher {
    public:
 	Dispatcher(account_name n): _self(n) { }

        //Query provider data
        void query(account_name subscriber, account_name provider, std::string endpoint, std::string query, bool onchain_provider, bool onchain_subscriber);

        //Respond to query
        void respond(account_name responder, uint64_t id, std::string params);

    private:
        account_name _self;   

        bool delete_query(uint64_t id) {
            db::queryIndex queries(_self, _self);

            auto iterator = queries.find(id);
            if (iterator != queries.end()) {
                queries.erase(iterator);
                return true;
            } else {
                return false;
            }
        }
        
        uint64_t get_bound_dots(account_name subscriber, account_name provider, std::string endpoint) {
            db::holderIndex holders(_self, subscriber);
            
            key256 hash = key256(db::hash(provider, endpoint));
            auto hidx = holders.get_index<N(byhash)>();
            auto holders_iterator = hidx.find(hash);
            
            if (holders_iterator != hidx.end()) {
                return holders_iterator->dots;
            } else {
                return 0;
            }
        }

        //Escrow dots to specified endpoint
        //Can be called only by dispatch provider
        //User can not withdraw dots from escrow
        void escrow(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
            db::holderIndex holders(_self, subscriber);

            key256 hash = key256(db::hash(provider, endpoint));
            auto idx = holders.get_index<N(byhash)>();
            auto holders_iterator = idx.find(hash);
        
            eosio_assert(holders_iterator != idx.end(), "Holder not found.");
            eosio_assert(holders_iterator->dots >= dots, "Not enough dots.");
         
            // Remove specified amount of dots and add them to subscriber escrow
            idx.modify(holders_iterator, subscriber, [&] (auto& h) {
                h.dots = h.dots - dots;
                h.escrow = h.escrow + dots;
            });   
            print_f("Escrow updated, escrow dots added = %.\n", dots);
        }    

        //Convert escrowed dots of subscriber to zap tokens and send tokens to provider
        //Escrow dots will be removed
        //Provider will receive zap tokens for escrowed dots
        //Dots will be removed from total issued of endpoint
        //Dots will be removed from subscriber holder
        void release(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
            db::holderIndex subscriber_holders(_self, subscriber);
            db::endpointIndex endpoints(_self, provider);
            db::issuedIndex issued(_self, provider);
        
            key256 hash = key256(db::hash(provider, endpoint));
            auto e_idx = endpoints.get_index<N(byhash)>();
            auto endpoints_iterator = e_idx.find(hash);
            auto issued_iterator = issued.find(endpoints_iterator->id);

            auto s_idx = subscriber_holders.get_index<N(byhash)>();
            auto subscriber_iterator = s_idx.find(hash);

            // Check that subscriber have bonded dots and his escrow dots are bigger or equal to dots for release
            eosio_assert(issued_iterator != issued.end(), "Issued dots not found.");
            eosio_assert(subscriber_iterator != s_idx.end(), "Holder not found.");
            eosio_assert(subscriber_iterator->escrow >= dots, "Not enough escrow dots.");

            // Calculate amount of zap tokens that provider will receive
            uint64_t price = Bondage::get_withdraw_price(provider, endpoint, issued_iterator->dots, dots);

            // Send tokens to provider
            db::transfer_tokens(_self, provider, price, "release");
            print_f("Escrow updated, escrow dots removed = %, zap tokens transfered = %.\n", dots, price);
 
            // Remove specified amount of dots from subscriber escrow
            s_idx.modify(subscriber_iterator, subscriber, [&] (auto& h) {
                h.escrow = h.escrow - dots;
            });   

            issued.modify(issued_iterator, subscriber, [&](auto& i) {
                i.dots = i.dots - dots;
            });
            print_f("Bond: issued updated, substructed value = %.\n", dots);
        }    

};

