ZAP EOS SMART CONTRACTS
-

Software versions
 - EOS v1.1.x
 - CMake v3.5.0^
 - Nodejs
 - Eosjs
 - binaryen v37.0.0
 - Sleep
 - Chai
 - Mocha

 Commands
 - to run tests use '*npm test*'
 - to compile use '*cmake ./*' then '*make all*' or '*make -f makefile.self lcompile*'
 - to generate abi use '*make -f makefile.self generate_abi*'

Issues
- cmake generated .wasm file doesn't work, use '*lcompile*' command