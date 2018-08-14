#MUST BE YOUR WALLET PASSWORDS
#PASSWORD FOR WALLET THAT WILL DEPLOY CONTRACTS
WALLET_PWD = PW5KDk8rKpGrNbVZbt6PG3HEWGHAqb5mj13qRNHemvCNaysFtN78n
#PUBLIC KEY OF ACCOUNTS FOR WALLET ABOVE
PUBLIC_KEY = EOS7yo4KpfPVGrigndBv9AHP4KMAkpAAUA93RUozZFMLp8eoX56BG
#ACCOUNTS THAT USES PUBLIC KEY ABOVE
MAIN_ACC = zap.main
TOKEN_ACC = zap.token

#PASSWORD FOR WALLET THAT WILL CREATE PROVIDER AND CALL BOND/UNBOND
TEST_WALLET_PWD = PW5Kkwogcis8LiiDbF5hZXKBzSHiKoYhF4FAwkZzKsoM6sEJL3jhA
#PUBLIC KEY OF ACCOUNTS FOR WALLET ABOVE
PUBLIC_KEY_USER_ACCS = EOS84zFiw5gCEhjtsDipPKHx3b89oZ5dk1Bu1CQ3RK9PCVMiQZCzk
#ACCOUNTS THAT USES PUBLIC KEY ABOVE
TEST_PROVIDER_ACC = provider
TEST_ACC = kostya.s

#PROJEC DIRECTORY
PROJECT_DIR = /home/kostya/blockchain/zap_eos_contracts
#EOS SOURCES DIRECTORY (SOURCEC MUST BE BUILDED)
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
	
deploy_all:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos set contract $(MAIN_ACC) $(PROJECT_DIR)/build/main -p $(MAIN_ACC)
	-cleos set contract $(TOKEN_ACC) $(TOKEN_DIR) -p $(TOKEN_ACC)
	-cleos push action $(TOKEN_ACC) create '["zap.token", "1000000000 TST"]' -p $(TOKEN_ACC)

deploy_token:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos set contract $(TOKEN_ACC) $(TOKEN_DIR) -p $(TOKEN_ACC)
	cleos push action $(TOKEN_ACC) create '["zap.token", "1000000000 TST"]' -p $(TOKEN_ACC)

deploy_main:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos set contract $(MAIN_ACC) $(PROJECT_DIR)/build/main -p $(MAIN_ACC)

#                          #
# SMART CONTRACTS COMMANDS #
#                          #

init_provider:
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos push action $(MAIN_ACC) newprovider '["provider", "tst01", 1]' -p $(TEST_PROVIDER_ACC)
	cleos push action $(MAIN_ACC) addendpoint '["provider", "tst01_endpoint01", [200, 3, 0], [0, 1000000], [1]]' -p $(TEST_PROVIDER_ACC)

bond:
	-cleos wallet lock -n default --password $(WALLET_PWD)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos push action $(MAIN_ACC) bond '["kostya.s", "provider", "tst01_endpoint01", 1]' -p $(TEST_ACC)

unbond:
	-cleos wallet lock -n default --password $(WALLET_PWD)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos push action $(MAIN_ACC) unbond '["kostya.s", "provider", "tst01_endpoint01", 1]' -p $(TEST_ACC)

init_accs:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos create account eosio $(MAIN_ACC) $(PUBLIC_KEY)
	cleos create account eosio $(TOKEN_ACC) $(PUBLIC_KEY)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos create account eosio $(TEST_ACC) $(PUBLIC_KEY_USER_ACCS)
	cleos create account eosio $(TEST_PROVIDER_ACC) $(PUBLIC_KEY_USER_ACCS)

grant_permissions:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	-cleos wallet unlock -n test --password $(TEST_WALLET_PWD)
	cleos set account permission $(TEST_ACC) active '{"threshold": 1,"keys": [{"key": "EOS8LV5e4oGMFwKkWJyfffdgYaD2JPbhmBcaLd4kkE3zTjr3hxk2C","weight": 1}],"accounts": [{"permission":{"actor":"zap.main","permission":"eosio.code"},"weight":1}]}' owner -p $(TEST_ACC)
	cleos set account permission $(MAIN_ACC) active '{"threshold": 1,"keys": [{"key": "EOS7jEmggyJqyKitbsaTYELQCRN71jAhmhwX1Zv89NFZLzaKfrX7X","weight": 1}],"accounts": [{"permission":{"actor":"zap.main","permission":"eosio.code"},"weight":1}]}' owner -p $(MAIN_ACC)

issue_tokens_for_testacc: 
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos push action $(TOKEN_ACC) issue '["kostya.s", "1000000 TST", ""]' -p $(TOKEN_ACC)

testacc_balance:
	-cleos wallet unlock -n default --password $(WALLET_PWD)
	cleos get currency balance $(TOKEN_ACC) $(TEST_ACC) "TST"



