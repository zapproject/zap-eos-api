#PROJEC DIRECTORY
PROJECT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

install:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/main
	-mkdir $(PROJECT_DIR)/build/token

compile:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/main
	-mkdir $(PROJECT_DIR)/build/token
	eosio-cpp -o $(PROJECT_DIR)/build/main/main.wasm $(PROJECT_DIR)/src/main.cpp --abigen
	eosio-cpp -o $(PROJECT_DIR)/build/token/eosio.token.wasm $(PROJECT_DIR)/src/eosio.token.cpp --abigen

genabi:
	eosio-abigen $(PROJECT_DIR)/src/main.cpp --contract=main --output=$(PROJECT_DIR)/build/main/main.abi
	eosio-abigen $(PROJECT_DIR)/src/eosio.token.cpp --contract=eosio.token --output=$(PROJECT_DIR)/build/token/eosio.token.abi

#                          #
# SMART CONTRACTS COMMANDS #
#                          #

#TODO: Remove after tests
grant_permissions:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos set account permission $(TEST_ACC) active '{"threshold": 1,"keys": [{"key": "EOS8LV5e4oGMFwKkWJyfffdgYaD2JPbhmBcaLd4kkE3zTjr3hxk2C","weight": 1}],"accounts": [{"permission":{"actor":"zap.main","permission":"eosio.code"},"weight":1}]}' owner -p $(TEST_ACC)
	cleos set account permission $(MAIN_ACC) active '{"threshold": 1,"keys": [{"key": "EOS7Gj5sWupAArPgqTvM1B3cjENqbh2LeWMJi5z7byTiiy6hR6XRQ","weight": 1}],"accounts": [{"permission":{"actor":"zap.main","permission":"eosio.code"},"weight":1}]}' owner -p $(MAIN_ACC)




