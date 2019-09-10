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
    main::tdFactory.td_init(main::embToken, main::registry, provider, specifier, functions, maximum_supply);
}

void main::tdbond(name issuer, name provider, std::string specifier, uint64_t dots) {
    main::tdFactory.td_bond(main::embToken, main::bondage, issuer, provider, specifier, dots);
}

void main::tdunbond(name issuer, name provider, std::string specifier, uint64_t dots) {
    main::tdFactory.td_unbond(main::embToken, main::bondage, issuer, provider, specifier, dots);
}

void main::cinit(name provider, uint64_t finish, name oracle, std::vector<db::endp> endpoints) {
    main::contest.c_init(main::embToken, main::registry, provider, finish, oracle, endpoints);
}

void main::cjudge(uint64_t contest_id, name provider, name oracle, std::string winner, uint64_t win_value) {
    main::contest.c_judge(contest_id, provider, oracle, winner, win_value);    
}

void main::csettle(name provider, uint64_t contest_id) {
    main::contest.c_settle(main::bondage, provider, contest_id);
}

void main::cbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots) {
    main::contest.c_bond(main::embToken, main::bondage, issuer, provider, contest_id, specifier, dots);
}

void main::cunbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots) {
    main::contest.c_unbond(main::embToken, main::bondage, issuer, provider, contest_id, specifier, dots);
}

void main::testrp(name issuer) {
    if (has_auth(issuer)) {
        require_recipient(_self, _self);
        print_f("Require recepient called\n");
        print_f("Self = %\n", name(_self));
        if (has_auth(_self)) {
            print_f("Has self auth\n");
        } else {
            print_f("There is no self auth\n");
        }

        eosio::transaction t;
        // always double check the action name as it will fail silently
        // in the deferred transaction
        t.actions.emplace_back(
            // when sending to _self a different authorization can be used
            // otherwise _self must be used
            permission_level(_self, "active"_n),
            // account the action should be send to
            _self,
            // action to invoke
            "rpcallback"_n,
            // arguments for the action
            std::make_tuple(issuer));

        // set delay in seconds
        t.delay_sec = 5;

        // first argument is a unique sender id
        // second argument is account paying for RAM
        // third argument can specify whether an in-flight transaction
        // with this senderId should be replaced
        // if set to false and this senderId already exists
        // this action will fail
        t.send(now(), issuer /*, false */);
    } else if (has_auth(_self)) {
        print_f("Callback called");
    }
}


void main::rpcallback(name issuer) {
    print_f("Callback called");
    require_auth(_self);
    print_f("Self auth provided");
}

void main::tcallback(name from, name to, asset quantity, std::string memo) {
    if (has_auth(from)) {
        print_f("have from auth");
    }

    if (has_auth(to)) {
        print_f("have to auth");
    }

    if (has_auth(_self)) {
        print_f("have self auth");
    }

    if (has_auth("eosio.token"_n)) {
        print_f("have token auth");
    }
}

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        main _main(name(receiver));

        if (has_auth(receiver)) {
             print_f("apply function have _self auth!!!");
        }
        
        print_f("receiver = %\n", name(receiver));
        print_f("code = %\n", name(code));
        print_f("action = %\n", name(action));
        if (code == receiver) {
            print_f("Called action = %\n", name(action));
            switch (action) {
                EOSIO_DISPATCH_HELPER(main, (newprovider)(addendpoint)(bond)(unbond)(estimate)(query)(respond)(subscribe)(unsubscribe)
                (setparams)(cancelquery)(create)(issue)(transfer)(open)(close)(retire)(burn)(tdinit)(tdbond)(tdunbond)(cinit)
                (cjudge)(csettle)(cbond)(cunbond)(testrp)(rpcallback))
            };
        } else {
            print_f("Handle require_recepient action = %\n", name(action));
            if (action == name("testrp").value) {
                execute_action(name(receiver), name(code), &main::rpcallback);
            }
            if (action == name("transfer").value) {
                execute_action(name(receiver), name(code), &main::tcallback);
            }
        }
        

        eosio_exit(0);
    }
}

//EOSIO_DISPATCH(main, (newprovider)(addendpoint)(bond)(unbond)(estimate)(query)(respond)(subscribe)(unsubscribe)(setparams)(cancelquery)(create)(issue)(transfer)(open)(close)(retire)(burn)(tdinit)(tdbond)(tdunbond)(cinit)(cjudge)(csettle)(cbond)(cunbond))

