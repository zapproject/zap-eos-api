const Sleep = require('sleep');
const Eos = require('eosjs');
const spawn = require('child_process').spawn;
const tkill = require('tree-kill');

const STARTUP_TIMEOUT = 30000;
const STARTUP_REQUESTS_DELAY = 100;
const STARTUP_BLOCK = 3;

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


class Node {
    constructor({verbose, key_provider}) {
        this.eos_test_config = {
            chainId: null, // 32 byte (64 char) hex string
            keyProvider: key_provider, // WIF string or array of keys..
            httpEndpoint: 'http://127.0.0.1:8888',
            expireInSeconds: 60,
            broadcast: true,
            verbose: verbose, // API activity
            sign: true
        };

        this.verbose = verbose;
        this.running = false;
        this.instance = null;
    }

    async _waitNodeStartup(timeout) {
        // wait for block production
        let startTime = new Date();
        let eos = Eos(this.eos_test_config);
        while (true) {
            try {
                let res = await eos.getInfo({});
                if (res.head_block_producer) {
                    while (true) {
                        try {
                            let res = await eos.getBlock(STARTUP_BLOCK);
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
        this.instance = this.instance = spawn('nodeos', 
        ['-e -p eosio', '--delete-all-blocks', '--contracts-console', '--plugin eosio::producer_plugin', '--plugin eosio::history_plugin', '--plugin eosio::chain_api_plugin', '--plugin eosio::history_api_plugin', '--plugin eosio::http_plugin'],
        {shell: true, detached: true});
         
        // -e -p eosio --plugin eosio::producer_plugin --plugin eosio::chain_api_plugin --plugin eosio::http_plugin --plugin eosio::history_plugin --plugin eosio::history_api_plugin --access-control-allow-origin='*' --contracts-console --http-validate-host=false --verbose-http-errors --filter-on='*' --delete-all-blocks
        
        // this.instance.stdout.on('data', function(data) {
        //     console.log(data.toString()); 
        // });

        // this.instance.stderr.on('data', function(data) {
        //     console.log(data.toString()); 
        // });
        


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
            tkill(this.instance.pid);
            this.instance = null;
            this.running = false;
            if (this.verbose) console.log('Eos node killed.');
        }
    }

    setRunning() {
        this.running = true;
    }

    async restart() {
        this.kill();
        await this.run();
    }

    async connect() {
        if (!this.running) {
            throw new Error('Eos node must running for establishing connection.');
        }
        await this._waitNodeStartup(STARTUP_TIMEOUT);
        return Eos(this.eos_test_config);
    }

}

module.exports = Node;