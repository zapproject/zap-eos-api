WALLET_PWD = PW5KG8hve1T9HyfWUJJC5E5Q2vNSXEeQiLs6WpphFbwf2EAuAXPYW
PUBLIC_KEY = EOS7jEmggyJqyKitbsaTYELQCRN71jAhmhwX1Zv89NFZLzaKfrX7X
REGISTRY_ACC = zap.registry
BONDAGE_ACC = zap.bondage
TEST_ACC = kostya.s
TOKEN_ACC = zap.token
PROJECT_DIR = /home/kostya/blockchain/zap_eos_contracts
EOS_DIR = /home/kostya/blockchain/eos
TOKEN_DIR = $(EOS_DIR)/build/contracts/eosio.token

init_provider:
	cleos push action $(REGISTRY_ACC) newprovider '["kostya.s", "tst01", 1]' -p $(TEST_ACC)
	cleos push action $(REGISTRY_ACC) addendpoint '["kostya.s", "tst01_endpoint01", [200, 3, 0], [0, 1000000], [1]]' -p $(TEST_ACC)
	
init_accs:
	-cleos wallet unlock --password $(WALLET_PWD)
	cleos create account eosio $(REGISTRY_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(BONDAGE_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(TEST_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(TOKEN_ACC) $(PUBLIC_KEY)

grant_permissions:
	cleos set account permission $(TEST_ACC) active '{"threshold": 1,"keys": [{"key": "EOS7jEmggyJqyKitbsaTYELQCRN71jAhmhwX1Zv89NFZLzaKfrX7X","weight": 1}],"accounts": [{"permission":{"actor":"zap.bondage","permission":"eosio.code"},"weight":1}]}' owner -p $(TEST_ACC)

issue_tokens_for_testacc: 
	cleos push action $(TOKEN_ACC) issue '["kostya.s", "1000000 TST", ""]' -p $(TOKEN_ACC)

testacc_balance:
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
	-cleos wallet unlock --password $(WALLET_PWD)
	-cleos set contract $(REGISTRY_ACC) $(PROJECT_DIR)/build/registry -p $(REGISTRY_ACC)
	-cleos set contract $(BONDAGE_ACC) $(PROJECT_DIR)/build/bondage -p $(BONDAGE_ACC)

deploy_token:
	-cleos wallet unlock --password $(WALLET_PWD)
	cleos set contract $(TOKEN_ACC) $(TOKEN_DIR) -p $(TOKEN_ACC)
	cleos push action $(TOKEN_ACC) create '["zap.token", "1000000000 TST"]' -p $(TOKEN_ACC)

deploy_bondage:
	-cleos wallet unlock --password $(WALLET_PWD)
	-cleos set contract $(BONDAGE_ACC) $(PROJECT_DIR)/build/bondage -p $(BONDAGE_ACC)

deploy_registry:
	-cleos wallet unlock --password $(WALLET_PWD)
	-cleos set contract $(REGISTRY_ACC) $(PROJECT_DIR)/build/registry -p $(REGISTRY_ACC)



