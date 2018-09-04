const Node = require('./environment.js');
const expect = require('chai')
    .use(require('chai-as-promised'))
    .expect;

async function configureEnvironment(func) {
    await func();
}

console.log("Starting...");


describe('Main', function() {

    describe('EOS Node', function () {
        let node = new Node();

        beforeEach(function (done) {
            this.timeout(10000);
            node.kill();
            done();

        });

        it('#run()', async () => {
            await node.run();
            await expect(node.isRunning).to.be.equal(true);
        });

        it('#restart()', async () => {
            await node.restart();
            await expect(node.isRunning).to.be.equal(true);
        });

        it('#kill()', async () => {
            await node.run();
            node.kill();
            await expect(node.isRunning).to.be.equal(false);
        });

    });

    describe('Contracts', function () {
        let node = new Node();

        beforeEach(function (done) {
            this.timeout(10000);
            configureEnvironment(async () => {
                try {
                    await node.restart();
                } catch (e) {
                    console.log(e);
                }
                done();
            });
        });


        it('#bond()', async () => {
            await node.initAccounts();
        });

        it('#unbond()', async () => {
            await node.initAccounts();
        });


        after(function () {
            node.kill();
        })
    });
});
