#ifndef DATABASE_HEADER
#define DATABASE_HEADER

#define ZAP_TOKEN_SYMBOL "EOS"
#define ZAP_TOKEN_DECIMALS 4

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/fixed_key.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/multi_index.hpp>
#include <string>
#include <vector>

using namespace eosio;

namespace db {

    static fixed_bytes<32> hash(name provider, std::string specifier);
    static std::array<uint128_t, 2> checksum256_to_uint128array(capi_checksum256 c);

    // TODO: must be reviewed
    // Does this hash will be always unique?
    static fixed_bytes<32> hash(name provider, std::string specifier) {
        // TODO: std::to_string currently not working with eosio.cdt compiler
        // https://github.com/EOSIO/eosio.cdt/issues/95, then use name{}.to_string()
        std::string provider_string = name{provider}.to_string();
        std::string concatenated = provider_string + specifier;
        uint32_t result_size = concatenated.length() + 1;

        char *data = new char[result_size];
        strcpy(data, concatenated.c_str());

        capi_checksum256 hash_result;
        sha256(data, result_size, &hash_result);

        delete [] data;
        return fixed_bytes<32>(checksum256_to_uint128array(hash_result));
    }

    // TODO: must be reviewed
    // !!! according this doc https://developers.eos.io/eosio-cpp/docs/random-number-generation checksum element is 4 bytes (uin32_t == int) length
    // !!! but in sources it is 1 byte length (uint8_t == unsigned char)
    static std::array<uint128_t, 2> checksum256_to_uint128array(capi_checksum256 c) {
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

    //@abi table endpoint i64
    //Table for provider endpoints, created in context of specified provider
    struct [[eosio::table]] endpoint {
        uint64_t id;
        name provider;
        std::string specifier;
        name broker;
        std::vector<int64_t> functions;

        uint64_t primary_key() const { return id; }
        uint64_t get_provider() const { return provider.value; }

        // I haven't found any examples of fixed_bytes<32> usage, but, according doc, multi_index supports 256 bytes secondary keys 
        // this secondary key allows to find item with specified provider and specifier by using find() method
        fixed_bytes<32> by_hash() const { return db::hash(provider, specifier); }

        EOSLIB_SERIALIZE(endpoint, (id)(provider)(specifier)(broker)(functions))
    };

    //@abi table provider i64
    //Table for providers, created in context of zap registry contract
    struct [[eosio::table]] provider {
        name user;
        std::string title;
        uint64_t key;

        uint64_t primary_key() const { return user.value; }

        EOSLIB_SERIALIZE(provider, (user)(title)(key))
    };

    //@abi table holder i64
    //Table for user holders, created in context of user
    struct [[eosio::table]] holder {
        uint64_t id;
        name provider;
        std::string endpoint;
        uint64_t dots;
        uint64_t escrow;

        uint64_t primary_key() const { return id; }
        uint64_t get_provider() const { return provider.value; }
        fixed_bytes<32> get_hash() const { return db::hash(provider, endpoint); }

        EOSLIB_SERIALIZE(holder, (id)(provider)(endpoint)(dots)(escrow))
    };

    //@abi table issued i64
    //Table to store total issued dots for endpoint, created in context of provider, pk is same as pk in endpoints
    struct [[eosio::table]] issued {
        uint64_t endpointid;
        uint64_t dots;
        
        uint64_t primary_key() const { return endpointid; }

        EOSLIB_SERIALIZE(issued, (endpointid)(dots))
    };

    //@abi table qdata i64
    //Table to store user queries
    struct [[eosio::table]] qdata {
        uint64_t id;
        name provider;
        name subscriber;
        std::string endpoint;
        std::string data;
        bool onchain;
        uint128_t timestamp;

        uint64_t primary_key() const { return id; }
        uint64_t get_provider() const { return provider.value; }
        uint128_t get_timestamp() const { return timestamp; }

        EOSLIB_SERIALIZE(qdata, (id)(provider)(subscriber)(endpoint)(data)(onchain)(timestamp))
    };

    //@abi table subscription i64
    struct [[eosio::table]] subscription {
        uint64_t id;
        uint64_t price;
        uint64_t start;
        uint64_t end;
        name subscriber;
        std::string endpoint;

        uint64_t primary_key() const { return id; }
        fixed_bytes<32> get_hash() const { return db::hash(subscriber, endpoint); }

        EOSLIB_SERIALIZE(subscription, (id)(price)(start)(end)(subscriber)(endpoint))
    };

    //@abi table params i64
    struct [[eosio::table]] params {
        uint64_t id;
        name provider;
        std::string endpoint;
        std::vector<std::string> values;

        uint64_t primary_key() const { return id; }
        fixed_bytes<32> by_hash() const { return db::hash(provider, endpoint); }

        EOSLIB_SERIALIZE(params, (id)(provider)(endpoint)(values))
    };


    //Embedded token tables
    struct [[eosio::table]] account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }

        EOSLIB_SERIALIZE(account, (balance))
    };

    struct [[eosio::table]] currency_stats {
        asset supply;
        asset max_supply;
        name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }

        EOSLIB_SERIALIZE(currency_stats, (supply)(max_supply)(issuer))
    };
 
    //Token dot factory tables
    struct [[eosio::table]] ftoken {
        uint64_t id;
        symbol symbol;
        std::string endpoint;
        name provider;

        uint64_t primary_key() const { return id; }
        fixed_bytes<32> by_hash() const { return db::hash(provider, endpoint); }

        EOSLIB_SERIALIZE(ftoken, (id)(symbol)(endpoint)(provider))
    };

    //Contest
    struct [[eosio::table]] contest {
        uint64_t id;
        name provider;
        name oracle;
        uint64_t finish;
        uint64_t status;
        std::string winner;
        std::vector<std::string> endpoints;

        uint64_t primary_key() const { return id; }
        uint64_t get_provider() const { return provider.value; }

        EOSLIB_SERIALIZE(contest, (provider)(status)(oracle))
    };

    // not stored in db
    struct endp {
        std::string specifier;
        std::vector<int64_t> functions;
        asset maximum_supply;      
    };

    typedef multi_index<"contest"_n, contest,
                indexed_by<"byprovider"_n, const_mem_fun<contest, uint64_t, &contest::get_provider>>
            > contestIndex;

    typedef multi_index<"ftoken"_n, ftoken,
                indexed_by<"byhash"_n, const_mem_fun<ftoken, fixed_bytes<32>, &ftoken::by_hash>>
            > ftokenIndex;

    typedef multi_index<"accounts"_n, account> accounts;
    typedef multi_index<"stat"_n, currency_stats> stats;


    typedef multi_index<"provider"_n, provider> providerIndex;

    typedef multi_index<"endpoint"_n, endpoint,
                indexed_by<"byprovider"_n, const_mem_fun<holder, uint64_t, &endpoint::get_provider>>,
                indexed_by<"byhash"_n, const_mem_fun<endpoint, fixed_bytes<32>, &endpoint::by_hash>>
            > endpointIndex;

    typedef multi_index<"holder"_n, holder,
                indexed_by<"byhash"_n, const_mem_fun<holder, fixed_bytes<32>, &holder::get_hash>>,
                indexed_by<"byprovider"_n, const_mem_fun<holder, uint64_t, &holder::get_provider>>
            > holderIndex;

    typedef multi_index<"issued"_n, issued> issuedIndex;

    typedef multi_index<"qdata"_n, qdata,
                indexed_by<"byprovider"_n, const_mem_fun<qdata, uint64_t, &qdata::get_provider>>,
                indexed_by<"bytimestamp"_n, const_mem_fun<qdata, uint128_t, &qdata::get_timestamp>>
            > queryIndex;

    typedef multi_index<"subscription"_n, subscription,
                indexed_by<"byhash"_n, const_mem_fun<subscription, fixed_bytes<32>, &subscription::get_hash>>
            > subscriptionIndex;

    typedef multi_index<"params"_n, params,
            indexed_by<"byhash"_n, const_mem_fun<params, fixed_bytes<32>, &params::by_hash>>
    > paramsIndex;
}

#endif
