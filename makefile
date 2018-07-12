WALLET_PWD = PW5KG8hve1T9HyfWUJJC5E5Q2vNSXEeQiLs6WpphFbwf2EAuAXPYW
PUBLIC_KEY = EOS7jEmggyJqyKitbsaTYELQCRN71jAhmhwX1Zv89NFZLzaKfrX7X
CONTRACTS_ACC = cs.owner
TEST_ACC = kostya.s
PROJECT_DIR = /home/kostya/blockchain/zap_eos_contracts
	
init_accs:
	cleos create account eosio $(CONTRACTS_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(TEST_ACC) $(PUBLIC_KEY)
	
build_contracts:
	-mkdir $(PROJECT_DIR)/build
	-mkdir $(PROJECT_DIR)/build/dispatcher
	-mkdir $(PROJECT_DIR)/build/registry
	eosiocpp -o $(PROJECT_DIR)/build/dispatcher/dispatcher.wast $(PROJECT_DIR)/src/dispatcher.cpp
	eosiocpp -g $(PROJECT_DIR)/build/dispatcher/dispatcher.abi $(PROJECT_DIR)/src/dispatcher.hpp
	eosiocpp -o $(PROJECT_DIR)/build/registry/registry.wast $(PROJECT_DIR)/src/registry.cpp
	eosiocpp -g $(PROJECT_DIR)/build/registry/registry.abi $(PROJECT_DIR)/src/registry.hpp
	
deploy:
	-cleos wallet unlock --password $(WALLET_PWD)
	-cleos set contract $(CONTRACTS_ACC) $(PROJECT_DIR)/build/dispatcher -p $(CONTRACTS_ACC)
	-cleos set contract $(CONTRACTS_ACC) $(PROJECT_DIR)/build/registry -p $(CONTRACTS_ACC)
