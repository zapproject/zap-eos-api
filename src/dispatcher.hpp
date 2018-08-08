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

        uint64_t get_bound_dots(account_name subscriber, account_name provider, std::string endpoint) {
            db::holderIndex holders(_self, subscriber);
            
            auto hidx = holders.get_index<N(byhash)>();
            auto holders_iterator = hidx.find(db::hash(provider, endpoint));
            
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
            db::holderIndex* holders = new db::holderIndex(_self, subscriber);

            auto idx = holders->get_index<N(byhash)>();
            auto holders_iterator = idx.find(db::hash(provider, endpoint));
        
            eosio_assert(holders_iterator != idx.end(), "Holder not found.");
            eosio_assert(holders_iterator->dots >= dots, "Not enough dots.");
         
            // Remove specified amount of dots and add them to subscriber escrow
            db::update_holder(*holders, subscriber, provider, endpoint, -dots, dots);
            print_f("Escrow updated, escrow dots added = %.\n", dots);

            delete holders;
        }    

        //Convert escrowed dots of subscriber to zap tokens and send tokens to provider
        //Escrow dots will be removed
        //Provider will receive zap tokens for escrowed dots
        //Dots will be removed from total issued of endpoint
        //Dots will be removed from subscriber holder
        void release(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
             db::holderIndex* holders = new db::holderIndex(_self, subscriber);
             db::endpointIndex* endpoints = new db::endpointIndex(_self, provider);
             db::issuedIndex* issued = new db::issuedIndex(_self, provider);
        
            auto endpoints_index = endpoints->get_index<N(byhash)>();
            auto endpoints_iterator = endpoints_index.find(db::hash(provider, endpoint));
            auto issued_iterator = issued->find(endpoints_iterator->id);

            auto holders_index = holders->get_index<N(byhash)>();
            auto holders_iterator = holders_index.find(db::hash(provider, endpoint));

            // Check that subscriber have bonded dots and his escrow dots are bigger or equal to dots for release
            eosio_assert(issued_iterator != issued->end(), "Issued dots not found.");
            eosio_assert(holders_iterator != holders_index.end(), "Holder not found.");
            eosio_assert(holders_iterator->escrow >= dots, "Not enough escrow dots.");

            // Calculate amount of zap tokens that provider will receive
            uint64_t price = Bondage::get_withdraw_price(endpoints->get(endpoints_iterator->id), issued_iterator->dots, dots);

            // Send tokens to provider
            db::transfer_tokens(_self, provider, price, "release");
            print_f("Escrow updated, escrow dots removed = %, zap tokens transfered = %.\n", dots, price);
 
            // Remove specified amount of dots from subscriber escrow
            db::update_holder(*holders, subscriber, provider, endpoint, 0, -dots);
            db::update_issued(*issued, subscriber, issued_iterator->endpointid, -dots);

            delete holders;
            delete endpoints;
            delete issued;
        }    

};

