#include "dispatcher.hpp"
#include "bondage.hpp"

#define QUERY_CALL_PRICE 1
#define DOT_SECONDS 60

void Dispatcher::query(account_name subscriber, account_name provider, std::string endpoint, std::string query, bool onchain_provider, bool onchain_subscriber) {
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
        });

        print_f("Query called: id = %, sub = %, provider = %, endpoint = %;", availableKey, name{subscriber}, name{provider}, endpoint);
    }   
}

void Dispatcher::respond(account_name responder, uint64_t id, std::string params) {
    require_auth(responder);

    auto q = queries.find(id);

    eosio_assert(q != queries.end(), "Query fullfilled or doesn't exists.");
    eosio_assert(q->provider == responder, "Only query provider can respond to query.");  

    if (q->onchain) {
        action(
            permission_level{ responder, N(active) },
            q->subscriber, N(callback),
            std::make_tuple(params)
        ).send();
    } else {
        //TODO: implement event for offchain subscriber
    }

    Dispatcher::release(q->subscriber, q->provider, q->endpoint, QUERY_CALL_PRICE);

    bool deleted = Dispatcher::delete_query(queries, q->id);
    if (deleted) {
        print_f("Query responded successfully, id = %", q->id);
    } else {
        print_f("Query responded with error while deleting, id = %", q->id);
    }
}

void Dispatcher::subscribe(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);

    eosio_assert(dots > 0, "Dots number must be bigger than zero.");

    db::subscriptionIndex subscriptions(_self, provider);

    auto sub_hash_index = subscriptions.get_index<N(byhash)>();
    auto sub_iterator = sub_hash_index.find(db::hash(subscriber, endpoint));

    eosio_assert(sub_iterator == sub_hash_index.end(), "Already subscribed.");

    Dispatcher::escrow(subscriber, provider, endpoint, dots);

    time start = now();
    time end = start + (dots * DOT_SECONDS);

    subscriptions.emplace(subscriber, [&] (auto& s) {
        s.id = subscriptions.available_primary_key();
        s.price = dots;
        s.start = start;
        s.end = end;
        s.subscriber = subscriber;
        s.endpoint = endpoint;
    });
}

void Dispatcher::unsubscribe(account_name subscriber, account_name provider, std::string endpoint, bool from_sub) {
    if (from_sub) {
        require_auth(subscriber);
    } else {
        require_auth(provider);
    }

    db::subscriptionIndex subscriptions(_self, provider);

    auto sub_hash_index = subscriptions.get_index<N(byhash)>();
    auto sub_iterator = sub_hash_index.find(db::hash(subscriber, endpoint));

    eosio_assert(sub_iterator != sub_hash_index.end(), "Can not found subscribtion.");
  
    time current_time = now();
    if (current_time < sub_iterator->end) {
        time passed = current_time - sub_iterator->start;
        uint64_t dots_used = passed / DOT_SECONDS;
    } else {
    
    }
}







