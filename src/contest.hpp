#include "database.hpp"
#include "bondage.hpp"
#include "registry.hpp"

using namespace eosio;
using std::string;


class Contest {
public:
    Contest(name n) : _self(n) {}

    void c_init(name provider, uint64_t finish, name oracle, std::vector<db::endp> endpoints);

    void c_judge(name oracle, std::string winner);

    void c_settle();

    void c_bond(Bondage bondage, name issuer, uint64_t contest_id, std::string specifier, uint64_t dots);

    void c_unbond(Bondage bondage, name issuer, uint64_t contest_id, std::string specifier, uint64_t dots);

private:
    name _self;

    //TODO: must be changed to prod account
    const name zap_token = "eosio.token"_n;

    //Convert specified amount of tokens to <asset> structure
    eosio::asset to_asset(uint64_t tokensAmount, symbol symbol) {
        return asset(tokensAmount, symbol);
    }
};
