const addon = require('../index');

addon.recompress(JSON.stringify(
    {
        "sourcePath": "./examples/dicom.dcm",
        "storagePath": "./examples/out.dcm",
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
