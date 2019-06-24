#include "contest.hpp"

#define STATUS_INNITIALIZED 0
#define STATUS_JUDGED 1
#define STATUS_SETTELED 2
#define STATUS_CANCELED 3

void Contest::c_init(name provider, uint64_t finish, name oracle, std::vector<db::endp> endpoints) {
    require_auth(provider);

    std::vector<std::string> specifiers;

    // create endpoints and tokens for them
    for (db::endp e: endpoints) {
        // create new endpoint
        registry.addendpoint(provider, specifier, functions, provider);

        // create token for this endpoint
        action(
            permission_level{ _self, "active"_n },
            _self, "create"_n,
            std::make_tuple(_self, maximum_supply)
        ).send();

        // store token symbol for specified curve
        db::ftokenIndex ftokens(_self, provider.value);
        ftokens.emplace(provider, [&](auto& newFtoken) {
            newFtoken.id = ftokens.available_primary_key();
	    newFtoken.provider = provider;
	    newFtoken.endpoint = specifier;
	    newFtoken.symbol = maximum_supply.symbol;
        });

        specifiers.push_back(e.specifier);
    }

    db::contestIndex contests(_self, provider.value);
    contests.emplace(provider, [&](auto& newContest) {
            newContest.id = ftokens.available_primary_key();
	    newContest.provider = provider;
	    newContest.oracle = oracle;
	    newContest.finish = finish;
	    newContest.status = 0;
	    newContest.winner = "";
	    newContest.endpoints = specifiers;
    });
}

void Contest::c_judge(uint64_t contest_id, name oracle, std::string winner, uint64_t win_value) {
    require_auth(oracle);

    // find contest
    db::contestIndex contests(_self, provider.value);
    auto contests_iterator = contests.find(contest_id);
    eosio_assert(contests_iterator != contests.end(), "Contest not found!");

    eosio_assert(contests_iterator->status != STATUS_JUDGED, "Contest is finished or canceled!");
    eosio_assert(contests_iterator->finished <= now_time, "Contest timeout!");

    contests.modify(contests_iterator, oracle, [&](auto &contest) {
        contest.status = STATUS_SETTELED;
        contest.winValue = win_value;
    });

}

void Contest:c_settle(name provider, uint64_t contest_id) {
    require_auth(oracle);

    // find contest
    db::contestIndex contests(_self, provider.value);
    auto contests_iterator = contests.find(contest_id);
    eosio_assert(contests_iterator != contests.end(), "Contest not found!");

    eosio_assert(contests_iterator->status != STATUS_INNITIALIZED, "Contest is finished or canceled!");
    eosio_assert(contests_iterator->finished <= now_time, "Contest timeout!");

    db::issuedIndex issued(_self, provider.value);
    db::endpointIndex endpoints(_self, provider.value);
    for (std::string s: contests_iterator->endpoints) {
        auto endpoint_index = endpoints.get_index<"byhash"_n>();
        auto endpoints_iterator = endpoint_index.find(db::hash(provider, s));
        eosio_assert(endpoints_iterator != endpoint_index.end(), "Endpoint doesn't exists.");

        auto issued_iterator = issued.find(endpoint_iterator->id);
        if (issued_iterator->dots > 0) {
             bondage.unbond(provider, provider, specifier, dots);
        }
    }

    contests.modify(contests_iterator, oracle, [&](auto &contest) {
        contest.status = STATUS_SETTELED;
    });
}

void Contest::c_bond(Bondage bondage, name issuer, uint64_t contest_id, std::string specifier, uint64_t dots) {
    require_auth(issuer);
    uint64_t now_time = now();

    // find contest
    db::contestIndex contests(_self, provider.value);
    auto contests_iterator = contests.find(contest_id);
    eosio_assert(contests_iterator != contests.end(), "Contest not found!");

    // check that user can bond
    eosio_assert(contests_iterator->status != STATUS_INNITIALIZED, "Contest is finished or canceled!");
    eosio_assert(contests_iterator->finished <= now_time, "Contest timeout!");

    bool specifier_exists = false;
    for (std::string e: contests_iterator->endpoints) {
        if (e == specifier) {
            specifier_exists = true;
            break;
        }
    }
    eosio_assert(!specifier_exists, "Specifier not found!");


    // find factory token
    db::ftokenIndex ftokens(_self, contest_iterator->provider.value);
    auto ftokens_index = ftokens.get_index<"byhash"_n>();
    auto ftokens_iterator = ftokens_index.find(db::hash(contest_iterator->provider, specifier));
    eosio_assert(ftokens_iterator != ftokens_index.end(), "Factory token not found!");

   
    // send provider zap tokens to bond
    bondage.bond(provider, provider, specifier, dots);

    // mint factory token to issuer
    asset tokens_to_mint = to_asset(dots, ftokens_iterator->symbol);
    action(
        permission_level{ _self, "active"_n },
        _self, "mint"_n,
        std::make_tuple(issuer, tokens_to_mint)
    ).send();
}

void Contest::c_unbond(Bondage bondage, name issuer, uint64_t contest_id, std::string specifier, uint64_t dots) {
    require_auth(issuer);
    uint64_t now_time = now();

    // find contest
    db::contestIndex contests(_self, provider.value);
    auto contests_iterator = contests.find(contest_id);
    eosio_assert(contests_iterator != contests.end(), "Contest not found!");

    // check that user can unbond
    if (contests_iterator->status == STATUS_SETTELED) {
         eosio_assert(contests_iterator->winner != specifier, "Only winner endpoint allows unbond");

         bool is_redeemed = false;
         for (name n: contests_iterator->reedemed) {
             if (n.value == issuer.value) {
                 is_redeemed = true;
                 break;
             }
         }
         eosio_assert(!is_redeemed, "Already redeemed!");

         action(
             permission_level{ _self, "active"_n },
             zap_token, "transfer"_n,
             std::make_tuple(_self, issuer, to_asset(contests_iterator->winValue), "reward")
         ).send();         

         std::vector<name> redeemed = contests_iterator->redeemed;
         redeemed.push_back(issuer);
         contests.modify(contests_iterator, issuer, [&](auto &contest) {
             contest.redeemed = redeemed;
         });
    } else {
        bool can_unbond = contests_iterator->status == STATUS_CANCELED || contests_iterator->finished <= now_time
        eosio_assert(!can_unbond, "Can not unbond before contest finish!");

        bool specifier_exists = false;
        for (std::string e: contests_iterator->endpoints) {
            if (e == specifier) {
                specifier_exists = true;
                break;
            }
        }
        eosio_assert(!specifier_exists, "Specifier not found!");

        // send provider zap tokens to bond
        bondage.unbond(provider, provider, specifier, dots);
    }
   
    // find factory token
    db::ftokenIndex ftokens(_self, contest_iterator->provider.value);
    auto ftokens_index = ftokens.get_index<"byhash"_n>();
    auto ftokens_iterator = ftokens_index.find(db::hash(contest_iterator->provider, specifier));
    eosio_assert(ftokens_iterator != ftokens_index.end(), "Factory token not found!");

    // burn factory token from issuer
    asset tokens_to_burn = to_asset(dots, ftokens_iterator->symbol);
    action(
        permission_level{ _self, "active"_n },
        _self, "burn"_n,
        std::make_tuple(issuer, tokens_to_burn)
    ).send();
}