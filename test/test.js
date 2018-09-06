const Node = require('./environment.js');
const expect = require('chai')
    .use(require('chai-as-promised'))
    .expect;
const Transaction = require('./transaction.js');

async function configureEnvironment(func) {
    await func();
}

describe('Main', function () {

    describe('EOS Node', function () {
        let node = new Node();

        beforeEach(function (done) {
            this.timeout(30000);
            node.kill();
            done();

        });

        it('#run()', async () => {
            this.timeout(20000);
            await node.run();
            await expect(node.running).to.be.equal(true);
        });

        it('#restart()', async () => {
            this.timeout(20000);
            await node.restart();
            await expect(node.running).to.be.equal(true);
        });

        it('#kill()', async () => {
            this.timeout(20000);
            await node.run();
            node.kill();
            await expect(node.running).to.be.equal(false);
        });
    });

    describe('Environment', function () {
        let node = new Node(false, false);

        function findElement(array, field, value) {
            for (let i in array) {
                if (array.hasOwnProperty(i)) {
                    if (array[i][field] === value) {
                        return i;
                    }
                }
            }

            return null;
        }

        beforeEach(function (done) {
            this.timeout(30000);
            configureEnvironment(async () => {
                try {
                    await node.restart();
                } catch (e) {
                    console.log(e);
                }
                done();
            });
        });

        it('#transaction()', async () => {
            let accounts = node.getAccounts();
            let transferTransaction = new Transaction()
                .receiver(accounts.token)
                .action('transfer')
                .auth(accounts.user.getAuth('active'))
                .data({from: accounts.user.name, to: accounts.provider.name, quantity: '7 TST', memo: 'hi'})
                .build();

            let manualCreatedTransaction = {
                actions: [
                    {
                        account: accounts.token.name,
                        name: 'transfer',
                        authorization: [accounts.user.getAuth('active')],
                        data: {
                            from: accounts.user.name,
                            to: accounts.provider.name,
                            quantity: '7 TST',
                            memo: 'hi'
                        }
                    }
                ]
            };

            await expect(JSON.stringify(transferTransaction)).to.be.equal(JSON.stringify(manualCreatedTransaction));
        });

        it('#accounts()', async () => {
            let eos = await node.connect();
            let accounts = node.getAccounts();
            let res = await node.registerAccounts(eos);
            for (let i in res) {
                await expect(res[i].processed.receipt.status).to.be.equal('executed');
            }
            for (let role in accounts) {
                let request = await eos.getAccount(accounts[role].name);
                await expect(request.account_name).to.be.equal(accounts[role].name);
            }
        }).timeout(5000);

        it('#deploy()', async () => {
            let eos = await node.connect();
            await node.registerAccounts(eos);
            let res = await node.deploy(eos);
            for (let i in res) {
                for (let j in res[i]) {
                    await expect(res[i][j].processed.receipt.status).to.be.equal('executed');
                }
            }
        }).timeout(20000);

        it('#issueTokens()', async () => {
            let eos = await node.connect();
            await node.registerAccounts(eos);
            await node.deploy(eos);
            let res = await node.issueTokens(eos);
            await expect(res.processed.receipt.status).to.be.equal('executed');
        }).timeout(20000);

        it('#grantPermissions()', async () => {
            let eos = await node.connect();
            await node.registerAccounts(eos);
            await node.deploy(eos);
            await node.grantPermissions(eos);

            let user = await eos.getAccount(node.getAccounts().user.name);
            let main = await eos.getAccount(node.getAccounts().main.name);

            await expect(user.permissions[findElement(user.permissions, 'perm_name', 'active')].required_auth.accounts).to.have.lengthOf(1);
            await expect(main.permissions[findElement(main.permissions, 'perm_name', 'active')].required_auth.accounts).to.have.lengthOf(1);
        }).timeout(20000);

        it('#init()', async () => {
            await node.init(false);
        }).timeout(20000);


        after(function () {
            node.kill();
        })
    });

    describe('Contracts', function () {
        let node = new Node(false, false);

        beforeEach(function (done) {
            this.timeout(30000);
            configureEnvironment(async () => {
                try {
                    await node.restart();
                    await node.init(false);
                } catch (e) {
                    console.log(e);
                }
                done();
            });
        });

        it('#transfer()', async () => {
            let eos = await node.connect();

            await new Transaction()
                .sender(node.getAccounts().user)
                .receiver(node.getAccounts().token)
                .action('transfer')
                .data({
                    from: node.getAccounts().user.name,
                    to: node.getAccounts().provider.name,
                    quantity: '7 TST',
                    memo: 'hi'
                })
                .execute(eos);
            let tokensAmount = await eos.getCurrencyBalance('zap.token', 'provider', 'TST');
            await expect(tokensAmount[0].toString()).to.be.equal('7 TST');
        });

        it('#newprovider()', async () => {
            let eos = await node.connect();

            let res = await new Transaction()
                .sender(node.getAccounts().provider)
                .receiver(node.getAccounts().main)
                .action('newprovider')
                .data({provider: node.getAccounts().provider.name, title: 'test', public_key: 1})
                .execute(eos);
            await expect(res.processed.receipt.status).to.be.equal('executed');
        });

        it('#addendpoint()', async () => {
            let eos = await node.connect();

            await new Transaction()
                .sender(node.getAccounts().provider)
                .receiver(node.getAccounts().main)
                .action('newprovider')
                .data({provider: node.getAccounts().provider.name, title: 'test', public_key: 1})
                .execute(eos);
            let res = await new Transaction()
                .sender(node.getAccounts().provider)
                .receiver(node.getAccounts().main)
                .action('addendpoint')
                .data({
                    provider: node.getAccounts().provider.name,
                    specifier: 'test_endpoint',
                    constants: [200, 3, 0],
                    parts: [0, 1000000],
                    dividers: [1]
                })
                .execute(eos);
            await expect(res.processed.receipt.status).to.be.equal('executed');

        });

        it('#bond()', async () => {
            let eos = await node.connect();
            await new Transaction()
                .sender(node.getAccounts().provider)
                .receiver(node.getAccounts().main)
                .action('newprovider')
                .data({provider: node.getAccounts().provider.name, title: 'test', public_key: 1})
                .execute(eos);
            await new Transaction()
                .sender(node.getAccounts().provider)
                .receiver(node.getAccounts().main)
                .action('addendpoint')
                .data({
                    provider: node.getAccounts().provider.name,
                    specifier: 'test_endpoint',
                    constants: [200, 3, 0],
                    parts: [0, 1000000],
                    dividers: [1]
                })
                .execute(eos);
            let res = await new Transaction()
                .sender(node.getAccounts().user)
                .receiver(node.getAccounts().main)
                .action('bond')
                .data({
                    subscriber: node.getAccounts().user.name,
                    provider: node.getAccounts().provider.name,
                    endpoint: 'test_endpoint',
                    dots: 1
                })
                .execute(eos);
            await expect(res.processed.receipt.status).to.be.equal('executed');

        });

        it('#unbond()', async () => {
            let eos = await node.connect();

            await new Transaction()
                .sender(node.getAccounts().provider)
                .receiver(node.getAccounts().main)
                .action('newprovider')
                .data({provider: node.getAccounts().provider.name, title: 'test', public_key: 1})
                .execute(eos);
            await new Transaction()
                .sender(node.getAccounts().provider)
                .receiver(node.getAccounts().main)
                .action('addendpoint')
                .data({
                    provider: node.getAccounts().provider.name,
                    specifier: 'test_endpoint',
                    constants: [200, 3, 0],
                    parts: [0, 1000000],
                    dividers: [1]
                })
                .execute(eos);
            await new Transaction()
                .sender(node.getAccounts().user)
                .receiver(node.getAccounts().main)
                .action('bond')
                .data({
                    subscriber: node.getAccounts().user.name,
                    provider: node.getAccounts().provider.name,
                    endpoint: 'test_endpoint',
                    dots: 1
                })
                .execute(eos);
            let res = await new Transaction()
                .sender(node.getAccounts().user)
                .receiver(node.getAccounts().main)
                .action('unbond')
                .data({
                    subscriber: node.getAccounts().user.name,
                    provider: node.getAccounts().provider.name,
                    endpoint: 'test_endpoint',
                    dots: 1
                })
                .execute(eos);
            await expect(res.processed.receipt.status).to.be.equal('executed');

        });

        after(function () {
            node.kill();
        })
    });
});
