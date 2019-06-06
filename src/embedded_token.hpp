#include "database.hpp"

using namespace eosio;
using std::string;

namespace eosio
{
class EmbeddedToken
{
public:
    EmbeddedToken(name n) : _self(n), {}

                                          [[eosio::action]] void create(name issuer, asset maximum_supply);

    [[eosio::action]] void issue(name to, asset quantity, string memo);

    [[eosio::action]] void burn(name from, asset quantity, string memo);

    [[eosio::action]] void retire(asset quantity, string memo);

    [[eosio::action]] void transfer(name from, name to, asset quantity, string memo);

    [[eosio::action]] void open(name owner, const symbol &symbol, name ram_payer);

    [[eosio::action]] void close(name owner, const symbol &symbol);

    static asset get_supply(name token_contract_account, symbol_code sym_code)
    {
        stats statstable(token_contract_account, sym_code.raw());
        const auto &st = statstable.get(sym_code.raw());
        return st.supply;
    }

    static asset get_balance(name token_contract_account, name owner, symbol_code sym_code)
    {
        accounts accountstable(token_contract_account, owner.value);
        const auto &ac = accountstable.get(sym_code.raw());
        return ac.balance;
    }

private:
    name _self;

    //TODO: must be changed to prod account
    const name zap_token = "zap.token"_n;

    void sub_balance(name owner, asset value);
    void add_balance(name owner, asset value, name ram_payer);
};
} // namespace eosio