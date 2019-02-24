#pragma once

#include "bondage.cpp"
#include "registry.cpp"
#include "dispatcher.cpp"

using namespace eosio;

class [[eosio::contract("Main")]] Main : public contract {
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
    void addendpoint(name provider, std::string specifier, std::vector <int64_t> functions, name broker);

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
               bool onchain_provider, bool onchain_subscriber);

    //Query provider data
    [[eosio::action]]
    void respond(name responder, uint64_t id, std::string params);

    //Buy subscription to provider endpoint
    [[eosio::action]]
    void subscribe(name subscriber, name provider, std::string endpoint, uint64_t dots);

    //Remove subscription
    [[eosio::action]]
    void unsubscribe(name subscriber, name provider, std::string endpoint, bool from_sub);

private:
    Bondage bondage = Bondage(get_self());
    Registry registry = Registry(get_self());
    Dispatcher dispatcher = Dispatcher(get_self());
};

EOSIO_DISPATCH(Main, (newprovider)(addendpoint)(bond)(unbond)(estimate)(query)(respond)(subscribe)(unsubscribe))
