import { parseFile, parseOptions } from '../index';
import p from 'path';

const options: parseOptions = {
    sourcePath: p.join(__dirname, 'dicom', 'test.dcm'), // should point to single dicom file
    verbose: true,
};

parseFile(options, (result) => {
    console.log(JSON.parse(result));
});
