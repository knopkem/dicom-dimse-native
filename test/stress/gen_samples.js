// gen_samples.js — generates synthetic DICOM Part-10 files (Explicit VR Little Endian,
// Secondary Capture, 64x64 8-bit) for the stress test. No patient data, no external tools.
//
//   node gen_samples.js <outDir>
//
// Creates:
//   <outDir>/unique/c1..c30/img1..5.dcm  30 "modalities", each with its own patient/study/series
//   <outDir>/dup/img1..5.dcm             one set that all clients send at the same time (stress)
//   <outDir>/charset/acento.dcm          ISO_IR 100 patient name "JOSÉ^CONCEIÇÃO"
//   <outDir>/mass/c1..c500/img1..10.dcm  5,000 distinct instances for the soak test
'use strict';
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const SC_CLASS = '1.2.840.10008.5.1.4.1.1.7';
const EXPLICIT_LE = '1.2.840.10008.1.2.1';
const ROOT = '1.2.826.0.1.3680043.8.498';           // UID root reserved for examples/tests
const LONG_VR = new Set(['OB', 'OW', 'OF', 'SQ', 'UT', 'UN']);

function pad(buf, vr) {
  if (buf.length % 2 === 0) return buf;
  const fill = (vr === 'UI' || vr === 'OB') ? 0x00 : 0x20;
  return Buffer.concat([buf, Buffer.from([fill])]);
}

function element(group, elem, vr, value) {
  let v;
  if (Buffer.isBuffer(value)) v = value;
  else if (vr === 'US') { v = Buffer.alloc(2); v.writeUInt16LE(value); }
  else if (vr === 'UL') { v = Buffer.alloc(4); v.writeUInt32LE(value); }
  else v = Buffer.from(String(value), 'latin1');
  v = pad(v, vr);
  const head = Buffer.alloc(LONG_VR.has(vr) ? 12 : 8);
  head.writeUInt16LE(group, 0); head.writeUInt16LE(elem, 2); head.write(vr, 4, 'latin1');
  if (LONG_VR.has(vr)) head.writeUInt32LE(v.length, 8); else head.writeUInt16LE(v.length, 6);
  return Buffer.concat([head, v]);
}

function dicomFile({ sopUid, studyUid, seriesUid, patientName, patientId, instance, charset }) {
  const meta = Buffer.concat([
    element(0x0002, 0x0001, 'OB', Buffer.from([0x00, 0x01])),
    element(0x0002, 0x0002, 'UI', SC_CLASS),
    element(0x0002, 0x0003, 'UI', sopUid),
    element(0x0002, 0x0010, 'UI', EXPLICIT_LE),
    element(0x0002, 0x0012, 'UI', `${ROOT}.0.1`),
  ]);
  const ds = [];
  if (charset) ds.push(element(0x0008, 0x0005, 'CS', charset));
  ds.push(
    element(0x0008, 0x0016, 'UI', SC_CLASS),
    element(0x0008, 0x0018, 'UI', sopUid),
    element(0x0008, 0x0020, 'DA', '20261108'),
    element(0x0008, 0x0060, 'CS', 'OT'),
    element(0x0010, 0x0010, 'PN', Buffer.isBuffer(patientName) ? patientName : Buffer.from(patientName, 'latin1')),
    element(0x0010, 0x0020, 'LO', patientId),
    element(0x0020, 0x000D, 'UI', studyUid),
    element(0x0020, 0x000E, 'UI', seriesUid),
    element(0x0020, 0x0013, 'IS', String(instance)),
    element(0x0028, 0x0002, 'US', 1),
    element(0x0028, 0x0004, 'CS', 'MONOCHROME2'),
    element(0x0028, 0x0010, 'US', 64),
    element(0x0028, 0x0011, 'US', 64),
    element(0x0028, 0x0100, 'US', 8),
    element(0x0028, 0x0101, 'US', 8),
    element(0x0028, 0x0102, 'US', 7),
    element(0x0028, 0x0103, 'US', 0),
    element(0x7FE0, 0x0010, 'OB', crypto.randomBytes(64 * 64)),
  );
  return Buffer.concat([
    Buffer.alloc(128), Buffer.from('DICM', 'latin1'),
    element(0x0002, 0x0000, 'UL', meta.length), meta,
    ...ds,
  ]);
}

const out = path.resolve(process.argv[2] || 'samples');
fs.rmSync(out, { recursive: true, force: true });
const base = `${ROOT}.${Date.now()}`;
let n = 0;
function series(dir, tag, name, id, charset) {
  fs.mkdirSync(dir, { recursive: true });
  for (let i = 1; i <= 5; i++) {
    fs.writeFileSync(path.join(dir, `img${i}.dcm`), dicomFile({
      sopUid: `${base}.${tag}.1.${i}`, studyUid: `${base}.${tag}.2`, seriesUid: `${base}.${tag}.3`,
      patientName: name, patientId: id, instance: i, charset,
    }));
    n++;
  }
}
for (let c = 1; c <= 30; c++) series(path.join(out, 'unique', `c${c}`), c, `TESTE^CLIENTE${c}`, `TESTE${c}`);
// mass/c1..c500: 500 patients/studies x 10 images = 5,000 distinct instances (soak test)
for (let c = 1; c <= 500; c++) {
  const dir = path.join(out, 'mass', `c${c}`);
  fs.mkdirSync(dir, { recursive: true });
  for (let i = 1; i <= 10; i++) {
    fs.writeFileSync(path.join(dir, `img${i}.dcm`), dicomFile({
      sopUid: `${base}.${1000 + c}.1.${i}`, studyUid: `${base}.${1000 + c}.2`, seriesUid: `${base}.${1000 + c}.3`,
      patientName: `TESTE^MASSA${c}`, patientId: `MASSA${c}`, instance: i,
    }));
    n++;
  }
}
series(path.join(out, 'dup'), 99, 'TESTE^CARGA', 'TESTE001');
fs.mkdirSync(path.join(out, 'charset'), { recursive: true });
fs.writeFileSync(path.join(out, 'charset', 'acento.dcm'), dicomFile({
  sopUid: `${base}.100.1.1`, studyUid: `${base}.100.2`, seriesUid: `${base}.100.3`,
  patientName: Buffer.from('JOSÉ^CONCEIÇÃO', 'latin1'), patientId: 'ACENTO01', instance: 1, charset: 'ISO_IR 100',
}));
console.log(`generated ${n + 1} files in ${out}`);
