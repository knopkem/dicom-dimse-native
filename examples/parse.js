const addon = require('../index');

addon.parseFile(JSON.stringify(
    {
        "sourcePath": "./examples/dicom.dcm", // single DICOM file only
        "verbose": true
    }
), (result) => {
        if (result && result.length > 0) {
            try
            {
                console.log(JSON.parse(result));
            }
            catch (e) {
                console.log(e, result);
            }
        }
});
