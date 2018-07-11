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

        // table endpoint i64
        struct endpoint {
            uint64_t id;
            account_name provider;
            std::string specifier;
            key256 hash;
            std::vector<int64_t> constants;
            std::vector<uint64_t> parts;
            std::vector<uint64_t> dividers;

            uint64_t primary_key() const { return id; }
            uint64_t get_provider() const { return provider; }
            key256 get_hash() const { return hash; }
	    
            EOSLIB_SERIALIZE(endpoint, (id)(provider)(specifier)(constants)(parts)(dividers)(hash))
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
        void hashview(account_name provider, std::string specifier);


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
            print("\niterator ok.\n");
            while (iterator != idx.end()) {
                auto item = *iterator;
                print("\nitem ok.\n");
                if (item.specifier == specifier && item.provider == provider) return false;
                iterator++;
            }
            return true;
        }

        key256 hash(account_name provider, std::string specifier) {
            std::string provider_string = std::to_string(provider);
            std::string concatenated = provider_string + specifier;
            uint32_t result_size = concatenated.length() + 1;

            print_f("conc = %\n", concatenated);

            char *data = new char[result_size];
            strcpy(data, concatenated.c_str());

            const char *c_data = (const char*) data;
            print_f("data  = %\n size = %\n", c_data, result_size);

            checksum256 hash_result;
            sha256(data, result_size, &hash_result);

            for (uint32_t i = 0; i < sizeof(hash_result.hash); i++) {
                print_f("%", std::to_string(hash_result.hash[i]));
            }
            print("\n");

            delete [] data;
            return checksum256_to_key256(hash_result);
        }

        key256 checksum256_to_key256(checksum256 c) {
            uint128_t first_word = 0;
            uint128_t second_word = 0;

            uint32_t half_size = sizeof(c.hash) / 2;
            uint32_t byte_size = 8;
            for (uint32_t i = 0; i < half_size; i++) {
                first_word = (first_word << byte_size) + c.hash[(half_size - 1) - i];
                second_word = (second_word << byte_size) + c.hash[((half_size * 2) - 1) - i];
            }
            print_f("First word = %\n", first_word);
            print_f("Second word = %\n", second_word);

            std::array<uint128_t, 2> words = { first_word, second_word };
            return key256(words);
        }
	    
};

EOSIO_ABI(Registry, (newprovider)(addendpoint)(viewps)(viewes)(hashview))
