const Eos = require('eosjs');
const spawn = require('child_process').spawn;

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

class Node {
    constructor(verbose) {
        this.verbose = verbose;
        this.NODEOS_PATH = '/home/kostya/blockchain/eos/build/programs/nodeos';

        this.ACC_TEST_PRIV_KEY = '5JesJCSXGys63ycUGqUiNRkAK6MfrqtLyhGDSJXg2kPunWgWbj3';
        this.ACC_OWNER_PRIV_KEY = '5K8eg5nreako7Q2gb1ASML4MRkXFEX9DCa1jhft2hQcu4texkpB';

        this.eos_test_config = {
            chainId: null, // 32 byte (64 char) hex string
            keyProvider: [this.ACC_TEST_PRIV_KEY, this.ACC_OWNER_PRIV_KEY], // WIF string or array of keys..
            httpEndpoint: 'http://127.0.0.1:8888',
            expireInSeconds: 60,
            broadcast: true,
            verbose: false, // API activity
            sign: true
        };

        this.isRunning = false;
        this.eos = null;
    }

    async initAccounts() {
        let block = await this.eos.getBlock(1);
        console.log(JSON.stringify(block));
    }

    async run() {
        if (this.instance) {
            throw 'Test EOS node is already running.';
        }

        // use spawn function because nodeos has infinity output
        this.instance = spawn(this.NODEOS_PATH + '/nodeos', ['--contracts-console', '--delete-all-blocks', '--access-control-allow-origin=*']);

        // read nodeos output by lines using waitEvent function
        // read until nodeos doesn't produce blocks
        // after nodeos starts to produce blocks we can connect to node using eosjs
        while (this.isRunning === false) {
            let message = (await waitEvent(this.instance.stderr, 'data')).toString();
            if (this.isRunning === false && message.indexOf('produce_block') > -1) {
                this.isRunning = true;
                this.eos = Eos(this.eos_test_config);
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

    async restart() {
        this.kill();
        await this.run();
    }
}

module.exports = Node;
