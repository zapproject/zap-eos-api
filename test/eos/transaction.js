const Account = require('./account.js');

class Transaction {
    /*
    {
    // ...headers,
    // context_free_actions: [],
    actions: [
      {
        account: 'eosio.token',
        name: 'transfer',
        authorization: [{
          actor: 'inita',
          permission: 'active'
        }],
        data: {
          from: 'inita',
          to: 'initb',
          quantity: '7.0000 SYS',
          memo: ''
        }
       }
     ]
    }
    */

    constructor() {
        this.actions = [{}];
    }

    sender(account) {
        if (!(account instanceof Account)) {
            throw new Error('Account must be instance of account.js');
        }

        this.actions[0]['authorization'][0] = {
            actor: account.name,
            permission: account.default_auth
        };

        return this;
    }

    receiver(account) {
        if (!(account instanceof Account)) {
            throw new Error('Account must be instance of account.js');
        }

        this.actions[0].account = account.name;
        return this;
    }

    action({action}) {
        this.actions[0].name = action;
        return this;
    }

    data({data}) {
        this.actions[0].data = data;
        return this;
    }

    merge(transaction) {
        if (!(transaction instanceof Transaction)) {
            throw new Error('Account must be instance of account.js');
        }

        for (let i in transaction.actions) {
            if (transaction.actions.hasOwnProperty(i)) {
                this.actions.push(transaction.actions[i]);
            }
        }
    }


    build() {
        return { actions: this.actions };
    }

    async execute(eos) {
        return await eos.transaction({ actions: this.actions })
    }
}

module.exports = Transaction;