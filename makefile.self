#PROJEC DIRECTORY
PROJECT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
#EOS SOURCES DIRECTORY (SOURCES MUST BE BUILDED)
EOS_DIR = /home/kostya/blockchain/eos
#DIRECTORY OF DEFAULT TOKEN CONTRACT
TOKEN_DIR = $(EOS_DIR)/build/contracts/eosio.token

install:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/main
	
generate_abi:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/main
	eosiocpp -g $(PROJECT_DIR)/build/main/main.abi $(PROJECT_DIR)/src/main.hpp

lcompile:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/main
	eosiocpp -o $(PROJECT_DIR)/build/main/main.wast $(PROJECT_DIR)/src/main.cpp

#                          #
# SMART CONTRACTS COMMANDS #
#                          #

#TODO: Remove after tests
grant_permissions:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos set account permission $(TEST_ACC) active '{"threshold": 1,"keys": [{"key": "EOS8LV5e4oGMFwKkWJyfffdgYaD2JPbhmBcaLd4kkE3zTjr3hxk2C","weight": 1}],"accounts": [{"permission":{"actor":"zap.main","permission":"eosio.code"},"weight":1}]}' owner -p $(TEST_ACC)
	cleos set account permission $(MAIN_ACC) active '{"threshold": 1,"keys": [{"key": "EOS7Gj5sWupAArPgqTvM1B3cjENqbh2LeWMJi5z7byTiiy6hR6XRQ","weight": 1}],"accounts": [{"permission":{"actor":"zap.main","permission":"eosio.code"},"weight":1}]}' owner -p $(MAIN_ACC)




