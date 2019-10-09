#include "td_factory.hpp"

void TdFactory::td_init(EmbeddedToken embededd_token, Registry registry, name provider, std::string specifier, std::vector<int64_t> functions, asset maximum_supply) {
    require_auth(provider);

    // create new endpoint
    registry.addendpoint(provider, specifier, functions, provider);
	
    // create token for this endpoint
    embededd_token.internal_create(_self, maximum_supply);

    // store token symbol for specified curve
    db::ftokenIndex ftokens(_self, provider.value);
    ftokens.emplace(provider, [&](auto& newFtoken) {
            newFtoken.id = ftokens.available_primary_key();
	    newFtoken.provider = provider;
	    newFtoken.endpoint = specifier;
	    newFtoken.supply.symbol = maximum_supply.symbol;
    });

    db::fproviderIndex fproviders(_self, _self.value);
    auto iterator = fproviders.find(provider.value);
    if (iterator == fproviders.end()) {
        fproviders.emplace(provider, [&](auto& fp) {
            fp.provider = provider;
        });
    }
}

void TdFactory::td_bond(EmbeddedToken embededd_token, Bondage bondage, name issuer, name provider, std::string specifier, uint64_t dots) {
    require_auth(issuer);

    // find factory token
    db::ftokenIndex ftokens(_self, provider.value);
    auto ftokens_index = ftokens.get_index<"byhash"_n>();
    auto ftokens_iterator = ftokens_index.find(db::hash(provider, specifier));
    eosio_assert(ftokens_iterator != ftokens_index.end(), "Factory token not found!");

    // send provider zap tokens to bond
    bondage.internal_bond(provider, provider, specifier, dots, issuer);

    // mint factory token to issuer
    asset tokens_to_mint = to_asset(dots, ftokens_iterator->supply.symbol);
    embededd_token.internal_mint(issuer, tokens_to_mint);
}

void TdFactory::td_unbond(EmbeddedToken embededd_token, Bondage bondage, name issuer, name provider, std::string specifier, uint64_t dots) {
    // find factory token
    db::ftokenIndex ftokens(_self, provider.value);
    auto ftokens_index = ftokens.get_index<"byhash"_n>();
    auto ftokens_iterator = ftokens_index.find(db::hash(provider, specifier));
    eosio_assert(ftokens_iterator != ftokens_index.end(), "Factory token not found!");

    // send provider zap tokens to bond
    bondage.internal_unbond(provider, provider, specifier, dots, issuer);

    // mint factory token to issuer
    asset tokens_to_burn = to_asset(dots, ftokens_iterator->supply.symbol);
    embededd_token.internal_burn(issuer, tokens_to_burn);
}
