// res.writeHead(200, {'Content-Type': 'application/json'});

// //var obj = {collection : bt.binType(new Date().toISOString().slice(0, 10))};

// // Need to take in to account British summer/winter times
// var d = new Date();     // Get date
// var lon = d.toLocaleString('en-GB', {timeZone: 'Europe/London' });  // Get local date
// var dateOnly = lon.slice(0, 10);  // Only need year/month/day
// let mdy = dateOnly.split('/');  // Extract year/monty/day in to array

// // Date is dd/mm/yyyy, we want yyyy-mm-dd to match json file
// var formattedDate = mdy[2] + '-' + mdy[1] + '-' + mdy[0];

// var obj = {collection : bt.binType(formattedDate)};
// res.end(JSON.stringify(obj));


// For command line testing only

let obj = new Date().toISOString().slice(0, 10);
console.log(obj);

let d = new Date();
console.log("UTC time " + d);
let lon = d.toLocaleString('en-GB', {timeZone: 'Europe/London' });
let dateOnly = lon.slice(0, 10);
// Date is dd/mm/yy, we want yyyy-mm-dd to match json file
let mdy = dateOnly.split('/');
var formattedDate = mdy[2] + '-' + mdy[1] + '-' + mdy[0];
console.log("Formatted Date " + formattedDate);
console.log("Your time zone " + lon);
console.log("Your time zone date " + lon.slice(0, 10));