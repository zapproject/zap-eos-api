#include "database.hpp"

using namespace eosio;
using std::string;


class EmbeddedToken {
public:
    EmbeddedToken(name n) : _self(n) {}

    void create(name issuer, asset maximum_supply);

    void issue(name to, asset quantity, string memo);

    void mint(name to, asset quantity);

    void burn(name from, asset quantity);

    void retire(asset quantity, string memo);

    void transfer(name from, name to, asset quantity, string memo);

    void open(name owner, const symbol &symbol, name ram_payer);

    void close(name owner, const symbol &symbol);

    void internal_create(name issuer, asset maximum_supply);

    void internal_issue(name to, asset quantity, string memo);

    void internal_mint(name to, asset quantity);

    void internal_burn(name from, asset quantity);

    void internal_transfer(name from, name to, asset quantity, string memo);

    static asset get_supply(name token_contract_account, symbol_code sym_code)
    {
        db::stats statstable(token_contract_account, sym_code.raw());
        const auto &st = statstable.get(sym_code.raw());
        return st.supply;
    }

    static asset get_balance(name token_contract_account, name owner, symbol_code sym_code)
    {
        db::accounts accountstable(token_contract_account, owner.value);
        const auto &ac = accountstable.get(sym_code.raw());
        return ac.balance;
    }

private:
    name _self;

    //TODO: must be changed to prod account
    const name zap_token = "eosio.token"_n;

    void sub_balance(name owner, asset value);
    void add_balance(name owner, asset value, name ram_payer);
};
