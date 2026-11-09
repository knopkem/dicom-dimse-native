// scp_server.js — starts the Storage SCP with the options used in production and,
// optionally, exercises child_process every EXEC_MS ms (execSync/spawnSync, like
// apps that call ss/df/ping) to detect fork()/pipe interactions.
//
//   EXEC_MS=100 node scp_server.js <libDir> <port> <aet> <storageDir> <stats.json>
'use strict';
const fs = require('fs');
const path = require('path');
const { execSync, spawnSync } = require('child_process');

const [libDir, port, aet, storage, statsFile] = process.argv.slice(2);
const EXEC_MS = Number(process.env.EXEC_MS || 0);
const WIN = process.platform === 'win32';
fs.mkdirSync(storage, { recursive: true });
const { startStoreScp } = require(path.resolve(libDir, 'index.js'));

const stats = {
  pid: process.pid, exec_ms: EXEC_MS, exec_ok: 0, exec_fail: 0, exec_wrong_status: 0,
  last_exec_error: null, rss_mb_start: 0, rss_mb: 0, rss_mb_max: 0, heartbeat: 0,
};

startStoreScp({
  source: { aet, ip: '127.0.0.1', port: Number(port) },
  peers: [],
  storagePath: storage,
  permissive: true,
  verbose: true,
}, () => {});

if (EXEC_MS > 0) {
  setInterval(() => {
    try {
      execSync(WIN ? 'cmd /c exit 0' : 'true', { stdio: 'ignore', windowsHide: true });
      const r = WIN ? spawnSync('cmd', ['/c', 'exit 7'], { windowsHide: true }) : spawnSync('sh', ['-c', 'exit 7']);
      if (r.status !== 7) { stats.exec_wrong_status++; stats.last_exec_error = `status=${r.status} err=${r.error}`; }
      else stats.exec_ok++;
    } catch (e) {
      stats.exec_fail++; stats.last_exec_error = String(e.message).slice(0, 200);
    }
  }, EXEC_MS);
}

setInterval(() => {
  stats.rss_mb = Math.round(process.memoryUsage().rss / 1048576);
  if (!stats.rss_mb_start) stats.rss_mb_start = stats.rss_mb;
  stats.rss_mb_max = Math.max(stats.rss_mb_max, stats.rss_mb);
  stats.heartbeat = Math.floor(Date.now() / 1000);
  try { fs.writeFileSync(statsFile, JSON.stringify(stats)); } catch (_) {}
}, 1000);
