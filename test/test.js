const Node = require('./environment.js');
const expect = require('chai')
    .use(require('chai-as-promised'))
    .expect;

async function configureEnvironment(func) {
    await func();
}

const userAuth = {
    actor: 'kostya.s',
    permission: 'active'
};

const providerAuth = {
    actor: 'provider',
    permission: 'active'
};

function transferActions({actor, permission}, {to, amount, memo}) {
    return [
        {
            account: 'zap.token',
            name: 'transfer',
            authorization: [{
                actor: actor,
                permission: permission
            }],
            data: {
                from: actor,
                to: to,
                quantity: amount + ' TST',
                memo: memo
            }
        }
    ]
}

function newProviderActions({actor, permission}, {title, key}) {
    return [
        {
            account: 'zap.main',
            name: 'newprovider',
            authorization: [{
                actor: actor,
                permission: permission
            }],
            data: {
                provider: actor,
                title: title,
                public_key: key
            }
        }
    ]
}

function bondActions({actor, permission}, {provider, endpoint, dots}) {
    return [
        {
            account: 'zap.main',
            name: 'bond',
            authorization: [{
                actor: actor,
                permission: permission
            }],
            data: {
                subscriber: actor,
                provider: provider,
                endpoint: endpoint,
                dots: dots
            }
        }
    ]
}

function addEndpointActions({actor, permission}, {endpoint, constants, parts, dividers}) {
    return [
        {
            account: 'zap.main',
            name: 'addendpoint',
            authorization: [{
                actor: actor,
                permission: permission
            }],
            data: {
                provider: actor,
                specifier: endpoint,
                constants: constants,
                parts: parts,
                dividers: dividers
            }
        }
    ]
}


describe('Main', function() {

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
            await expect(node.isRunning).to.be.equal(true);
        });

        it('#restart()', async () => {
            this.timeout(20000);
            await node.restart();
            await expect(node.isRunning).to.be.equal(true);
        });

        it('#kill()', async () => {
            this.timeout(20000);
            await node.run();
            node.kill();
            await expect(node.isRunning).to.be.equal(false);
        });

    });

    describe('Contracts', function () {
        let node = new Node(false, false);

        beforeEach(function (done) {
            this.timeout(30000);
            configureEnvironment(async () => {
                try {
                    await node.restart();
                    await node.init();
                } catch (e) {
                    console.log(e);
                }
                done();
            });
        });

        it('#transfer()', async () => {
            let eos = await node.connect();
            if (eos != null) {
                let res = await eos.transaction({
                    actions: transferActions(userAuth, {to: 'zap.main', amount: 7, memo: 'hi'})
                });
                let tokensAmount = await eos.getCurrencyBalance('zap.token', 'zap.main', 'TST');
                await expect(tokensAmount[0]).to.be.equal('7 TST');
            } else {
                throw 'failed'
            }
        });

        it('#newprovider()', async () => {
            let eos = await node.connect();
            if (eos != null) {
                let res = await eos.transaction({
                    actions: newProviderActions(providerAuth, {title: 'test', key: 1})
                });
                await expect(res.processed.receipt.status).to.be.equal('executed');
            } else {
                throw 'failed'
            }
        });

        it('#addendpoint()', async () => {
            let eos = await node.connect();
            if (eos != null) {
                await eos.transaction({
                    actions: newProviderActions(providerAuth, {title: 'test', key: 1})
                });
                let res = await eos.transaction({
                    actions: addEndpointActions(providerAuth, {
                        endpoint: 'test_endpoint',
                        constants: [200, 3, 0],
                        parts: [0, 1000000],
                        dividers: [1]
                    })
                });
                await expect(res.processed.receipt.status).to.be.equal('executed');
            } else {
                throw 'failed'
            }
        });

        it('#bond()', async () => {
            let eos = await node.connect();
            if (eos != null) {
                await eos.transaction({
                    actions: newProviderActions(providerAuth, {title: 'test', key: 1})
                });
                await eos.transaction({
                    actions: addEndpointActions(providerAuth, {
                        endpoint: 'test_endpoint',
                        constants: [200, 3, 0],
                        parts: [0, 1000000],
                        dividers: [1]
                    })
                });
                let res = await eos.transaction({
                    actions: bondActions(userAuth, {
                        provider: providerAuth.actor,
                        endpoint: 'test_endpoint',
                        dots: 1
                    })
                });
                await expect(res.processed.receipt.status).to.be.equal('executed');
            } else {
                throw 'failed'
            }
        });

     /*   it('#unbond()', async () => {
            await node.initAccounts();
        });*/


        after(function () {
            node.kill();
        })
    });
});
