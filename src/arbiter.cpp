#include <arbiter.hpp>

//TODO: must be changed for prod
#define PERIOD_FOR_DOT 60

void subscribe(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    eosio_assert(dosts > 0, "Dots number must be bigger than zero.");

    db::subscriptionIndex subscriptions(_self, provider);

    auto sub_hash_index = subscriptions.get_index<N(byhash)>();
    auto sub_iterator = sub_hash_index.find(db::hash(subscriber, endpoint));

    eosio_assert(sub_iterator == sub_hash_index.end(), "Already subscribed.");

    
}

void unsubscribe(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots, bool from_sub) {
}
