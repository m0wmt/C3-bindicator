const { readFileSync } = require('fs');

exports.binType = function (binDate) {
    var binType = 'err';

    const data = JSON.parse(readFileSync('./bindicator.json', 'utf8'));

    var keys = Object.keys(data);
    for( var i = 0,length = keys.length; i < length; i++ ) {
        if (data[ keys[ i ] ]["date"] === binDate) {
            binType = data[ keys[ i ] ]["bins"];
            break;
        }
    }

    return binType;
};