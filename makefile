#MUST BE YOUR WALLET PASSWORDS
#PASSWORD FOR WALLET THAT WILL DEPLOY CONTRACTS
WALLET_PWD = PW5KG8hve1T9HyfWUJJC5E5Q2vNSXEeQiLs6WpphFbwf2EAuAXPYW
#PUBLIC KEY OF ACCOUNTS FOR WALLET ABOVE
PUBLIC_KEY = EOS7jEmggyJqyKitbsaTYELQCRN71jAhmhwX1Zv89NFZLzaKfrX7X
#ACCOUNTS THAT USES PUBLIC KEY ABOVE
REGISTRY_ACC = zap.registry
BONDAGE_ACC = zap.bondage
DISPATCH_ACC = zap.dispatch
TOKEN_ACC = zap.token

#PASSWORD FOR WALLET THAT WILL CREATE PROVIDER AND CALL BOND/UNBOND
TEST_WALLET_PWD = PW5KDd3voZ78YAECFSbz1E5KQA1xS8GNAmTqQvicU81nmii7NstPA
#PUBLIC KEY OF ACCOUNTS FOR WALLET ABOVE
PUBLIC_KEY_USER_ACCS = EOS8LV5e4oGMFwKkWJyfffdgYaD2JPbhmBcaLd4kkE3zTjr3hxk2C
#ACCOUNTS THAT USES PUBLIC KEY ABOVE
TEST_PROVIDER_ACC = provider
TEST_ACC = kostya.s

#PROJEC DIRECTORY
PROJECT_DIR = /home/kostya/blockchain/zap_eos_contracts
#EOS SOURCES DIRECTORY (SOURCEC MUST BE BUILDED)
EOS_DIR = /home/kostya/blockchain/eos
#DIRECTORY OF DEFAULT TOKEN CONTRACT
TOKEN_DIR = $(EOS_DIR)/build/contracts/eosio.token

init_provider:
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos push action $(REGISTRY_ACC) newprovider '["provider", "tst01", 1]' -p $(TEST_PROVIDER_ACC)
	cleos push action $(REGISTRY_ACC) addendpoint '["provider", "tst01_endpoint01", [200, 3, 0], [0, 1000000], [1]]' -p $(TEST_PROVIDER_ACC)

bond:
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos push action $(BONDAGE_ACC) bond '["kostya.s", "provider", "tst01_endpoint01", 1]' -p $(TEST_ACC)

unbond:
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos push action $(BONDAGE_ACC) unbond '["kostya.s", "provider", "tst01_endpoint01", 1]' -p $(TEST_ACC)

init_accs:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos create account eosio $(REGISTRY_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(BONDAGE_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(TOKEN_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(DISPATCH_ACC) $(PUBLIC_KEY)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos create account eosio $(TEST_ACC) $(PUBLIC_KEY_USER_ACCS)
	cleos create account eosio $(TEST_PROVIDER_ACC) $(PUBLIC_KEY_USER_ACCS)

grant_permissions:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos set account permission $(TEST_ACC) active '{"threshold": 1,"keys": [{"key": "EOS8LV5e4oGMFwKkWJyfffdgYaD2JPbhmBcaLd4kkE3zTjr3hxk2C","weight": 1}],"accounts": [{"permission":{"actor":"zap.bondage","permission":"eosio.code"},"weight":1}]}' owner -p $(TEST_ACC)
	cleos set account permission $(BONDAGE_ACC) active '{"threshold": 1,"keys": [{"key": "EOS7jEmggyJqyKitbsaTYELQCRN71jAhmhwX1Zv89NFZLzaKfrX7X","weight": 1}],"accounts": [{"permission":{"actor":"zap.bondage","permission":"eosio.code"},"weight":1}]}' owner -p $(BONDAGE_ACC)

issue_tokens_for_testacc: 
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos push action $(TOKEN_ACC) issue '["kostya.s", "1000000 TST", ""]' -p $(TOKEN_ACC)

testacc_balance:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos get currency balance $(TOKEN_ACC) $(TEST_ACC) "TST"
	
build_all:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/dispatcher
	-mkdir $(PROJECT_DIR)/build/registry
	-mkdir $(PROJECT_DIR)/build/bondage
	eosiocpp -o $(PROJECT_DIR)/build/registry/registry.wast $(PROJECT_DIR)/src/registry.cpp
	eosiocpp -g $(PROJECT_DIR)/build/registry/registry.abi $(PROJECT_DIR)/src/registry.hpp
	eosiocpp -o $(PROJECT_DIR)/build/bondage/bondage.wast $(PROJECT_DIR)/src/bondage.cpp
	eosiocpp -g $(PROJECT_DIR)/build/bondage/bondage.abi $(PROJECT_DIR)/src/bondage.hpp

build_bondage:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/bondage
	eosiocpp -o $(PROJECT_DIR)/build/bondage/bondage.wast $(PROJECT_DIR)/src/bondage.cpp
	eosiocpp -g $(PROJECT_DIR)/build/bondage/bondage.abi $(PROJECT_DIR)/src/bondage.hpp

build_registry:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/registry
	eosiocpp -o $(PROJECT_DIR)/build/registry/registry.wast $(PROJECT_DIR)/src/registry.cpp
	eosiocpp -g $(PROJECT_DIR)/build/registry/registry.abi $(PROJECT_DIR)/src/registry.hpp
	
deploy_all:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos set contract $(REGISTRY_ACC) $(PROJECT_DIR)/build/registry -p $(REGISTRY_ACC)
	-cleos set contract $(BONDAGE_ACC) $(PROJECT_DIR)/build/bondage -p $(BONDAGE_ACC)
	-cleos set contract $(TOKEN_ACC) $(TOKEN_DIR) -p $(TOKEN_ACC)
	-cleos push action $(TOKEN_ACC) create '["zap.token", "1000000000 TST"]' -p $(TOKEN_ACC)

deploy_token:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos set contract $(TOKEN_ACC) $(TOKEN_DIR) -p $(TOKEN_ACC)
	cleos push action $(TOKEN_ACC) create '["zap.token", "1000000000 TST"]' -p $(TOKEN_ACC)

deploy_bondage:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos set contract $(BONDAGE_ACC) $(PROJECT_DIR)/build/bondage -p $(BONDAGE_ACC)

deploy_registry:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos set contract $(REGISTRY_ACC) $(PROJECT_DIR)/build/registry -p $(REGISTRY_ACC)



