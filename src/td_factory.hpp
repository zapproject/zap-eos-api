#include "database.hpp"
#include "bondage.hpp"
#include "registry.hpp"

using namespace eosio;
using std::string;


class TdFactory {
public:
    TdFactory(name n) : _self(n) {}

    void td_init(Registry registry, name provider, std::string specifier, std::vector<int64_t> functions, asset maximum_supply);

    void td_bond(Bondage bondage, name issuer, name provider, std::string specifier, uint64_t dots);

    void td_unbond(Bondage bondage, name issuer, name provider, std::string specifier, uint64_t dots);

private:
    name _self;

    //TODO: must be changed to prod account
    const name zap_token = "eosio.token"_n;

    //Convert specified amount of tokens to <asset> structure
    eosio::asset to_asset(uint64_t tokensAmount, symbol symbol) {
        return asset(tokensAmount, symbol);
    }
};
