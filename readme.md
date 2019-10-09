ZAP EOS SMART CONTRACTS
-

Software versions
 - EOS v1.5.0
 - EOS CDT v1.4.1
 - CMake v3.5.0^
 - Nodejs
 - Eosjs
 - binaryen v37.0.0
 - Sleep
 - Chai
 - Mocha

How to run:
- docker-compose up
- docker-compose run eos bash -c "cd /app && bash ./rebuild.sh"
- docker-compose run eos bash -c "cd /app && npm install"
- docker-compose run eos bash -c "cd /app && npm test"
