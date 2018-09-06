const Eos = require('eosjs');
const path = require('path');
const Sleep = require('sleep');
const Account = require('./account.js');
const Deployer = require('./deployer.js');
const Transaction = require('./transaction.js');

const spawn = require('child_process').spawn;
const execSync = require('child_process').execSync;


const PROJECT_PATH = path.join(__dirname + '/..');

//TODO: receive dynamically
const NODEOS_PATH = '/home/kostya/blockchain/eos/build/programs/nodeos';
const EOS_DIR = '/home/kostya/blockchain/eos';
const TOKEN_DIR = EOS_DIR + '/build/contracts/eosio.token';

// Params for waiting node startup
// Can be different for different hardware configurations
const STARTUP_BLOCK = 3;
const STARTUP_REQUESTS_DELAY = 100;
const STARTUP_TIMEOUT = 5000;

// helper function receiver promisify event
function waitEvent(event, type) {
    return new Promise(function(resolve, reject) {
        function listener(data) {
            event.removeListener(type, listener);
            resolve(data);
        }

        event.on(type, listener);
    });
}

function checkTimeout(startTime, timeout) {
    let currentTime = new Date();
    let timeoutException = new Error('Timeout exception.');
    if (startTime.getTime() - currentTime.getTime() > timeout) {
        throw timeoutException
    }
}

function findElement(array, field, value) {
    for (let i in array) {
        if (array.hasOwnProperty(i)) {
            if (array[i][field] === value) {
                return i;
            }
        }
    }

    return -1;
}


class Node {
    constructor(verbose, recompile) {
        this.verbose = verbose;
        this.recompile = recompile;

        this.ACC_TEST_PRIV_KEY = '5KfFufnUThaEeqsSeMPt27Poan5g8LUaEorsC1hHm1FgNJfr3sX';
        this.ACC_OWNER_PRIV_KEY = '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3';

        this.account_user = new Account({account_name: 'user'}).fromPrivateKey({private_key: this.ACC_TEST_PRIV_KEY});
        this.account_provider = new Account({account_name: 'provider'}).fromPrivateKey({private_key: this.ACC_TEST_PRIV_KEY});
        this.account_token = new Account({account_name: 'zap.token'}).fromPrivateKey({private_key: this.ACC_OWNER_PRIV_KEY});
        this.account_main = new Account({account_name: 'zap.main'}).fromPrivateKey({private_key: this.ACC_OWNER_PRIV_KEY});

        this.eos_test_config = {
            chainId: null, // 32 byte (64 char) hex string
            keyProvider: [this.ACC_OWNER_PRIV_KEY, this.ACC_TEST_PRIV_KEY], // WIF string or array of keys..
            httpEndpoint: 'http://127.0.0.1:8888',
            expireInSeconds: 60,
            broadcast: true,
            verbose: this.verbose, // API activity
            sign: true
        };

        this.running = false;
    }

    async _waitNodeStartup(timeout) {
        // wait for block production
        let startTime = new Date();
        while (true) {
            let eos = Eos(this.eos_test_config);
            try {
                let res = await eos.getInfo({});
                if (res.head_block_producer) {
                    while (true) {
                        try {
                            await eos.getBlock(STARTUP_BLOCK);
                            break;
                        } catch (e) {
                            Sleep.msleep(STARTUP_REQUESTS_DELAY);
                            checkTimeout(startTime, timeout);
                        }
                    }
                    break;
                }
            } catch (e) {
                Sleep.msleep(STARTUP_REQUESTS_DELAY);
                checkTimeout(startTime, timeout);
            }
        }
    }

    async run() {
        if (this.instance) {
            throw new Error('Test EOS node is already running.');
        }

        // use spawn function because nodeos has infinity output
        this.instance = spawn(NODEOS_PATH + '/nodeos', ['--contracts-console', '--delete-all-blocks', '--access-control-allow-origin=*']);

        // wait until node is running
        while (this.running === false) {
            await waitEvent(this.instance.stderr, 'data');
            if (this.running === false) {
                this.running = true;
            }
        }

        if (this.verbose) console.log('Eos node is running.')
    }

    kill() {
        if (this.instance) {
            this.instance.kill();
            this.instance = null;
            this.running = false;

            if (this.verbose) console.log('Eos node killed.');
        }
    }

    async init(useMakefile) {
        if (!this.running) {
            throw new Error('Eos node must running receiver setup initial state.');
        }

        await this._waitNodeStartup(STARTUP_TIMEOUT);

        if (this.recompile) {
           await this.compile();
        }

        if (useMakefile) {
            let options = {cwd: PROJECT_PATH.toString(), /*disable output*/ stdio: 'pipe'};
            execSync('make -f makefile.self init_accs', options);
            execSync('make -f makefile.self deploy_all', options);
            execSync('make -f makefile.self grant_permissions', options);
            execSync('make -f makefile.self issue_tokens_for_testacc', options);
        } else {
            let eos = await this.connect();
            await this.registerAccounts(eos);
            await this.deploy(eos);
            await this.issueTokens(eos);
            await this.grantPermissions(eos);
        }
    }

    async compile() {
        let options = {cwd: PROJECT_PATH.toString(), /*disable output*/ stdio: 'pipe'};
        execSync('make -f makefile.self lcompile', options);
        execSync('make -f makefile.self generate_abi', options);
    }

    async registerAccounts(eos) {
        let results = [];
        results.push(await this.account_user.register(eos));
        results.push(await this.account_provider.register(eos));
        results.push(await this.account_token.register(eos));
        results.push(await this.account_main.register(eos));

        return results;
    }

    async deploy(eos) {
        let results = [];

        results.push(
            await new Deployer({eos: eos, contract_name: 'main', verbose: false})
                .from(this.account_main)
                .read()
                .deploy()
        );

        let createTokenTransaction = new Transaction()
            .sender(this.account_token)
            .receiver(this.account_token)
            .action('create')
            .data({issuer: this.account_token.name, maximum_supply: '1000000000 TST'})
            .build();

        results.push(
            await new Deployer({eos: eos, contract_name: 'eosio.token', verbose: false})
                .from(this.account_token)
                .read(TOKEN_DIR)
                .afterDeploy(createTokenTransaction)
                .deploy()
        );

        return results;
    }

    // TODO: compare with makefile command result
    async grantPermissions(eos) {
        let newPermission = {
            permission: {
                actor: this.account_main.name,
                permission: 'eosio.code'
            },
            weight: 1
        };

        let user = await eos.getAccount(this.account_user.name);
        let main = await eos.getAccount(this.account_main.name);

        let newUserAuth = user.permissions[findElement(user.permissions, 'perm_name', 'active')];
        newUserAuth.required_auth.accounts.push(newPermission);

        let newMainAuth = main.permissions[findElement(main.permissions, 'perm_name', 'active')];
        newMainAuth.required_auth.accounts.push(newPermission);

        await eos.transaction(tr => {
                tr.updateauth({
                    account: user.account_name,
                    permission: 'active',
                    parent: 'owner',
                    auth: newUserAuth.required_auth
                }, {authorization: `${user.account_name}@owner`});

                tr.updateauth({
                    account: main.account_name,
                    permission: 'active',
                    parent: 'owner',
                    auth: newMainAuth.required_auth
                }, {authorization: `${main.account_name}@owner`});
            }
        );
    }

    async issueTokens(eos) {
        return await new Transaction()
            .sender(this.account_token)
            .receiver(this.account_token)
            .action('issue')
            .data({to: this.account_user.name, quantity: '1000000 TST', memo: ''})
            .execute(eos);
    }

    async connect() {
        if (!this.running) {
            throw new Error('Eos node must running for establishing connection.');
        }

        await this._waitNodeStartup(STARTUP_TIMEOUT);
        return Eos(this.eos_test_config);
    }

    async restart() {
        this.kill();
        await this.run();
    }

    getAccounts() {
        return {
            user: this.account_user,
            provider: this.account_provider,
            token: this.account_token,
            main: this.account_main
        };
    }
}

module.exports = Node;
