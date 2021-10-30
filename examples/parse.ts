import { parseFile, parseOptions } from '../index';

const options: parseOptions = {
    sourcePath: __dirname + './data',
    verbose: true,
};


parseFile(options, (result) => {
    console.log(JSON.parse(result));
});
