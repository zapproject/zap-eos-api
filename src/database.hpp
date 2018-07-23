#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/fixed_key.hpp>
#include <string>
#include <vector>

using namespace eosio;

namespace db {

    static std::array<uint128_t, 2> checksum256_to_uint128array(checksum256 c);
    static key256 hash(account_name provider, std::string specifier);

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
    struct endpoint {
        uint64_t id;
        account_name provider;
        std::string specifier;
        std::vector<int64_t> constants;
        std::vector<uint64_t> parts;
        std::vector<uint64_t> dividers;
        uint128_t issued;

        uint64_t primary_key() const { return id; }
        uint64_t get_provider() const { return provider; }
           
        // EXPERIMENTAL FEATURE
        // I haven't found any examples of key256 usage, but, according doc, multi_index supports 256 bytes secondary keys 
        // this secondary key allows to find item with specified provider and specifier by using find() method
        key256 get_hash() const { return db::hash(provider, specifier); }
	  
        EOSLIB_SERIALIZE(endpoint, (id)(provider)(specifier)(constants)(parts)(dividers)(issued))
    };

    //@abi table provider i64
    struct provider {
        account_name user;
        std::string title;
        uint64_t key;

        uint64_t primary_key() const { return user; }

        EOSLIB_SERIALIZE(provider, (user)(title)(key))
    };

    //@abi table holder i64
    struct holder {
        account_name provider;
        std::string endpoint;
        uint64_t dots;
        uint64_t escrow;

        uint64_t primary_key() const { return provider; }
        key256 get_hash() const { return db::hash(provider, endpoint); }

        EOSLIB_SERIALIZE(holder, (provider)(endpoint)(dots)(escrow))
    };     

    typedef multi_index<N(provider), provider> providerIndex;
    typedef multi_index<N(endpoint), endpoint,
                indexed_by<N(byprovider), const_mem_fun<endpoint, uint64_t, &endpoint::get_provider>>,
                indexed_by<N(byhash), const_mem_fun<endpoint, key256, &endpoint::get_hash>>
            > endpointIndex;
    typedef multi_index<N(holder), holder,
                indexed_by<N(byhash), const_mem_fun<holder, key256, &holder::get_hash>>
            > holderIndex;
}
