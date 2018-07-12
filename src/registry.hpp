#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/fixed_key.hpp>
#include <string>
#include <vector>

using namespace eosio;

class Registry: public eosio::contract {
    public:
        using contract::contract;

        //@!abi table endpoint i64
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
            // this secondary key allows to find item by using find() method
            key256 get_hash() const { return Registry::hash(provider, specifier); }
	    
            EOSLIB_SERIALIZE(endpoint, (id)(provider)(specifier)(constants)(parts)(dividers))
	};

	//@abi table provider i64
        struct provider {
            account_name user;
            std::string title;
            uint64_t key;

            uint64_t primary_key() const { return user; }

            EOSLIB_SERIALIZE(provider, (user)(title)(key))
        };

	
        // REGISTER METHODS

        //@abi action
        void newprovider(account_name provider, std::string title, uint64_t public_key);

        //@abi action
        void addendpoint(account_name provider, std::string specifier, std::vector<int64_t> constants, std::vector<uint64_t> parts, std::vector<uint64_t> dividers);

	
        // VIEW METHODS

        //@abi action
        void viewps(uint64_t from, uint64_t to);

        //@abi action
        void viewes(account_name provider, uint64_t from, uint64_t to);

        //@abi action
        void endpbyhash(account_name provider, std::string specifier);


        typedef multi_index<N(provider), provider> providerIndex;
        typedef multi_index<N(endpoint), endpoint,
				indexed_by<N(byprovider), const_mem_fun<endpoint, uint64_t, &endpoint::get_provider>>,
                                indexed_by<N(byhash), const_mem_fun<endpoint, key256, &endpoint::get_hash>>
                           > endpointIndex;

    private:
	std::string vector_to_string(std::vector<uint64_t> v) {
            std::string str = "[";
            
            uint64_t counter = 0;
            for (auto const& item: v) {
                if (counter > 0) {
                    str += ", ";	
                }
	
                str += std::to_string(item);
                counter++;
            }
            
            str += "]";
            return str;
        }

        std::string vector_to_string(std::vector<int64_t> v) {
            std::string str = "[";
            
            uint64_t counter = 0;
            for (auto const& item: v) {
                if (counter > 0) {
                    str += ", ";	
                }
	
                str += std::to_string(item);
                counter++;
            }
            
            str += "]";
            return str;
        }

        // TODO: should be refactored to optimize search
        bool validateEndpoint(const Registry::endpointIndex &idx, account_name provider, std::string specifier) {
            auto iterator = idx.begin();
            while (iterator != idx.end()) {
                auto item = *iterator;
                if (item.specifier == specifier && item.provider == provider) return false;
                iterator++;
            }
            return true;
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
	    
};

EOSIO_ABI(Registry, (newprovider)(addendpoint)(viewps)(viewes)(endpbyhash))
