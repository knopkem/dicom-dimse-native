// run.js — stress test for the Storage SCP (startStoreScp).
//
//   node run.js --lib <libDir> --samples <dir from gen_samples.js> --port 11200 \
//               [--exec 100] [--label name] [--strict]
//
// Phases
//   F2  30 SCUs in parallel, each storing its OWN 5 images (distinct patients) x3 -> all must be indexed
//   F2b 30 SCUs storing the SAME 5 images at the same time (stress; the process must survive)
//   F3  100 concurrent C-ECHO
//   F1  300 sequential C-ECHO (more than maxAssociations=128 in total)
//   F4  200 TCP connections closed without an association
//   F5  20 C-ECHO after the load
//   CS  ISO_IR 100 patient name must be stored as UTF-8
// With --exec N the SCP process also runs execSync/spawnSync every N ms.
// With --strict the process exits 1 if any acceptance criterion fails.
'use strict';
const fs = require('fs');
const os = require('os');
const path = require('path');
const net = require('net');
const { spawn, execFileSync } = require('child_process');

const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(`--${k}`); return i >= 0 ? args[i + 1] : d; };
const LIB = path.resolve(opt('lib', '.'));
const SAMPLES = path.resolve(opt('samples', 'samples'));
const PORT = opt('port', '11200');
const EXEC_MS = opt('exec', '0');
const LABEL = opt('label', `exec${EXEC_MS}`);
const STRICT = args.includes('--strict');
const SCU = opt('scu', 'lib');                    // 'lib' (this library) or 'dcmtk' (echoscu/storescu tools)
const WIN = process.platform === 'win32';
const RUN = path.join(os.tmpdir(), `dimse-stress-${LABEL}-${PORT}`);
const STATS = path.join(RUN, 'stats.json');
const CLIENT_DIR = path.join(RUN, 'clients');      // SCU processes write their dicom.log here
fs.rmSync(RUN, { recursive: true, force: true });
fs.mkdirSync(CLIENT_DIR, { recursive: true });

const lines = [];
const log = (s) => { const l = `[${LABEL}] ${s}`; console.log(l); lines.push(l); };
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

const srv = spawn(process.execPath, [path.join(__dirname, 'scp_server.js'), LIB, PORT, 'PACS_TESTE', path.join(RUN, 'storage'), STATS],
  { cwd: RUN, env: { ...process.env, EXEC_MS }, windowsHide: true,
    stdio: ['ignore', fs.openSync(path.join(RUN, 'server.out'), 'w'), fs.openSync(path.join(RUN, 'server.err'), 'w')] });
let exited = null;
srv.on('exit', (code, sig) => { exited = `code=${code} signal=${sig}`; });

const R = { alive: true, frozen: false };
const alive = () => { if (exited) R.alive = false; return exited ? `NO(${exited})` : 'yes'; };
function loop() {
  try {
    const age = Math.floor(Date.now() / 1000) - JSON.parse(fs.readFileSync(STATS, 'utf8')).heartbeat;
    if (age > 5) { R.frozen = true; return `FROZEN(${age}s)`; }
    return 'ok';
  } catch (_) { return '?'; }
}
function sh(cmd, a) { try { return execFileSync(cmd, a, { encoding: 'utf8', windowsHide: true, env: { ...process.env, PYTHONIOENCODING: 'utf-8', PYTHONUTF8: '1' } }).trim(); } catch (_) { return ''; } }
function procInfo() {
  if (exited) return {};
  if (WIN) {
    const o = sh('powershell', ['-NoProfile', '-Command', `$p=Get-Process -Id ${srv.pid}; "$($p.HandleCount) $($p.Threads.Count)"`]).split(' ');
    return { handles: Number(o[0]), threads: Number(o[1]) };
  }
  const fds = sh('lsof', ['-p', String(srv.pid)]).split('\n').length - 1;
  const zombies = sh('ps', ['-A', '-o', 'ppid=,stat=']).split('\n')
    .map((l) => l.trim().split(/\s+/)).filter(([pp, st]) => pp === String(srv.pid) && st && st.startsWith('Z')).length;
  const children = sh('ps', ['-A', '-o', 'ppid=']).split('\n').filter((l) => l.trim() === String(srv.pid)).length;
  return { fds, zombies, children };
}
function client(a) {
  return new Promise((resolve) => {
    const p = spawn(process.execPath, [path.join(__dirname, 'client.js'), LIB, ...a],
      { windowsHide: true, cwd: CLIENT_DIR, env: { ...process.env, STRESS_SCU: SCU } });
    let out = ''; p.stdout.on('data', (d) => (out += d));
    p.on('exit', () => {
      let r;
      try { r = JSON.parse(out.trim().split('\n').pop()); } catch (_) { r = { ok: 0, f: 1, err: `client crashed: ${out.slice(-200)}` }; }
      if (r.err && clientErrors.length < 3 && !clientErrors.includes(r.err)) clientErrors.push(r.err);
      resolve(r);
    });
  });
}
const clientErrors = [];
const sum = (rs) => rs.reduce((t, r) => ({ ok: t.ok + r.ok, f: t.f + r.f }), { ok: 0, f: 0 });
function sql(query) {
  const py = `import sqlite3,sys\nc=sqlite3.connect(sys.argv[1])\ntry:\n  print(c.execute(sys.argv[2]).fetchone()[0])\nexcept Exception as e:\n  print('ERR:'+str(e)[:80])`;
  return sh(WIN ? 'python' : 'python3', ['-c', py, path.join(RUN, 'storage', 'image.db'), query]);
}
const JOIN = 'from image i join series s on i.referenceId=s.id join study t on s.referenceId=t.id join patient p on t.referenceId=p.id';
function topLogErrors(n, dir = RUN) {
  const counts = {};
  for (const f of fs.readdirSync(dir).filter((f) => /^dicom\.log/.test(f))) {
    for (const l of fs.readFileSync(path.join(dir, f), 'utf8').split(/\r?\n/)) {
      if (!/^(ERROR|FATAL)/.test(l) || /state 1 event 7/.test(l)) continue;
      const k = l.replace(/\[[^\]]*\]/g, '').replace(/[0-9]+\.[0-9.]{10,}/g, 'UID').replace(/\/[^ :]+/g, 'PATH').trim().slice(0, 160);
      counts[k] = (counts[k] || 0) + 1;
    }
  }
  return Object.entries(counts).sort((a, b) => b[1] - a[1]).slice(0, n).map(([k, c]) => `${c}x ${k}`);
}
function logCount(re) {
  let n = 0;
  for (const f of fs.readdirSync(RUN).filter((f) => /^dicom\.log/.test(f))) n += (fs.readFileSync(path.join(RUN, f), 'utf8').match(re) || []).length;
  return n;
}

// --soak N: N C-ECHO (10 SCU processes, 4 concurrent each) while 50 SCUs store 5,000 distinct
// images (500 patients x 10 images, one association per patient), then check that every image
// is on disk and indexed, and that no fds/handles leaked. Replaces the normal phases.
async function soak(total) {
  const watch = setInterval(loop, 1000);                 // catches event-loop freezes during the load
  const t0 = Date.now();
  const [echo, store] = await Promise.all([
    Promise.all(Array.from({ length: 10 }, () => client(['echo', PORT, String(total / 10), '4']))).then(sum),
    Promise.all(Array.from({ length: 50 }, (_, j) => client(['storerange', PORT, path.join(SAMPLES, 'mass'), `${j * 10 + 1}-${j * 10 + 10}`]))).then(sum),
  ]);
  clearInterval(watch);
  const secs = Math.round((Date.now() - t0) / 1000);
  const indexed = Number(sql(`select count(*) ${JOIN} where p.PatientName like 'TESTE^MASSA%'`)) || 0;
  const countFiles = (d) => fs.readdirSync(d, { withFileTypes: true })
    .reduce((n, e) => n + (e.isDirectory() ? countFiles(path.join(d, e.name)) : (/^image\.db/.test(e.name) ? 0 : 1)), 0);
  const onDisk = countFiles(path.join(RUN, 'storage'));
  log(`SOAK ${total} C-ECHO + 500 C-STORE associations (5,000 images) in ${secs}s: echo ok=${echo.ok} fail=${echo.f} | store ok=${store.ok}/500 fail=${store.f} files=${onDisk}/5000 indexed=${indexed}/5000 | alive=${alive()} loop=${loop()}`);
  await sleep(8000);
  const after = await client(['echo', PORT, '20', '1']);
  const end = procInfo();
  log(`after 8s idle: 20 C-ECHO ok=${after.ok} | start ${JSON.stringify(R.start)} end ${JSON.stringify(end)}`);
  const errOut = fs.readFileSync(path.join(RUN, 'server.err'), 'utf8') + fs.readFileSync(path.join(RUN, 'server.out'), 'utf8');
  const tooMany = logCount(/TooManyAssociations/g);
  const crash = (errOut.match(/terminate called|Segmentation fault|Abort trap|double free|pointer being freed|FATAL ERROR/gi) || []).length;
  let stats = {}; try { stats = JSON.parse(fs.readFileSync(STATS, 'utf8')); } catch (_) {}
  srv.kill();
  await sleep(1000);
  const grow = WIN ? (end.handles - R.start.handles) : (end.fds - R.start.fds);
  const fails = [];
  if (!R.alive) fails.push('process died');
  if (R.frozen) fails.push('event loop frozen');
  if (echo.ok !== total) fails.push(`echo ${echo.ok}/${total}`);
  if (store.ok !== 500 || onDisk !== 5000 || indexed !== 5000) fails.push(`store ${store.ok}/500 files ${onDisk}/5000 indexed ${indexed}/5000`);
  if (after.ok !== 20) fails.push(`after-load echo ${after.ok}/20`);
  if (tooMany) fails.push(`TooManyAssociations=${tooMany}`);
  if (crash) fails.push('crash message');
  if (end.zombies) fails.push(`zombies=${end.zombies}`);
  if (grow > (WIN ? 50 : 15)) fails.push(`${WIN ? 'handles' : 'fds'} grew by ${grow}`);
  if (Number(EXEC_MS) > 0 && (stats.exec_fail || stats.exec_wrong_status)) fails.push('child_process errors');
  const pass = fails.length === 0;
  const summary = `${LABEL} on ${process.platform}/${process.arch}: ${pass ? 'PASS' : 'FAIL'} | ${total} echo ok ${echo.ok} | ` +
    `500 store assoc ok ${store.ok} | 5000 images: files ${onDisk} indexed ${indexed} | ${secs}s | ${WIN ? 'handles' : 'fds'} ${WIN ? R.start.handles : R.start.fds}->${WIN ? end.handles : end.fds} | ` +
    `rss ${stats.rss_mb_start}->${stats.rss_mb} MB | exec_ok ${stats.exec_ok || 0} | alive ${R.alive} | frozen ${R.frozen} | TooMany ${tooMany} | crash ${crash}` +
    (fails.length ? ` | problems: ${fails.join('; ')}` : '') +
    (fails.length && clientErrors.length ? ` | client errors: ${clientErrors.join(' || ')}` : '') +
    (fails.length ? ` | server log: ${topLogErrors(5).join(' || ') || '(no ERROR lines)'}` : '');
  console.log(`::${pass ? 'notice' : (STRICT ? 'error' : 'warning')} title=${LABEL} ${process.platform}-${process.arch}::${summary}`);
  if (process.env.GITHUB_STEP_SUMMARY) fs.appendFileSync(process.env.GITHUB_STEP_SUMMARY, `### ${LABEL} (${process.platform}/${process.arch})\n\n\`\`\`\n${lines.join('\n')}\n\`\`\`\n\n**${pass ? 'PASS' : 'FAIL'}** ${fails.join('; ')}\n\n`);
  if (process.env.STRESS_ARTIFACT_DIR) fs.cpSync(RUN, path.join(process.env.STRESS_ARTIFACT_DIR, path.basename(RUN)), { recursive: true });
  process.exit(STRICT && !pass ? 1 : 0);
}

(async () => {
  await sleep(4000);
  R.start = procInfo();
  log(`start pid=${srv.pid} alive=${alive()} loop=${loop()} ${JSON.stringify(R.start)}`);
  const SOAK = Number(opt('soak', '0'));
  if (SOAK > 0) return soak(SOAK);

  let s = { ok: 0, f: 0 };
  for (let i = 0; i < 3; i++) {
    const t = sum(await Promise.all(Array.from({ length: 30 }, (_, j) => client(['store', PORT, path.join(SAMPLES, 'unique', `c${j + 1}`), '1']))));
    s.ok += t.ok; s.f += t.f;
  }
  R.f2 = { ...s, indexed: Number(sql(`select count(*) ${JOIN} where p.PatientName like 'TESTE^CLIENTE%'`)) || 0 };
  log(`F2  90 realistic C-STORE (30 parallel x3): ok=${s.ok} fail=${s.f} indexed=${R.f2.indexed}/150 alive=${alive()} loop=${loop()}`);

  R.f2b = sum(await Promise.all(Array.from({ length: 30 }, () => client(['store', PORT, path.join(SAMPLES, 'dup'), '1']))));
  log(`F2b 30 duplicate C-STORE (stress): ok=${R.f2b.ok} fail=${R.f2b.f} alive=${alive()} loop=${loop()}`);

  R.f3 = sum(await Promise.all(Array.from({ length: 10 }, () => client(['echo', PORT, '10', '10']))));
  log(`F3  100 concurrent C-ECHO: ok=${R.f3.ok} fail=${R.f3.f} alive=${alive()} loop=${loop()}`);

  R.f1 = await client(['echo', PORT, '300', '1']);
  log(`F1  300 sequential C-ECHO: ok=${R.f1.ok} fail=${R.f1.f} alive=${alive()} loop=${loop()}`);

  for (let i = 0; i < 200; i++) await new Promise((ok) => { const c = net.connect(Number(PORT), '127.0.0.1', () => { c.destroy(); ok(); }); c.on('error', ok); });
  log(`F4  200 aborted TCP connections: alive=${alive()} loop=${loop()}`);

  const cs = await client(['store', PORT, path.join(SAMPLES, 'charset'), '1']);
  const name = sql("select PatientName from patient where PatientID='ACENTO01'");
  R.charset = cs.ok === 1 && name === 'JOSÉ^CONCEIÇÃO';
  log(`CS  ISO_IR 100 name stored as UTF-8: ${R.charset ? 'ok' : `FAIL (stored=${JSON.stringify(name)})`}`);

  await sleep(6000);
  R.f5 = await client(['echo', PORT, '20', '1']);
  R.end = procInfo();
  log(`F5  20 C-ECHO after load: ok=${R.f5.ok} fail=${R.f5.f} alive=${alive()} loop=${loop()} ${JSON.stringify(R.end)}`);

  const errOut = fs.readFileSync(path.join(RUN, 'server.err'), 'utf8') + fs.readFileSync(path.join(RUN, 'server.out'), 'utf8');
  R.log = {
    tooMany: logCount(/TooManyAssociations/g), dbNotInitialized: logCount(/database not initialized/g),
    notIndexed: logCount(/image not indexed/g), threadException: logCount(/Association thread failed/g),
    crash: (errOut.match(/terminate called|Segmentation fault|Abort trap|double free|pointer being freed|FATAL ERROR/gi) || []).length,
  };
  try { R.stats = JSON.parse(fs.readFileSync(STATS, 'utf8')); } catch (_) { R.stats = {}; }
  log(`LOG ${JSON.stringify(R.log)} exec_ok=${R.stats.exec_ok} exec_fail=${R.stats.exec_fail} exec_wrong_status=${R.stats.exec_wrong_status} rss_mb=${R.stats.rss_mb_start}->${R.stats.rss_mb}`);
  srv.kill();
  await sleep(1000);

  const fails = [];
  if (!R.alive) fails.push('process died');
  if (R.frozen) fails.push('event loop frozen');
  if (R.f2.ok !== 90 || R.f2.indexed !== 150) fails.push(`F2 ok=${R.f2.ok}/90 indexed=${R.f2.indexed}/150`);
  if (R.f3.ok !== 100) fails.push(`F3 ${R.f3.ok}/100`);
  if (R.f1.ok !== 300) fails.push(`F1 ${R.f1.ok}/300`);
  if (R.f5.ok !== 20) fails.push(`F5 ${R.f5.ok}/20`);
  if (!R.charset) fails.push('charset');
  if (R.log.tooMany) fails.push(`TooManyAssociations=${R.log.tooMany}`);
  if (R.log.crash) fails.push('crash message');
  if (Number(EXEC_MS) > 0 && (R.stats.exec_fail || R.stats.exec_wrong_status)) fails.push('child_process errors');
  if (R.end.zombies) fails.push(`zombies=${R.end.zombies}`);
  R.pass = fails.length === 0;

  const summary = `${LABEL} on ${process.platform}/${process.arch}: ${R.pass ? 'PASS' : 'FAIL'} | ` +
    `F2 ${R.f2.ok}/90 indexed ${R.f2.indexed}/150 | F2b ${R.f2b.ok}/30 | F3 ${R.f3.ok}/100 | F1 ${R.f1.ok}/300 | F5 ${R.f5.ok}/20 | ` +
    `charset ${R.charset ? 'ok' : 'FAIL'} | alive ${R.alive} | frozen ${R.frozen} | TooMany ${R.log.tooMany} | crash ${R.log.crash} | ` +
    `start ${JSON.stringify(R.start)} end ${JSON.stringify(R.end)}` + (fails.length ? ` | problems: ${fails.join('; ')}` : '') +
    (clientErrors.length ? ` | client errors: ${clientErrors.join(' || ')}` : '') +
    (fails.length ? ` | server log: ${topLogErrors(5).join(' || ') || '(no ERROR lines)'}` : '') +
    (fails.length ? ` | client log: ${topLogErrors(5, CLIENT_DIR).join(' || ') || '(no ERROR lines)'}` : '');
  console.log(`::${R.pass ? 'notice' : (STRICT ? 'error' : 'warning')} title=${LABEL} ${process.platform}-${process.arch}::${summary}`);
  if (process.env.GITHUB_STEP_SUMMARY) fs.appendFileSync(process.env.GITHUB_STEP_SUMMARY, `### ${LABEL} (${process.platform}/${process.arch})\n\n\`\`\`\n${lines.join('\n')}\n\`\`\`\n\n**${R.pass ? 'PASS' : 'FAIL'}** ${fails.join('; ')}\n\n`);
  fs.writeFileSync(path.join(RUN, 'result.json'), JSON.stringify(R, null, 1));
  if (process.env.STRESS_ARTIFACT_DIR) fs.cpSync(RUN, path.join(process.env.STRESS_ARTIFACT_DIR, path.basename(RUN)), { recursive: true });
  process.exit(STRICT && !R.pass ? 1 : 0);
})();
