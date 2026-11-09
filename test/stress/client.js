// client.js — DICOM SCU using this library's echoScu/storeScu. Prints one JSON line {ok, f}.
//   node client.js <libDir> echo  <port> <count> <concurrency>
//   node client.js <libDir> store <port> <dicomDir> <repeats>
//   node client.js <libDir> storerange <port> <parentDir> <from>-<to>
'use strict';
const path = require('path');
const [libDir, mode, port, a, b] = process.argv.slice(2);
let { echoScu, storeScu } = require(path.resolve(libDir, 'index.js'));
const source = { aet: 'STRESS_SCU', ip: '127.0.0.1', port: 9999 }; // not listened on; 0 is rejected
const target = { aet: 'PACS_TESTE', ip: '127.0.0.1', port: Number(port) };

var firstError = null;
function once(fn, opts) {
  return new Promise((resolve) => {
    const timer = setTimeout(() => { firstError = firstError || 'timeout (60 s)'; resolve(false); }, 60000);
    fn(opts, (res) => {
      let r; try { r = JSON.parse(res); } catch (_) { return; }
      if (r.code === 1) return;              // progress
      clearTimeout(timer);
      if (r.code !== 0 && !firstError) firstError = String(r.message || JSON.stringify(r)).slice(0, 300);
      resolve(r.code === 0);
    });
  });
}

// STRESS_SCU=dcmtk: use the DCMTK command line tools (echoscu/storescu) as SCU instead of this library
if (process.env.STRESS_SCU === 'dcmtk') {
  const { spawn } = require('child_process');
  const fs = require('fs');
  const run = (cmd, a) => new Promise((resolve) => {
    const p = spawn(cmd, a); let err = '';
    p.stderr.on('data', (d) => (err += d)); p.stdout.on('data', (d) => (err += d));
    const t = setTimeout(() => p.kill(), 60000);
    p.on('error', (e) => { clearTimeout(t); firstError = firstError || String(e.message); resolve(false); });
    p.on('exit', (code) => { clearTimeout(t); if (code !== 0 && !firstError) firstError = `${cmd} exit ${code}: ${err.trim().split('\n').pop()}`; resolve(code === 0); });
  });
  const base = ['-aet', source.aet, '-aec', target.aet, target.ip, String(target.port)];
  echoScu = (o, cb) => run('echoscu', base).then((ok) => cb(JSON.stringify({ code: ok ? 0 : 2 })));
  storeScu = (o, cb) => run('storescu', [...base, ...fs.readdirSync(o.sourcePath).map((f) => path.join(o.sourcePath, f))])
    .then((ok) => cb(JSON.stringify({ code: ok ? 0 : 2 })));
}

(async () => {
  let ok = 0, f = 0;
  if (mode === 'echo') {
    const total = Number(a), conc = Number(b);
    let started = 0;
    const worker = async () => { while (started < total) { started++; (await once(echoScu, { source, target })) ? ok++ : f++; } };
    await Promise.all(Array.from({ length: conc }, worker));
  } else if (mode === 'storerange') {
    // storerange <port> <parentDir> <from>-<to>: one association per folder c<from>..c<to>
    const [from, to] = b.split('-').map(Number);
    for (let c = from; c <= to; c++) (await once(storeScu, { source, target, sourcePath: path.resolve(a, `c${c}`) })) ? ok++ : f++;
  } else {
    for (let i = 0; i < Number(b); i++) (await once(storeScu, { source, target, sourcePath: path.resolve(a) })) ? ok++ : f++;
  }
  console.log(JSON.stringify({ ok, f, err: firstError }));
  process.exit(0);
})();
