#pragma once

#include "bondage.cpp"
#include "registry.cpp"
#include "dispatcher.cpp"
#include "embedded_token.cpp"
#include "td_factory.cpp"
#include "contest.cpp"

using namespace eosio;

class [[eosio::contract("main")]] main : public eosio::contract {
public:
    using contract::contract;

    // REGISTRY METHODS

    //Create new provider
    //<provider> param must be valid account and action sender must have permissions for this acc
    [[eosio::action]]
    void newprovider(name provider, std::string title, uint64_t public_key);

    //Add new endpoint for provider
    //<provider> param must be valid account and action sender must have permissions for this acc
    [[eosio::action]]
    void addendpoint(name provider, std::string specifier, std::vector<int64_t> functions, name broker);

    //Set params for endpoint or provider (endpoint == '')
    //<provider> param must be valid account and action sender must have permissions for this acc
    [[eosio::action]]
    void setparams(name provider, std::string specifier, std::vector<std::string> params);

    // BONDAGE METHODS

    //Buy dots for specified endpoint
    [[eosio::action]]
    void bond(name subscriber, name provider, std::string endpoint, uint64_t dots);

    //Withdraw dots for specified provider
    [[eosio::action]]
    void unbond(name subscriber, name provider, std::string endpoint, uint64_t dots);

    //Estimate price of dots (in integral zap tokens) for specified provider
    [[eosio::action]]
    void estimate(name provider, std::string endpoint, uint64_t dots);

    // DISPATCHER METHODS

    //Query provider data
    [[eosio::action]]
    void query(name subscriber, name provider, std::string endpoint, std::string query,
               bool onchain_provider, bool onchain_subscriber, uint128_t timestamp);

    //Query provider data
    [[eosio::action]]
    void respond(name responder, uint64_t id, std::string params, name subscriber);

    //Buy subscription to provider endpoint
    [[eosio::action]]
    void subscribe(name subscriber, name provider, std::string endpoint, uint64_t dots, std::string params);

    //Remove subscription
    [[eosio::action]]
    void unsubscribe(name subscriber, name provider, std::string endpoint, bool from_sub);

    //Remove query
    [[eosio::action]]
    void cancelquery(name subscriber, uint64_t query_id);


    // EMBEDDED TOKEN ACTIONS
    [[eosio::action]]
    void create(name issuer, asset maximum_supply);

    [[eosio::action]] 
    void issue(name to, asset quantity, string memo);

    [[eosio::action]] 
    void mint(name to, asset quantity);

    [[eosio::action]] 
    void burn(name from, asset quantity);

    [[eosio::action]] 
    void retire(asset quantity, string memo);

    [[eosio::action]] 
    void transfer(name from, name to, asset quantity, string memo);

    [[eosio::action]] 
    void open(name owner, const symbol &symbol, name ram_payer);

    [[eosio::action]] 
    void close(name owner, const symbol &symbol);


    // TOKEN DOT FACTORY ACTIONS
    [[eosio::action]] 
    void tdinit(name provider, std::string specifier, std::vector<int64_t> functions, asset maximum_supply);

    [[eosio::action]] 
    void tdbond(name issuer, name provider, std::string specifier, uint64_t dots);

    [[eosio::action]] 
    void tdunbond(name issuer, name provider, std::string specifier, uint64_t dots);


    // CONTEST ACTIONS
    [[eosio::action]] 
    void cinit(name provider, uint64_t finish, name oracle, std::vector<db::endp> endpoints);

    [[eosio::action]]
    void cjudge(uint64_t contest_id, name provider, name oracle, std::string winner, uint64_t win_value);

    [[eosio::action]]
    void csettle(name provider, uint64_t contest_id);

    [[eosio::action]]
    void cbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots);

    [[eosio::action]]
    void cunbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots);

    [[eosio::action]]
    void testrp(name issuer);

    [[eosio::action]]
    void rpcallback(name issuer);

    [[eosio::action]]
    void tcallback(name from, name to, asset quantity, std::string memo);

private:
    Bondage bondage = Bondage(get_self());
    Registry registry = Registry(get_self());
    Dispatcher dispatcher = Dispatcher(get_self());
    EmbeddedToken embToken = EmbeddedToken(get_self());
    TdFactory tdFactory = TdFactory(get_self());
    Contest contest = Contest(get_self());   
};

