const fs = require('fs');

exports.binType = function (binDate) {
    var binType = 'err';

    // Read file and compare date in file to date passed in, if they are the same
    // extract the bin type for that date and return.
    for (const line of fs.readFileSync('./bindicator.csv').toString().split('\n')) {
        if (line.substring(0, 10) == binDate) {
            binType = line.substring(11);
            break;
        }
    };

    binType = binType.replace(/(\r\n|\n|\r)/gm, "");    // remove new line etc.
    return binType;
};