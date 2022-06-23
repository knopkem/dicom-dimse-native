import { recompress, recompressOptions } from '../index';
import p from "path";

/*
Supported Transfer Syntaxes:
1.2.840.10008.1.2       Implicit VR Endian: Default Transfer Syntax for DICOM    
1.2.840.10008.1.2.1     Explicit VR Little Endian    
1.2.840.10008.1.2.2     Explicit VR Big Endian   
1.2.840.10008.1.2.4.50  JPEG Baseline (Process 1) - Lossy JPEG 8-bit Image Compression
1.2.840.10008.1.2.4.51  JPEG Baseline (Processes 2 & 4) - Lossy JPEG 12-bit Image Compression
1.2.840.10008.1.2.4.70  JPEG Lossless, Nonhierarchical, First- Order Prediction
1.2.840.10008.1.2.4.80  JPEG-LS Lossless Image Compression   <-- recommended
1.2.840.10008.1.2.4.81  JPEG-LS Lossy (Near- Lossless) Image Compression
1.2.840.10008.1.2.4.90  JPEG 2000 Image Compression (Lossless Only)	 
1.2.840.10008.1.2.4.91  JPEG 2000 Image Compression
1.2.840.10008.1.2.5     RLE Lossless
*/
const options: recompressOptions =
{
    sourcePath: p.join(__dirname, "dicom"), // can point to directory or single dicom file
    storagePath: p.join(__dirname, "output"), // existing directory only
    writeTransfer: "1.2.840.10008.1.2.4.51", // see supported ts 
    lossyQuality: 40, // only supported for JPEG Baseline (Processes 1, 2, 4) 0..100
    enableRecompression: true, // change compression of already compressed images 
    verbose: true
};

recompress(options, (result) => {
    console.log(JSON.parse(result));
});

