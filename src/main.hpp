#pragma once

#include "bondage.cpp"
#include "registry.cpp"
#include "dispatcher.cpp"

using namespace eosio;

class Main : public eosio::contract {
public:
    using contract::contract;

    Main(account_name n) : eosio::contract(n), bondage(n), registry(n), dispatcher(n) {}

    // REGISTRY METHODS

    //Create new provider
    //<provider> param must be valid account and action sender must have permissions for this acc
    [[eosio::action]]
    void newprovider(account_name provider, std::string title, uint64_t public_key);

    //Add new endpoint for provider
    //<provider> param must be valid account and action sender must have permissions for this acc
    [[eosio::action]]
    void addendpoint(account_name provider, std::string specifier, std::vector<int64_t> functions, account_name broker);

    //Set params for endpoint or provider (endpoint == '')
    //<provider> param must be valid account and action sender must have permissions for this acc
    [[eosio::action]]
    void setparams(account_name provider, std::string specifier, std::vector<std::string> params);

    // BONDAGE METHODS

    //Buy dots for specified endpoint
    [[eosio::action]]
    void bond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

    //Withdraw dots for specified provider
    [[eosio::action]]
    void unbond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

    //Estimate price of dots (in integral zap tokens) for specified provider
    [[eosio::action]]
    void estimate(account_name provider, std::string endpoint, uint64_t dots);

    // DISPATCHER METHODS

    //Query provider data
    [[eosio::action]]
    void query(account_name subscriber, account_name provider, std::string endpoint, std::string query,
               bool onchain_provider, bool onchain_subscriber);

    //Query provider data
    [[eosio::action]]
    void respond(account_name responder, uint64_t id, std::string params);

    //Buy subscription to provider endpoint
    [[eosio::action]]
    void subscribe(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

    //Remove subscription
    [[eosio::action]]
    void unsubscribe(account_name subscriber, account_name provider, std::string endpoint, bool from_sub);

    //Remove query
    [[eosio::action]]
    void cancelquery(account_name subscriber, uint64_t query_id);

private:
    Bondage bondage;
    Registry registry;
    Dispatcher dispatcher;
};

EOSIO_ABI(Main, (newprovider)(addendpoint)(bond)(unbond)(estimate)(query)(respond)(subscribe)(unsubscribe)(cancelquery)(setparams))