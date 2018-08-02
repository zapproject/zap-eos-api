#pragma once

#include <bondage.cpp>
#include <registry.cpp>

using namespace eosio;

class Main: public eosio::contract {
    public:
        using contract::contract;

        Main(account_name n): eosio::contract(n), bondage(n), regsitry(n) { }

        // REGISTRY METHODS

        //@abi action
        //Create new provider
        //<provider> param must be valid account and action sender must have permissions for this acc
        void newprovider(account_name provider, std::string title, uint64_t public_key);

        //@abi action
        //Add new endpoint for provider
        //<provider> param must be valid account and action sender must have permissions for this acc
        void addendpoint(account_name provider, std::string specifier, std::vector<int64_t> constants, std::vector<uint64_t> parts, std::vector<uint64_t> dividers);

        // BONDAGE METHODS

        //@abi action
        //Buy dots for specified endpoint
        void bond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        //Withdraw dots for specified provider
        void unbond(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        //Estimate price of dots (in integral zap tokens) for specified provider 
        void estimate(account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        //Escrow dots to specified endpoint
        //Can be called only by dispatch provider
        //User can not withdraw dots from escrow
        void escrow(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        //@abi action
        //Convert escrowed dots of subscriber to zap tokens and send tokens to provider
        //Escrow dots will be removed
        //Provider will receive zap tokens for escrowed dots
        //Dots will be removed from total issued of endpoint
        //Dots will be removed from subscriber holder
        void release(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

	
        // VIEW METHODS

        //@abi action
        //View all provider in specified bounds
        //<from> - start index
        //<to> - end index
        void viewps(uint64_t from, uint64_t to);

        //@abi action
        //View all endpoints for specified provider in specified bounds
        //<from> - start index
        //<to> - end index
        void viewes(account_name provider, uint64_t from, uint64_t to);

        //@abi action
        //View params of specified endpoint
        void endpbyhash(account_name provider, std::string specifier);

        //@abi action
        //View specified endpoint dots for specified holder
        void viewhe(account_name holder, account_name provider, std::string endpoint);

        //@abi action
        //View all endpoints for specified holder
        void viewh(account_name holder);
        
        //@abi action
        //View total issued dots for specified endpoint
        void viewi(account_name provider, std::string endpoint);


     
    private:

        Bondage bondage;

        Registry registry;
};

EOSIO_ABI(Main, (newprovider)(addendpoint)(bond)(unbond)(estimate)(escrow)(release)(viewps)(viewes)(endpbyhash)(viewhe)(viewh)(viewi))
