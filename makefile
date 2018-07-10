WALLET_PWD = PW5KG8hve1T9HyfWUJJC5E5Q2vNSXEeQiLs6WpphFbwf2EAuAXPYW
ACC_NAME = dispatcher.c
PROJECT_DIR = /home/kostya/blockchain/zap_eos_contracts
	
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
	-cleos set contract $(ACC_NAME) $(PROJECT_DIR)/build/dispatcher -p $(ACC_NAME)
	-cleos set contract $(ACC_NAME) $(PROJECT_DIR)/build/registry -p $(ACC_NAME)
