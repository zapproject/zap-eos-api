#include "main.hpp"


void main::newprovider(name provider, std::string title, uint64_t public_key) {
    main::registry.newprovider(provider, title, public_key);
}

void main::addendpoint(name provider, std::string specifier, std::vector <int64_t> functions, name broker) {
    main::registry.addendpoint(provider, specifier, functions, broker);
}

void main::bond(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    main::bondage.bond(subscriber, provider, endpoint, dots);
}

void main::unbond(name subscriber, name provider, std::string endpoint, uint64_t dots) {
    main::bondage.unbond(subscriber, provider, endpoint, dots);
}

void main::estimate(name provider, std::string endpoint, uint64_t dots) {
    main::bondage.estimate(provider, endpoint, dots);
}

void main::query(name subscriber, name provider, std::string endpoint, std::string query,
                 bool onchain_provider, bool onchain_subscriber, uint128_t timestamp) {
    main::dispatcher.query(subscriber, provider, endpoint, query, onchain_provider, onchain_subscriber, timestamp);
}

void main::respond(name responder, uint64_t id, std::string params, name subscriber) {
    main::dispatcher.respond(responder, id, params, subscriber);
}

void main::subscribe(name subscriber, name provider, std::string endpoint, uint64_t dots, std::string params) {
    main::dispatcher.subscribe(subscriber, provider, endpoint, dots, params);
}

void main::unsubscribe(name subscriber, name provider, std::string endpoint, bool from_sub) {
    main::dispatcher.unsubscribe(subscriber, provider, endpoint, from_sub);
}

void main::setparams(name provider, std::string specifier, std::vector<std::string> params) {
    main::registry.setparams(provider, specifier, params);
}

void main::cancelquery(name subscriber, uint64_t query_id) {
    main::dispatcher.cancelquery(subscriber, query_id);
}

void main::create(name issuer, asset maximum_supply) {
    main::embToken.create(issuer, maximum_supply);
}

void main::issue(name to, asset quantity, string memo) {
    main::embToken.issue(to, quantity, memo);
}

void main::mint(name to, asset quantity) {
    main::embToken.mint(to, quantity);
}

void main::burn(name from, asset quantity) {
    main::embToken.burn(from, quantity);
}

void main::retire(asset quantity, string memo) {
    main::embToken.retire(quantity, memo);
}

void main::transfer(name from, name to, asset quantity, string memo) {
    main::embToken.transfer(from, to, quantity, memo);
}

void main::open(name owner, const symbol &symbol, name ram_payer) {
    main::embToken.open(owner, symbol, ram_payer);
}

void main::close(name owner, const symbol &symbol) {
    main::embToken.close(owner, symbol);
}

void main::tdinit(name provider, std::string specifier, std::vector<int64_t> functions, asset maximum_supply) {
    main::tdFactory.td_init(main::registry, provider, specifier, functions, maximum_supply);
}

void main::tdbond(name issuer, name provider, std::string specifier, uint64_t dots) {
    main::tdFactory.td_bond(main::bondage, issuer, provider, specifier, dots);
}

void main::tdunbond(name issuer, name provider, std::string specifier, uint64_t dots) {
    main::tdFactory.td_unbond(main::bondage, issuer, provider, specifier, dots);
}

void main::cinit(name provider, uint64_t finish, name oracle, std::vector<db::endp> endpoints) {
    main::contest.c_init(main::registry, provider, finish, oracle, endpoints);
}

void main::cjudge(uint64_t contest_id, name provider, name oracle, std::string winner, uint64_t win_value) {
    main::contest.c_judge(contest_id, provider, oracle, winner, win_value);    
}

void main::csettle(name provider, uint64_t contest_id) {
    main::contest.c_settle(main::bondage, provider, contest_id);
}

void main::cbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots) {
    main::contest.c_bond(main::bondage, issuer, provider, contest_id, specifier, dots);
}

void main::cunbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots) {
    main::contest.c_unbond(main::bondage, issuer, provider, contest_id, specifier, dots);
}

EOSIO_DISPATCH(main, (newprovider)(addendpoint)(bond)(unbond)(estimate)(query)(respond)(subscribe)(unsubscribe)(setparams)(cancelquery)(create)(issue)(transfer)(open)(close)(retire)(burn)(tdinit)(tdbond)(tdunbond)(cinit)(cjudge)(csettle)(cbond)(cunbond))

