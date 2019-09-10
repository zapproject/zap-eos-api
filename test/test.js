const TestNode = require('./environment.js');
const Transaction = require('./eos/transaction.js');
const expect = require('chai')
    .use(require('chai-as-promised'))
    .expect;
const spawn = require('child_process').spawn;


async function configureEnvironment(func) {
    await func();
}

async function getRowsByPrimaryKey(eos, node, {scope, table_name, table_key}) {
    return await eos.getTableRows(
        true, // json
        node.getAccounts().main.name, // code
        scope, // scope
        table_name, // table name
        table_key, // table_key
        0, // lower_bound
        -1, // upper_bound
        10, // limit
        'i64', // key_type
        1 // index position
    );
}


describe('Main', function () {

    describe('EOS Node', function () {
        let node = new TestNode(false, false);

        beforeEach(function (done) {
            this.timeout(30000);
            node.kill();
            done();
        })

        it('#run()', async () => {
            await node.run();
            console.log(node.running);
            await expect(node.running).to.be.equal(true);
        }).timeout(20000);

        it('#restart()', async () => {
            await node.restart();
            await expect(node.running).to.be.equal(true);
        }).timeout(20000);

        it('#kill()', async () => {
            await node.run();
            node.kill();
            await expect(node.running).to.be.equal(false);
        }).timeout(20000);;
    });

    describe('Environment', function () {
        let node = new TestNode(true, false);

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
            this.timeout(50000);
            configureEnvironment(async () => {
                try {
                    node.kill();
                    await node.run();
                    console.log("restarted")
                } catch (e) {
                    console.log(e);
                }
                done();
            });
        })

        it('#transaction()', async () => {
            let accounts = node.getAccounts();
            let transferTransaction = new Transaction()
                .receiver(accounts.token)
                .action('transfer')
                .sender(accounts.user, 'active')
                .data({from: accounts.user.name, to: accounts.provider.name, quantity: '7.0000 EOS', memo: 'hi'})
                .build();

            let manualCreatedTransaction = {
                actions: [
                    {
                        account: accounts.token.name,
                        name: 'transfer',
                        authorization: [{
                            actor: accounts.user.name,
                            permission: accounts.user.default_auth
                        }],

                        data: {
                            from: accounts.user.name,
                            to: accounts.provider.name,
                            quantity: '7.0000 EOS',
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
        }).timeout(40000);

        // it('#issueTokens()', async () => {
        //     let eos = await node.connect();
        //     await node.registerAccounts(eos);
        //     await node.deploy(eos);
        //     let res = await node.issueTokens(eos);
        //     await expect(res.processed.receipt.status).to.be.equal('executed');
        // }).timeout(20000);

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
            await node.init();
        }).timeout(20000);


        after(function () {
            node.kill();
        })
    });

    describe('Contracts', function () {
        let node = new TestNode(false, false);

        /*
        [[eosio::action]] 
    void cinit(name provider, uint64_t finish, name oracle, std::vector<db::endp> endpoints);

    [[eosio::action]]
    void cjudge(uint64_t contest_id, name provider, name oracle, std::string winner, uint64_t win_value);

    [[eosio::action]]
    void csettle(name provider, uint64_t contest_id);

    [[eosio::action]]
    void cbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots);

    [[eosio::action]]
    void cunbond(name issuer, name provider, uint64_t contest_id, std::string specifier, uint64_t dots);
        */
        function createCinitTransaction(finish, oracle, endpoints) {
            return new Transaction()
                .sender(node.getAccounts().provider, 'active')
                .receiver(node.getAccounts().main)
                .action('cinit')
                .data({ provider: node.getAccounts().provider.name, finish: finish, oracle: oracle, endpoints: endpoints });
        }

        function createCsettleTransaction(contest_id) {
            return new Transaction()
                .sender(node.getAccounts().provider, 'active')
                .receiver(node.getAccounts().main)
                .action('csettle')
                .data({ provider: node.getAccounts().provider.name, contest_id: contest_id });
        }

        function createCjudgeTransaction(contest_id, oracle, winner, win_value) {
            return new Transaction()
                .sender(node.getAccounts().provider, 'active')
                .receiver(node.getAccounts().main)
                .action('cjudge')
                .data({ provider: node.getAccounts().provider.name, contest_id: contest_id, oracle: oracle, winner: winner, win_value: win_value });
        }

        function createCbondTransaction(contest_id, specifier, dots) {
            return new Transaction()
                .sender(node.getAccounts().user, 'active')
                .receiver(node.getAccounts().main)
                .action('cbond')
                .data({ issuer: node.getAccounts().user.name, provider: node.getAccounts().provider.name, contest_id: contest_id, specifier: specifier, dots: dots });
        }

        function createCunbondTransaction(contest_id, specifier, dots) {
            return new Transaction()
                .sender(node.getAccounts().user, 'active')
                .receiver(node.getAccounts().main)
                .action('cunbond')
                .data({ issuer: node.getAccounts().user.name, provider: node.getAccounts().provider.name, contest_id: contest_id, specifier: specifier, dots: dots });
        }

        function createNewProviderTransaction(title, key) {
            return new Transaction()
                .sender(node.getAccounts().provider, 'active')
                .receiver(node.getAccounts().main)
                .action('newprovider')
                .data({provider: node.getAccounts().provider.name, title: title, public_key: key});
        }

        function createAddEndpointTransaction(endpoint, broker) {
            return new Transaction()
                .sender(node.getAccounts().provider, 'active')
                .receiver(node.getAccounts().main)
                .action('addendpoint')
                .data({
                    provider: node.getAccounts().provider.name,
                    specifier: endpoint,
                    broker: broker,
                    functions: [3, 0, 0, 2, 10000]
                });
        }

        function createBondTransaction(endpoint, amount) {
            if (amount >= 0) {
                return new Transaction()
                    .sender(node.getAccounts().user, 'active')
                    .receiver(node.getAccounts().main)
                    .action('bond')
                    .data({
                        subscriber: node.getAccounts().user.name,
                        provider: node.getAccounts().provider.name,
                        endpoint: endpoint,
                        dots: amount
                    });
            } else {
                return new Transaction()
                    .sender(node.getAccounts().user, 'active')
                    .receiver(node.getAccounts().main)
                    .action('unbond')
                    .data({
                        subscriber: node.getAccounts().user.name,
                        provider: node.getAccounts().provider.name,
                        endpoint: endpoint,
                        dots: amount * -1
                    })
            }
        }

        function createQueryTransaction(endpoint, query, onchain_p, onchain_s) {
            return new Transaction()
                .sender(node.getAccounts().user, 'active')
                .receiver(node.getAccounts().main)
                .action('query')
                .data({
                    subscriber: node.getAccounts().user.name,
                    provider: node.getAccounts().provider.name,
                    endpoint: endpoint,
                    query: query,
                    onchain_provider: onchain_p ? 1 : 0,
                    onchain_subscriber: onchain_s ? 1 : 0,
                    timestamp: Date.now()
                });

        }

        function createSubscribeTransaction(endpoint, amount) {
            return new Transaction()
                .sender(node.getAccounts().user, 'active')
                .receiver(node.getAccounts().main)
                .action('subscribe')
                .data({
                    subscriber: node.getAccounts().user.name,
                    provider: node.getAccounts().provider.name,
                    endpoint: endpoint,
                    dots: amount,
                    params: 'some params'
                });
        }

        function createUnsubscribeSubTransaction(endpoint) {
            return new Transaction()
                .sender(node.getAccounts().user, 'active')
                .receiver(node.getAccounts().main)
                .action('unsubscribe')
                .data({
                    subscriber: node.getAccounts().user.name,
                    provider: node.getAccounts().provider.name,
                    endpoint: endpoint,
                    from_sub: 1
                });
        }

        function createUnsubscribeProTransaction(endpoint) {
            return new Transaction()
                .sender(node.getAccounts().provider, 'active')
                .receiver(node.getAccounts().main)
                .action('unsubscribe')
                .data({
                    subscriber: node.getAccounts().user.name,
                    provider: node.getAccounts().provider.name,
                    endpoint: endpoint,
                    from_sub: 0
                });
        }

        function createTestRpTransaction() {
            return new Transaction()
                .sender(node.getAccounts().user, 'active')
                .receiver(node.getAccounts().main)
                .action('testrp')
                .data({
                    issuer: node.getAccounts().user.name
                });
        }


        beforeEach(function (done) {
            this.timeout(40000);
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

            await new Transaction()
                .sender(node.getAccounts().user, 'active')
                .receiver(node.getAccounts().token)
                .action('transfer')
                .data({
                    from: node.getAccounts().user.name,
                    to: node.getAccounts().provider.name,
                    quantity: '7.0000 EOS',
                    memo: 'hi'
                })
                .execute(eos);

            let tokensAmount = await eos.getCurrencyBalance('eosio.token', 'provider', 'EOS');
            await expect(tokensAmount[0].toString()).to.be.equal('7.0000 EOS');
        });

        it('#newprovider()', async () => {
            let eos = await node.connect();

            await createNewProviderTransaction('test', 1).execute(eos);

            let res = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().main.name,
                table_name: 'provider',
                table_key: 'provider'
            });

            await expect(res.rows[0].title).to.be.equal('test');
        });

        it('#addendpoint()', async () => {
            let eos = await node.connect();

            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .execute(eos);

            let res = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'endpoint',
                table_key: 'id'
            });

            await expect(res.rows[0].specifier).to.be.equal('test_endpoint');
        });

        it('#bond()', async () => {
            let eos = await node.connect();

            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .merge(createBondTransaction('test_endpoint', 1))
                .execute(eos);

            let issued = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'issued',
                table_key: 'endpointid'
            });
            let holder = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().user.name,
                table_name: 'holder',
                table_key: 'provider'
            });

            await expect(issued.rows[0].dots).to.be.equal(1);
            await expect(holder.rows[0].dots).to.be.equal(1);

        });

        it('#bond() - wrong broker fail check', async () => {
            let eos = await node.connect();

            await expect(
                createNewProviderTransaction('test', 1)
                    .merge(createAddEndpointTransaction('test_endpoint', 'acc'))
                    .merge(createBondTransaction('test_endpoint', 1))
                    .execute(eos)
            ).to.be.eventually.rejected;
        });

        it('#unbond()', async () => {
            let eos = await node.connect();

            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .merge(createBondTransaction('test_endpoint', 1))
                .merge(createBondTransaction('test_endpoint', -1))
                .execute(eos);

            let issued = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'issued',
                table_key: 'endpointid'
            });
            let holder = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().user.name,
                table_name: 'holder',
                table_key: 'provider'
            });

            await expect(issued.rows[0].dots).to.be.equal(0);
            await expect(holder.rows[0].dots).to.be.equal(0);
        });

        it('#query()', async () => {
            let eos = await node.connect();
            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .merge(createBondTransaction('test_endpoint', 1))
                .merge(createQueryTransaction('test_endpoint', 'q', false, false))
                .execute(eos);

            let qdata = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().main.name,
                table_name: 'qdata',
                table_key: 'id'
            });
            let holder = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().user.name,
                table_name: 'holder',
                table_key: 'provider'
            });

            await expect(qdata.rows[0].data).to.be.equal('q');
            await expect(holder.rows[0].escrow).to.be.equal(1);
            await expect(holder.rows[0].dots).to.be.equal(0);
        });

        it('#query_action_listening', async () => {
            let eos = await node.connect();
            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .merge(createBondTransaction('test_endpoint', 1))
                .merge(createQueryTransaction('test_endpoint', 'q', false, false))
                .execute(eos);

            let listener = node.getEventListener();

            let foundQueryData = null;

            await listener.listen(
                (blockNumber, actions) => {
                    listener.stopListen();
                    foundQueryData = actions[0].payload.data;
                },
                'zap.main::query'
            );

            await expect(foundQueryData.query).to.be.equal('q');
        }).timeout(10000);

        it('#subscribe()', async () => {
            let eos = await node.connect();
            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .merge(createBondTransaction('test_endpoint', 1))
                .merge(createSubscribeTransaction('test_endpoint', 1))
                .execute(eos);


            let s = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'subscription',
                table_key: 'id'
            });

            await expect(s.rows.length).to.be.equal(1);
        });

        it('#unscubscribe() - from subscriber', async () => {
            let eos = await node.connect();
            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .merge(createBondTransaction('test_endpoint', 1))
                .merge(createSubscribeTransaction('test_endpoint', 1))
                //.merge(createUnsubscribeTransaction('test_endpoint', true))
                .execute(eos);

            await createUnsubscribeSubTransaction('test_endpoint').execute(eos);


            let s = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'subscription',
                table_key: 'id'
            });

            let p_holder = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'holder',
                table_key: 'id'
            });

            let s_holder = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().user.name,
                table_name: 'holder',
                table_key: 'id'
            });

            await expect(s.rows.length).to.be.equal(0);
            await expect(p_holder.rows[0].dots).to.be.equal(1);
            await expect(p_holder.rows[0].escrow).to.be.equal(0);
            await expect(s_holder.rows[0].dots).to.be.equal(0);
            await expect(s_holder.rows[0].escrow).to.be.equal(0);
        });

        it('#unscubscribe() - from provider', async () => {
            let eos = await node.connect();
            await createNewProviderTransaction('test', 1)
                .merge(createAddEndpointTransaction('test_endpoint', ''))
                .merge(createBondTransaction('test_endpoint', 1))
                .merge(createSubscribeTransaction('test_endpoint', 1))
                //.merge(createUnsubscribeTransaction('test_endpoint', false))
                .execute(eos);

            await createUnsubscribeProTransaction('test_endpoint').execute(eos);

            let s = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'subscription',
                table_key: 'id'
            });

            let p_holder = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().provider.name,
                table_name: 'holder',
                table_key: 'id'
            });

            let s_holder = await getRowsByPrimaryKey(eos, node, {
                scope: node.getAccounts().user.name,
                table_name: 'holder',
                table_key: 'id'
            });

            await expect(s.rows.length).to.be.equal(0);
            await expect(p_holder.rows[0].dots).to.be.equal(1);
            await expect(p_holder.rows[0].escrow).to.be.equal(0);
            await expect(s_holder.rows[0].dots).to.be.equal(0);
            await expect(s_holder.rows[0].escrow).to.be.equal(0);
        });

        //c_init, c_settle, c_judge, c_bond, c_unbond
        it('#cinit() - init contest', async () => {
            let eos = await node.connect();
            let date = new Date();
            let finish = new Date(date.setTime(date.getTime() + 1 * 86400000 ));
            let endpoints = new Array();
            endpoints.push({
                specifier: 'endp1',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPA'
            });
            endpoints.push({
                specifier: 'endp2',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPB'
            });

            await createNewProviderTransaction('test', 1)
                .merge(createCinitTransaction(finish.getDate(), node.getAccounts().provider.name, endpoints))
                .execute(eos);
        });

        it('#csettle() - settle contest', async () => {
            let eos = await node.connect();

            let date = new Date();
            let finish = new Date(date.setTime(date.getTime() + 1 * 86400000 ));
            let endpoints = new Array();
            endpoints.push({
                specifier: 'endp1',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPA'
            });
            endpoints.push({
                specifier: 'endp2',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPB'
            });

            let contest_id = 0;

            await createNewProviderTransaction('test', 1)
                .merge(createCinitTransaction(finish.getDate(), node.getAccounts().provider.name, endpoints))
                .merge(createCsettleTransaction(contest_id))
                .execute(eos);
        });

        it('#cjudge() - judge contest', async () => {
            let eos = await node.connect();

            let date = new Date();
            let finish = new Date(date.setTime(date.getTime() + 1 * 86400000 ));
            let endpoints = new Array();
            endpoints.push({
                specifier: 'endp1',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPA'
            });
            endpoints.push({
                specifier: 'endp2',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPB'
            });

            let contest_id = 0;

            await createNewProviderTransaction('test', 1)
                .merge(createCinitTransaction(finish.getDate(), node.getAccounts().provider.name, endpoints))
                .merge(createCsettleTransaction(contest_id))
                .merge(createCjudgeTransaction(contest_id, node.getAccounts().provider.name, 'endp1', 0))
                .execute(eos);
        });

        it('#cbond() - bond to contest', async () => {
            let eos = await node.connect();

            let date = new Date();
            let finish = new Date(date.setTime(date.getTime() + 1 * 86400000 ));
            let endpoints = new Array();
            endpoints.push({
                specifier: 'endp1',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPA'
            });
            endpoints.push({
                specifier: 'endp2',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPB'
            });

            let contest_id = 0;

            await createNewProviderTransaction('test', 1)
                .merge(createCinitTransaction(finish.getDate(), node.getAccounts().provider.name, endpoints))
                .merge(createCsettleTransaction(contest_id))
                .merge(createCbondTransaction(contest_id, 'endp1', 1))
                .execute(eos);
        });

        it('#cunbond() - unbond from contest', async () => {
            let eos = await node.connect();

            let date = new Date();
            let finish = new Date(date.setTime(date.getTime() + 1 * 86400000 ));
            let endpoints = new Array();
            endpoints.push({
                specifier: 'endp1',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPA'
            });
            endpoints.push({
                specifier: 'endp2',
                functions: [3, 0, 0, 2, 10000],
                maximum_supply: '1000 SPB'
            });

            let contest_id = 0;

            await createNewProviderTransaction('test', 1)
                .merge(createCinitTransaction(finish.getDate(), node.getAccounts().provider.name, endpoints))
                .merge(createCsettleTransaction(contest_id))
                .merge(createCbondTransaction(contest_id, 'endp1', 1))
                .merge(createCjudgeTransaction(contest_id, node.getAccounts().provider.name, 'endp1', 1))
                .merge(createCunbondTransaction(contest_id, 'endp1', 1))
                .execute(eos);
        });

        after(function () {
            node.kill();
        });
    });
});
