const addon = require('bindings')('dcmtk.node');

export interface Node {
  aet: string;
  ip: string;
  port: number;
};

export interface KeyValue {
  key: string;
  value: string;
}

interface scuOptions {
  source: Node;
  target: Node;
  verbose?: boolean;
}

interface scpOptions {
  source: Node;
  peers: Node[];
  verbose?: boolean;
}

export interface echoScuOptions extends scuOptions {
};

export interface findScuOptions extends scuOptions {
  netTransferPrefer?: string;
  tags: KeyValue[];
  charset?: string;
};

export interface getScuOptions extends scuOptions {
  netTransferPrefer?: string;
  tags: KeyValue[];
  storagePath?: string;
};

export interface moveScuOptions extends scuOptions {
  tags: KeyValue[];
  destination: string;
  netTransferPrefer?: string;
};

export interface storeScuOptions extends scuOptions {
  sourcePath: string;
  netTransferPropose?: string;
};

export interface storeScpOptions extends scpOptions {
  storagePath?: string;
  netTransferPrefer?: string;
  netTransferPropose?: string;
  writeTransfer?: string;
  permissive?: boolean;
  storeOnly?: boolean;
  writeFile?: boolean;
};

export interface shutdownScuOptions extends scuOptions {
};

export interface parseOptions {
  sourcePath: string;
  verbose?: boolean;
}

export interface recompressOptions {
  sourcePath: string;
  storagePath: string;
  writeTransfer?: string;
  lossyQuality?: number;
  verbose?: boolean;
};


export function echoScu(options: echoScuOptions, callback: (result: string) => void) {
  addon.echoScu(JSON.stringify(options), callback);
}

export function findScu(options: findScuOptions, callback: (result: string) => void) {
  addon.findScu(JSON.stringify(options), callback);
}

export function getScu(options: getScuOptions, callback: (result: string) => void) {
  addon.getScu(JSON.stringify(options), callback);
}

export function moveScu(options: moveScuOptions, callback: (result: string) => void) {
  addon.moveScu(JSON.stringify(options), callback);
}

export function storeScu(options: storeScuOptions, callback: (result: string) => void) {
  addon.storeScu(JSON.stringify(options), callback);
}

export function startStoreScp(options: storeScpOptions, callback: (result: string) => void) {
  addon.startScp(JSON.stringify(options), callback);
}

export function shutdownScu(options: shutdownScuOptions, callback: (result: string) => void) {
  addon.shutdownScu(JSON.stringify(options), callback);
}

export function parseFile(options: parseOptions, callback: (result: string) => void) {
  addon.parseFile(JSON.stringify(options), callback);
}

export function recompress(options: recompressOptions, callback: (result: string) => void) {
  addon.recompress(JSON.stringify(options), callback);
}
