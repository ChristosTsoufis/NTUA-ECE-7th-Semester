const constructURL = require('../lib/constructURL');
const errorHandler = require('../lib/errorHandler');
const axios = require('axios');
const chalk = require('chalk');
const fs = require('fs');
const https = require('https');

module.exports = function(o) {
    
    isWrong = false;

    if (o.provider === undefined || o.datefrom === undefined || o.dateto === undefined)
        isWrong = true;

    if (!isWrong) {
        if (o.format === 'csv') format = 'csv';
        else format = 'json'
        
        param1 = o.provider;
        param2 = o.datefrom;
        param3 = o.dateto;

        var url = constructURL('/SessionsPerProvider/', param1, param2, param3, format);
        
        fs.readFile('../cli-client/softeng20bAPI.token', 'utf8', (error, data) => {
            if (error){
                console.log(chalk.red('Not authorized user!'))
            }
            else {
                var config = {
                    method: 'get',
                    url: url,
                    headers: { 'X-OBSERVATORY-AUTH': data },
                    httpsAgent: new https.Agent({ rejectUnauthorized: false })
                };
                axios(config)
                .then(res => {
                    if (format == 'csv') {
                        fs.writeFile('sessionsPerProvider.csv', res.data, function (err) {
                            if (err) return console.log(err);
                            console.log('Data saved in sessionsPerProvider.csv');
                        })
                    }
                    else {
                        fs.writeFile('sessionsPerProvider.json', JSON.stringify(res.data, null, 2), function (err) {
                            if (err) return console.log(err);
                            console.log('Data saved in sessionsPerProvider.json');
                        })
                    }
                })
                .catch(err => {
                    errorHandler(err);
                })
            }
        })
    }
    else{
        console.log(chalk.red('Error: mandatory parameters omitted\n'));
        console.log(chalk.yellow('Mandatory Parameters: \n --provider [provider id] \n --datefrom [YYYYMMDD] \n --dateto [YYYYMMDD]'));
        console.log(chalk.yellow('Optional Parameter: \n --format [json | csv] \n'));
        console.log(chalk.yellow('ex: ev_group03 sppr --provider [provider id] --datefrom [date] --dateto [date] (--format [json | csv])\n'));
    }
    
}