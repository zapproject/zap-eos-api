#include "dispatcher.hpp"
#include "bondage.hpp"

#define QUERY_CALL_PRICE 1
#define DOT_SECONDS 60
#define MIN_SUB_PRICE 1

void Dispatcher::query(name subscriber, name provider, std::string endpoint, std::string query, bool onchain_provider, bool onchain_subscriber, uint128_t timestamp) {
    require_auth(subscriber);

    uint64_t bound = Dispatcher::get_bound_dots(subscriber, provider, endpoint);
    eosio_assert(bound > 0, "Haven't got bonded dots for provider." );

    Dispatcher::escrow(subscriber, provider, endpoint, QUERY_CALL_PRICE);

    if (onchain_provider) {
        require_recipient(provider); 
    } else {
        uint64_t availableKey = queries.available_primary_key();
        queries.emplace(subscriber, [&](auto& q) {
            q.id = availableKey;
            q.provider = provider;
            q.subscriber = subscriber;
            q.endpoint = endpoint;
            q.data = query;
            q.onchain = onchain_subscriber;
            q.timestamp = timestamp;
        });

        print_f("Query called: id = %, sub = %, provider = %, endpoint = %;", availableKey, name{subscriber}, name{provider}, endpoint.c_str());
    }   
}

// Don't need to use different function for different params, json can be passed
void Dispatcher::respond(name responder, uint64_t id, std::string params, name subscriber) {
    require_auth(responder);

    auto q = queries.find(id);

    eosio_assert(q != queries.end(), "Query fullfilled or doesn't exists.");
    eosio_assert(q->provider == responder, "Only query provider can respond to query.");  

    if (q->onchain) {
        action(
            permission_level{ responder, "active"_n },
            q->subscriber, "callback"_n,
            std::make_tuple(params)
        ).send();
    } else {
        //TODO: implement event for offchain subscriber
    }

    Dispatcher::release(responder, q->subscriber, q->provider, q->endpoint, QUERY_CALL_PRICE);

    bool deleted = Dispatcher::delete_query(queries, q->id);
    if (deleted) {
        print_f("Query responded successfully, id = %", q->id);
    } else {
        print_f("Query responded with error while deleting, id = %", q->id);
    }
}

void Dispatcher::subscribe(name subscriber, name provider, std::string endpoint, uint64_t dots, std::string params) {
    require_auth(subscriber);

    eosio_assert(dots > 0, "Dots number must be bigger than zero.");

    db::subscriptionIndex subscriptions(_self, provider.value);

    auto sub_hash_index = subscriptions.get_index<"byhash"_n>();
    auto sub_iterator = sub_hash_index.find(db::hash(subscriber, endpoint));

    eosio_assert(sub_iterator == sub_hash_index.end(), "Already subscribed.");

    Dispatcher::escrow(subscriber, provider, endpoint, dots);

    uint64_t start = now();
    uint64_t end = start + (dots * DOT_SECONDS);

    subscriptions.emplace(subscriber, [&] (auto& s) {
        s.id = subscriptions.available_primary_key();
        s.price = dots;
        s.start = start;
        s.end = end;
        s.subscriber = subscriber;
        s.endpoint = endpoint;
    });
}

void Dispatcher::unsubscribe(name subscriber, name provider, std::string endpoint, bool from_sub) {
    if (from_sub) {
        require_auth(subscriber);
    } else {
        require_auth(provider);
    }

    db::subscriptionIndex subscriptions(_self, provider.value);

    auto sub_hash_index = subscriptions.get_index<"byhash"_n>();
    auto sub_iterator = sub_hash_index.find(db::hash(subscriber, endpoint));

    eosio_assert(sub_iterator != sub_hash_index.end(), "Can not found subscription.");
  
    uint64_t current_uint64_t = now();
    if (current_uint64_t < sub_iterator->end) {
        uint64_t passed = current_uint64_t - sub_iterator->start;
        uint64_t dots_used = passed / DOT_SECONDS;
        if (dots_used <= 0) {
            dots_used = MIN_SUB_PRICE;
        }
        Dispatcher::release(from_sub ? subscriber : provider, subscriber, provider, endpoint, dots_used);
    } else {
        Dispatcher::release(from_sub ? subscriber : provider, subscriber, provider, endpoint, sub_iterator->price);
    }

    auto main_iterator = subscriptions.find(sub_iterator->id);
    subscriptions.erase(main_iterator);
}

void Dispatcher::cancelquery(name subscriber, uint64_t query_id) {
    require_auth(subscriber);

    db::holderIndex holders(_self, subscriber.value);

    auto query_iterator = queries.find(query_id);

    eosio_assert(query_iterator != queries.end(), "Query not found.");

    // return dots to user
    Dispatcher::update_holder(holders, subscriber, query_iterator->provider, query_iterator->endpoint, QUERY_CALL_PRICE, -QUERY_CALL_PRICE);

    // remove query call
    bool deleted = Dispatcher::delete_query(queries, query_iterator->id);
    if (deleted) {
        print_f("Query responded successfully, id = %", query_iterator->id);
    } else {
        print_f("Query responded with error while deleting, id = %", query_iterator->id);
    }
}







