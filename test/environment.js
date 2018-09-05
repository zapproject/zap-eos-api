const Eos = require('eosjs');
const path = require('path');
const Sleep = require('sleep');
const spawn = require('child_process').spawn;
const execSync = require('child_process').execSync;


const PROJECT_PATH = path.join(__dirname + '/..');

//TODO: receive dynamically
const NODEOS_PATH = '/home/kostya/blockchain/eos/build/programs/nodeos';

// Params for waiting node startup
// Can be different for different hardware configurations
const STARTUP_BLOCK = 3;
const STARTUP_REQUESTS_DELAY = 100;
const STARTUP_TIMEOUT = 5000;

// helper function to promisify event
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
    let timeoutException = 'Timeout exception.';
    if (startTime.getTime() - currentTime.getTime() > timeout) {
        throw timeoutException
    }
}

// TODO: Generate keys and use eosjs to setup initial state of node
class Node {
    constructor(verbose, recompile) {
        this.verbose = verbose;
        this.recompile = recompile;

        this.ACC_TEST_PRIV_KEY = '5KfFufnUThaEeqsSeMPt27Poan5g8LUaEorsC1hHm1FgNJfr3sX';
        this.ACC_OWNER_PRIV_KEY = '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3';

        this.eos_test_config = {
            chainId: null, // 32 byte (64 char) hex string
            keyProvider: [this.ACC_OWNER_PRIV_KEY, this.ACC_TEST_PRIV_KEY], // WIF string or array of keys..
            httpEndpoint: 'http://127.0.0.1:8888',
            expireInSeconds: 60,
            broadcast: true,
            verbose: this.verbose, // API activity
            sign: true
        };

        this.isRunning = false;
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
            throw 'Test EOS node is already running.';
        }

        // use spawn function because nodeos has infinity output
        this.instance = spawn(NODEOS_PATH + '/nodeos', ['--contracts-console', '--delete-all-blocks', '--access-control-allow-origin=*']);

        // wait until node is running
        while (this.isRunning === false) {
            let message = (await waitEvent(this.instance.stderr, 'data')).toString();
            if (this.isRunning === false) {
                this.isRunning = true;
            }
        }

        if (this.verbose) console.log('Eos node is running.')
    }

    kill() {
        if (this.instance) {
            this.instance.kill();
            this.instance = null;
            this.isRunning = false;

            if (this.verbose) console.log('Eos node killed.');
        }
    }

    async init() {
        if (!this.isRunning) {
            throw 'Eos node must running to setup initial state.';
        }

        await this._waitNodeStartup(STARTUP_TIMEOUT);

        let options = {cwd: PROJECT_PATH.toString(), /*disable output*/ stdio: 'pipe'};
        if (this.recompile) {
            execSync('make -f makefile.self lcompile', options);
            execSync('make -f makefile.self generate_abi', options);
        }
        execSync('make -f makefile.self init_accs', options);
        execSync('make -f makefile.self deploy_all', options);
        execSync('make -f makefile.self grant_permissions', options);
        execSync('make -f makefile.self issue_tokens_for_testacc', options);
    }

    async connect() {
        if (!this.isRunning) {
            throw 'Eos node must running for establishing connection.';
        }

        await this._waitNodeStartup(STARTUP_TIMEOUT);
        return Eos(this.eos_test_config);
    }

    async restart() {
        this.kill();
        await this.run();
    }
}

module.exports = Node;
