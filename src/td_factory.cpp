#include "td_factory.hpp"

void TdFactory::td_init(Registry registry, name provider, std::string specifier, std::vector<int64_t> functions, asset maximum_supply) {
    require_auth(provider);

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
}

void TdFactory::td_bond(Bondage bondage, name issuer, name provider, std::string specifier, uint64_t dots) {
    require_auth(issuer);

    // find factory token
    db::ftokenIndex ftokens(_self, provider.value);
    auto ftokens_index = ftokens.get_index<"byhash"_n>();
    auto ftokens_iterator = ftokens_index.find(db::hash(provider, specifier));
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

void TdFactory::td_unbond(Bondage bondage, name issuer, name provider, std::string specifier, uint64_t dots) {
    require_auth(issuer);

    // find factory token
    db::ftokenIndex ftokens(_self, provider.value);
    auto ftokens_index = ftokens.get_index<"byhash"_n>();
    auto ftokens_iterator = ftokens_index.find(db::hash(provider, specifier));
    eosio_assert(ftokens_iterator != ftokens_index.end(), "Factory token not found!");

    // send provider zap tokens to bond
    bondage.unbond(provider, provider, specifier, dots);

    // mint factory token to issuer
    asset tokens_to_burn = to_asset(dots, ftokens_iterator->symbol);
    action(
        permission_level{ _self, "active"_n },
        _self, "burn"_n,
        std::make_tuple(issuer, tokens_to_burn)
    ).send();
}
