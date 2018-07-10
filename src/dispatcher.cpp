#include <dispatcher.hpp>

void Dispatcher::query(account_name from, std::string endpoint) {
    require_auth(from);

    Dispatcher::queryIndex queries(_self, _self);

    uint64_t availableKey = queries.available_primary_key();
    print_f("query to save: id = %, user = %, endpoint = %, executed = false",
        availableKey, from, endpoint);

    queries.emplace(from, [&](auto& q) {
        q.id = availableKey;
        q.user = from;
        q.endpoint = endpoint;
        q.executed = false;
    });
}

void Dispatcher::respond(account_name provider, uint64_t id, std::string query) {
    Dispatcher::queryIndex queries(_self, _self);

    auto iterator = queries.find(id);
    eosio_assert(iterator != queries.end(), "Query not found!");

    queries.modify(iterator, provider, [&](auto& q) {
        q.executed = true;
    });

    print_f("Query with id = % is executed.", id);
}

void Dispatcher::queries() {
    Dispatcher::queryIndex queries(_self, _self);

    uint64_t id = 0;
    while(queries.find(id) != queries.end()) {
	auto q = queries.get(id);
    	print_f("query[%]: user=%, endpoint=%, executed=%\n", q.id, q.user, q.endpoint, q.executed);
	id++;
    }
}


