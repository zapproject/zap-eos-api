const Sleep = require('sleep');
const spawn = require('child_process').spawn;

const { Api, JsonRpc } = require('eosjs');
const fetch = require('node-fetch');                            // node only; not needed in browsers
const { TextDecoder, TextEncoder } = require('text-encoding');  // node, IE11 and IE Edge Browsers


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
   
    constructor({verbose, signature_provider}) {
        this.verbose = verbose;
        this.running = false;
        this.instance = null;
        this.signature_provider = signature_provider;
    }

    async run() {
        if (this.instance) {
            throw new Error('Test EOS node is already running.');
        }

        // use spawn function because nodeos has infinity output
        //         nodeos -e -p eosio \
        // --plugin eosio::producer_plugin \
        // --plugin eosio::chain_api_plugin \
        // --plugin eosio::http_plugin \
        // --plugin eosio::history_plugin \
        // --plugin eosio::history_api_plugin \
        // --access-control-allow-origin='*' \
        // --contracts-console \
        // --http-validate-host=false \
        // --verbose-http-errors 
        this.instance = spawn('nodeos', ['--plugin eosio::producer_plugin',
         '--plugin eosio::chain_api_plugin',
         '--plugin eosio::http_plugin',
         '--plugin eosio::history_plugin',
         '--plugin eosio::history_api_plugin',
         '--http-validate-host=false',
         '--verbose-http-errors',
         '--contracts-console', 
         '--delete-all-blocks', 
         '--filter-on=\'*\'',
         '--access-control-allow-origin=\'*\'']);

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
        
        let self = this;
        this.rpc = new JsonRpc('http://127.0.0.1:8888', { fetch });
        this.api = new Api({ rpc: self.rpc, signatureProvider: self.signature_provider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });
    }

    getRpc() {
        return this.rpc;
    }

    getApi() {
        return this.api;
    }

}

module.exports = Node;
