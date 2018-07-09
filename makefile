WALLET_PWD = PW5KG8hve1T9HyfWUJJC5E5Q2vNSXEeQiLs6WpphFbwf2EAuAXPYW
ACC_NAME = dispatcher.c
CONTRACTS_PATH = ../

build:
	eosiocpp -o dispatcher.wast dispatcher.cpp
	eosiocpp -g dispatcher.abi dispatcher.hpp
	eosiocpp -o registry.wast registry.cpp
	eosiocpp -g registry.abi registry.hpp
deploy:
	$(MAKE) deploy_with_unlock || $(MAKE) deploy_only
deploy_with_unlock:
	cleos wallet unlock --password $(WALLET_PWD)
	cleos set contract $(ACC_NAME) ./ -p $(ACC_NAME)
deploy_only:
	echo Account is unlocked. Trying to deploy...
	cleos set contract $(ACC_NAME) ./ -p $(ACC_NAME)

