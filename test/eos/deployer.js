const fs = require('fs');
const Account = require('./account.js');
const Transaction = require('./transaction.js');

class Deployer {
    constructor({eos, contract_name}) {
        this._eos = eos;
        this._contract_name = contract_name;
    }

    from(account) {
        if (!account.isAccount) {
            throw new Error('Account must be instance of account.js');
        }

        this._deployer_account = account;

        return this;
    }

    read(dir) {
        if (!dir) {
            throw new Error('Compiled contracts directory not specified');
        }

        this._wasm = fs.readFileSync(dir + '/' + this._contract_name + '.wasm');
        this._abi = fs.readFileSync(dir + '/' + this._contract_name + '.abi');

        return this;
    }

    afterDeploy(transaction) {
        if (!transaction.isTransaction) {
            throw new Error('Transaction must be instance of transaction.js');
        }

        this._after_deploy_tr = transaction;

        return this;
    }

    beforeDeploy(transaction) {
        if (!transaction.isTransaction) {
            throw new Error('Transaction must be instance of transaction.js');
        }

        this._before_deploy_tr = transaction;

        return this;
    }

    async deploy() {
        if (!this._wasm || !this._abi || !this._deployer_account) {
            throw new Error('Deployer not initialized');
        }

        if (this._before_deploy_tr) {
            await this._before_deploy_tr.execute(this._eos);
        }

        let result = [];
        // Publish contract to the blockchain
        result.push(await this._eos.setcode(this._deployer_account.name, 0, 0, this._wasm)); // @returns {Promise}
        result.push(await this._eos.setabi(this._deployer_account.name, JSON.parse(this._abi))); // @returns {Promise}

        if (this._after_deploy_tr) {
            await this._after_deploy_tr.execute(this._eos);
        }

        return result;
    }
}

module.exports = Deployer;