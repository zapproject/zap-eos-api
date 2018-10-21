const DemuxEos = require("demux-eos");

class EventReader {
    constructor(node, startBlock) {
        this._currentBlock = startBlock;
        this._node = node;
        this._listen = true;
        this._actionReader = new DemuxEos.NodeosActionReader(this._node.eos_test_config.httpEndpoint, this._currentBlock);
    }

    async readUntilEnd(callback, actionName) {
        let latest = await this._actionReader.getHeadBlockNumber();

        while (this._currentBlock <= latest) {
            await this.readBlock(actionName, callback);
            this._currentBlock++;
        }

        return true;
    }

    async listen(callback, actionName) {
        while (this._listen) {
            await this.readBlock(actionName, callback);
            this._currentBlock++;
        }
    }

    stopListen() {
        this._listen = false;
    }

    async readBlock(actionName, callback) {
        let block = await this._actionReader.getBlock(this._currentBlock);

        if (block) {
            let listenedActions = EventReader.findActions(block.actions, actionName);
            if (callback && listenedActions.length > 0) {
                callback(block.blockInfo.blockNumber, listenedActions);
            }
        }

        return true;
    }

    static findActions(actions, name) {
        let found = [];

        if (!name || name === '') {
            found = actions;
        } else {
            for (let i in actions) {
                if (actions.hasOwnProperty(i)) {
                    if (actions[i].type === name) {
                        found.push(actions[i]);
                    }
                }
            }
        }

        return found;
    }
}

module.exports = EventReader;