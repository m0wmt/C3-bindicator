const http = require('http');
const bt = require('./getbintype');
const url = require('url');

const server = http.createServer(function(req, res) {
    //res.writeHead(200, {'Content-Type': 'text/plain'});
    res.writeHead(200, {'Content-Type': 'application/json'});

    var obj = {collection : bt.binType(new Date().toISOString().slice(0, 10))};
    res.end(JSON.stringify(obj));
});

server.listen();

// Testing!!!

    // const baseURL = req.protocol + '://' + req.headers.host + '/';
    // const reqUrl = new URL(req.url, baseURL);

    // const searchParams = new URLSearchParams(reqUrl.search);    
    // const response = "err";

    // if (searchParams.has('binDate')) {     // binDate=yyyy-mm-dd available
    //     //const binDate = searchParams.get('binDate');
    //     response = bt.binType(searchParams.get('binDate').substring(0, 10));  // Restrict input to the length of a date yyyy-mm-dd
    //     //response = bt.binType(binDate);
    // } else {
        // const today = new Date().toISOString().slice(0, 10);
        //response = bt.binType(today); // Todays date, yyyy-mm-dd
//        response = new Date().toISOString().slice(0, 10);
    // }

//    res.end(searchParams.get('binDate'));
//    res.end(searchParams.toString());
    // if (searchParams.has('binDate')) {
    //     response = 'binDate present, value is: ' + searchParams.get('binDate');
    // } else {
    //     res.end(response);
    // }
    //res.end(':' + searchParams.get('binDate').substring(0, 10) + ':');
    //res.end('hkjhkjhlk ' + today);
