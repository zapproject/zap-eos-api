const eos_ecc = require('eosjs-ecc');

class Account {
    constructor({account_name}) {
        this._name = account_name;
        this._default_auth = 'active';
        this.isAccount = true;
    }

    fromPrivateKey({private_key}) {
        if (!eos_ecc.isValidPrivate(private_key)) {
            throw new Error("Private key is invalid.");
        }

        this.private_key = private_key;
        this.public_key = eos_ecc.privateToPublic(this.private_key);

        return this;
    }

    async register(api) {
        try {
        const result = await api.transact({
            actions: [{
              account: 'eosio',
              name: 'newaccount',
              authorization: [{
                actor: this.name,
                permission: 'active',
              }],
              data: {
                creator: 'eosio',
                name: this.name,
                owner: {
                  threshold: 1,
                  keys: [{
                    key: this.public_key,
                    weight: 1
                  }],
                  accounts: [],
                  waits: []
                },
                active: {
                  threshold: 1,
                  keys: [{
                    key: this.public_key,
                    weight: 1
                  }],
                  accounts: [],
                  waits: []
                },
              },
            },
            {
              account: 'eosio',
              name: 'buyrambytes',
              authorization: [{
                actor: this.name,
                permission: 'active',
              }],
              data: {
                payer: 'eosio',
                receiver: this.name,
                bytes: 8192,
              },
            },
            {
              account: 'eosio',
              name: 'delegatebw',
              authorization: [{
                actor: this.name,
                permission: 'active',
              }],
              data: {
                from: 'eosio',
                receiver: this.name,
                stake_net_quantity: '1.0000 SYS',
                stake_cpu_quantity: '1.0000 SYS',
                transfer: false,
              }
            }]
          }, {
            blocksBehind: 3,
            expireSeconds: 30,
          });
        } catch (e) {
            console.log(e);
        }

        return result;
    }

    get name() {
        return this._name;
    }

    set name(value) {
        this._name = value;
    }

    get default_auth() {
        return this._default_auth;
    }

    set default_auth(value) {
        this._default_auth = value;
    }
}

module.exports = Account;
