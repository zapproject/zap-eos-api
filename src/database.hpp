#ifndef DATABASE_HEADER
#define DATABASE_HEADER

#define ZAP_TOKEN_SYMBOL "TST"
#define ZAP_TOKEN_DECIMALS 0

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/fixed_key.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/multi_index.hpp>
#include <string>
#include <vector>

using namespace eosio;

namespace db {

    static key256 hash(account_name provider, std::string specifier);
    static std::array<uint128_t, 2> checksum256_to_uint128array(checksum256 c);

    // TODO: must be reviewed
    // Does this hash will be always unique?
    static key256 hash(account_name provider, std::string specifier) {
        std::string provider_string = std::to_string(provider);
        std::string concatenated = provider_string + specifier;
        uint32_t result_size = concatenated.length() + 1;

        char *data = new char[result_size];
        strcpy(data, concatenated.c_str());

        checksum256 hash_result;
        sha256(data, result_size, &hash_result);

        delete [] data;
        return key256(checksum256_to_uint128array(hash_result));
    }

    // TODO: must be reviewed
    // !!! according this doc https://developers.eos.io/eosio-cpp/docs/random-number-generation checksum element is 4 bytes (uin32_t == int) length
    // !!! but in sources it is 1 byte length (uint8_t == unsigned char)
    static std::array<uint128_t, 2> checksum256_to_uint128array(checksum256 c) {
        uint128_t first_word = 0;
        uint128_t second_word = 0;

        uint32_t half_size = sizeof(c.hash) / 2;
        uint32_t byte_size = 8;
        for (uint32_t i = 0; i < half_size; i++) {
            first_word = (first_word << byte_size) + c.hash[(half_size - 1) - i];
            second_word = (second_word << byte_size) + c.hash[((half_size * 2) - 1) - i];
        }
         
        std::array<uint128_t, 2> words = { first_word, second_word };
        return words;
    }

    //Table for provider endpoints, created in context of specified provider
    struct [[eosio::table]] endpoint {
        uint64_t id;
        account_name provider;
        std::string specifier;
        account_name broker;
        std::vector<int64_t> functions;

        uint64_t primary_key() const { return id; }
        uint64_t by_provider() const { return provider; }
           
        // TODO: EXPERIMENTAL FEATURE
        // I haven't found any examples of key256 usage, but, according doc, multi_index supports 256 bytes secondary keys 
        // this secondary key allows to find item with specified provider and specifier by using find() method
        key256 by_hash() const { return db::hash(provider, specifier); }
    };

    //Table for providers, created in context of zap registry contract
    struct [[eosio::table]] provider {
        account_name user;
        std::string title;
        uint64_t key;

        uint64_t primary_key() const { return user; }
    };

    //Table for user holders, created in context of user
    struct [[eosio::table]] holder {
        account_name provider;
        std::string endpoint;
        uint64_t dots;
        uint64_t escrow;

        uint64_t primary_key() const { return provider; }
        key256 get_hash() const { return db::hash(provider, endpoint); }
    };     

    //Table to store total issued dots for endpoint, created in context of provider, pk is same as pk in endpoints
    struct [[eosio::table]] issued {
        uint64_t endpointid;
        uint64_t dots;
        
        uint64_t primary_key() const { return endpointid; }
    };

    //Table to store user queries
    struct [[eosio::table]] qdata {
        uint64_t id;
        account_name provider;
        account_name subscriber;
        std::string endpoint;
        std::string data;
        bool onchain;

        uint64_t primary_key() const { return id; }
    };

    struct [[eosio::table]] subscription {
        uint64_t id;
        uint64_t price;
        uint64_t start;
        uint64_t end;
        account_name subscriber;
        std::string endpoint;

        uint64_t primary_key() const { return id; }
        key256 get_hash() const { return db::hash(subscriber, endpoint); }
    };


    typedef multi_index<N(provider), provider> providerIndex;

    typedef multi_index<N(endpoint), endpoint,
                indexed_by<N(byhash), const_mem_fun<endpoint, key256, &endpoint::by_hash>>,
                indexed_by<N(byprovider), const_mem_fun<endpoint, account_name, &endpoint::by_provider>>
            > endpointIndex;

    typedef multi_index<N(holder), holder,
                indexed_by<N(byhash), const_mem_fun<holder, key256, &holder::get_hash>>
            > holderIndex;

    typedef multi_index<N(issued), issued> issuedIndex;

    typedef multi_index<N(qdata), qdata> queryIndex;

    typedef multi_index<N(subscription), subscription,
                indexed_by<N(byhash), const_mem_fun<subscription, key256, &subscription::get_hash>>
            > subscriptionIndex;
 
}

#endif
