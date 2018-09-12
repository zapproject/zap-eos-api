const Sleep = require('sleep');
const Eos = require('eosjs');
const spawn = require('child_process').spawn;

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
    constructor({verbose, keyProvider}) {
        this.eos_test_config = {
            chainId: null, // 32 byte (64 char) hex string
            keyProvider: keyProvider, // WIF string or array of keys..
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