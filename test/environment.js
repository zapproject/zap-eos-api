const path = require('path');
const Node = require('./eos/eosnode.js');
const Account = require('./eos/account.js');
const Deployer = require('./eos/deployer.js');
const Transaction = require('./eos/transaction.js');
const EventListener = require('./eos/eventreader.js');

const execSync = require('child_process').execSync;


const PROJECT_PATH = path.join(__dirname + '/..');

//TODO: receive dynamically
const TOKEN_DIR = PROJECT_PATH + '/build/token';

const ACC_TEST_PRIV_KEY = '5KfFufnUThaEeqsSeMPt27Poan5g8LUaEorsC1hHm1FgNJfr3sX';
const ACC_OWNER_PRIV_KEY = '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3';


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


class TestNode extends Node {
    constructor(verbose, recompile) {
        super({verbose: verbose, key_provider: [ACC_TEST_PRIV_KEY, ACC_OWNER_PRIV_KEY]});

        this.recompile = recompile;

        this.account_user = new Account({account_name: 'user'}).fromPrivateKey({private_key: ACC_TEST_PRIV_KEY});
        this.account_provider = new Account({account_name: 'provider'}).fromPrivateKey({private_key: ACC_TEST_PRIV_KEY});
        this.account_token = new Account({account_name: 'zap.token'}).fromPrivateKey({private_key: ACC_OWNER_PRIV_KEY});
        this.account_main = new Account({account_name: 'zap.main'}).fromPrivateKey({private_key: ACC_OWNER_PRIV_KEY});
    }

    async init() {
        if (!this.running) {
            throw new Error('Eos node must running receiver setup initial state.');
        }

        if (this.recompile) {
            await this.compile();
        }

        let eos = await this.connect();
        await this.registerAccounts(eos);
        await this.deploy(eos);
        await this.issueTokens(eos);
        await this.grantPermissions(eos);
    }

    async compile() {
        let options = {cwd: PROJECT_PATH.toString(), /*disable output*/ stdio: 'pipe'};
        execSync('make -f makefile.self compile', options);
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
            await new Deployer({eos: eos, contract_name: 'main'})
                .from(this.account_main)
                .read(PROJECT_PATH + '/build/main')
                .deploy()
        );

        let createTokenTransaction = new Transaction()
            .sender(this.account_token)
            .receiver(this.account_token)
            .action('create')
            .data({issuer: this.account_token.name, maximum_supply: '1000000000 TST'});

        results.push(
            await new Deployer({eos: eos, contract_name: 'eosio.token'})
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


    getAccounts() {
        return {
            user: this.account_user,
            provider: this.account_provider,
            token: this.account_token,
            main: this.account_main
        };
    }

    getEventListener() {
        return new EventListener(this, 1);
    }
}

module.exports = TestNode;
