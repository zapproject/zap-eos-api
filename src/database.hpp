#ifndef DATABASE_HEADER
#define DATABASE_HEADER

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/fixed_key.hpp>
#include <eosiolib/asset.hpp>
#include <string>
#include <vector>

using namespace eosio;

namespace db {
    //TODO: must be changed to prod account
    static account_name zap_token = N(zap.token);
    
    static const std::string ZAP_TOKEN_SYMBOL = "TST";
    static const uint8_t ZAP_TOKEN_DECIMALS = 0;

    static std::array<uint128_t, 2> checksum256_to_uint128array(checksum256 c);
    static key256 hash(account_name provider, std::string specifier);

    //Convert specified amount of tokens to <asset> structure
    static eosio::asset toAsset(uint64_t tokensAmount) {
        return asset(tokensAmount, symbol_type(string_to_symbol(ZAP_TOKEN_DECIMALS, ZAP_TOKEN_SYMBOL.c_str())));
    }

    static void transfer_tokens(account_name from, account_name to, uint64_t amount, std::string memo) {
        action(
            permission_level{ from, N(active) },
            zap_token, N(transfer),
            std::make_tuple(from, to, toAsset(amount), memo)
        ).send();
    }

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

    //@abi table endpoint i64
    //Table for provider endpoints, created in context of specified provider
    struct endpoint {
        uint64_t id;
        account_name provider;
        std::string specifier;
        std::vector<int64_t> constants;
        std::vector<uint64_t> parts;
        std::vector<uint64_t> dividers;

        uint64_t primary_key() const { return id; }
        uint64_t get_provider() const { return provider; }
           
        // EXPERIMENTAL FEATURE
        // I haven't found any examples of key256 usage, but, according doc, multi_index supports 256 bytes secondary keys 
        // this secondary key allows to find item with specified provider and specifier by using find() method
        key256 get_hash() const { return db::hash(provider, specifier); }
	  
        EOSLIB_SERIALIZE(endpoint, (id)(provider)(specifier)(constants)(parts)(dividers))
    };

    //@abi table provider i64
    //Table for providers, created in context of zap registry contract
    struct provider {
        account_name user;
        std::string title;
        uint64_t key;

        uint64_t primary_key() const { return user; }

        EOSLIB_SERIALIZE(provider, (user)(title)(key))
    };

    //@abi table holder i64
    //Table for user holders, created in context of user
    struct holder {
        account_name provider;
        std::string endpoint;
        uint64_t dots;
        uint64_t escrow;

        uint64_t primary_key() const { return provider; }
        key256 get_hash() const { return db::hash(provider, endpoint); }

        EOSLIB_SERIALIZE(holder, (provider)(endpoint)(dots)(escrow))
    };     

    //@abi table issued i64
    //Table to store total issued dots for endpoint, created in context of provider, pk is same as pk in endpoints
    struct issued {
        uint64_t endpointid;
        uint64_t dots;
        
        uint64_t primary_key() const { return endpointid; }

        EOSLIB_SERIALIZE(issued, (endpointid)(dots))
    };

    //@abi table query_data i64
    //Table to store user queries
    struct query_data {
        uint64_t id;
        account_name provider;
        account_name subscriber;
        std::string endpoint;
        std::string data;
        bool onchain;

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(query_data, (id)(provider)(subscriber)(endpoint)(data)(onchain))
    };

    typedef multi_index<N(provider), provider> providerIndex;
    typedef multi_index<N(endpoint), endpoint,
                indexed_by<N(byprovider), const_mem_fun<endpoint, uint64_t, &endpoint::get_provider>>,
                indexed_by<N(byhash), const_mem_fun<endpoint, key256, &endpoint::get_hash>>
            > endpointIndex;
    typedef multi_index<N(holder), holder,
                indexed_by<N(byhash), const_mem_fun<holder, key256, &holder::get_hash>>
            > holderIndex;
    typedef multi_index<N(issued), issued> issuedIndex;
    typedef multi_index<N(query_data), query_data> queryIndex;

    static uint64_t update_issued(issuedIndex& issued, account_name payer, uint64_t endpoint_id, uint64_t dots) {
        auto issued_iterator = issued.find(endpoint_id);
        uint64_t total_issued_dots = 0;
        if (issued_iterator != issued.end()) {
            total_issued_dots = issued_iterator->dots;
            issued.modify(issued_iterator, payer, [&](auto& i) {
                i.dots = i.dots + dots;
            });
            print_f("Issued updated, added value = %.\n", dots);
        } else {
            issued.emplace(payer, [&](auto& i) {
                i.endpointid = endpoint_id;
                i.dots = dots;
            });
            print_f("New issued created, endpointid = %; dots = %.\n", endpoint_id, dots);
        }
        return total_issued_dots;
    }

    static auto update_holder(holderIndex& holders, account_name payer, account_name provider, std::string endpoint, uint64_t dots, uint64_t escrow) {
        auto holders_index = holders.get_index<N(byhash)>();
        auto holders_iterator = holders_index.find(db::hash(provider, endpoint));
        if (holders_iterator != holders_index.end()) {
            holders_index.modify(holders_iterator, payer, [&](auto& h) {
                h.dots = h.dots + dots;
                h.escrow = h.escrow + escrow;
            });
            print_f("Holder updated, added value = %.\n", dots);
        } else {
            holders.emplace(payer, [&](auto& h) {
                h.provider = provider;
                h.endpoint = endpoint;
                h.dots = dots;
                h.escrow = escrow;
            });
            print_f("New holder created, provider = %; endpoint = %; dots = %.\n", name{provider}, endpoint, dots);
        }
        return holders_iterator;
    }

    static bool delete_query(queryIndex& queries, uint64_t id) {       
        auto iterator = queries.find(id);
        if (iterator != queries.end()) {
            queries.erase(iterator);
            return true;
        } else {
            return false;
        }
    }
        
}

#endif DATABASE_HEADER
