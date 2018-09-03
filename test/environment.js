const util = require('util');
const exec = util.promisify(require('child_process').exec);

function startNode() {
}

function stopNode() {
}

module.exports = {
    startNode: startNode,
    stopNode: stopNode
}
