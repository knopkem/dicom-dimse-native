export interface Node {
    aet: string;
    ip: string;
    port: string;
}
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
}
export interface findScuOptions extends scuOptions {
    tags: KeyValue[];
}
export interface getScuOptions extends scuOptions {
    netTransferPrefer?: string;
    tags: KeyValue[];
}
export interface moveScuOptions extends scuOptions {
    tags: KeyValue[];
    destination: string;
    netTransferPrefer?: string;
}
export interface storeScuOptions extends scuOptions {
    sourcePath: string;
    netTransferPropose?: string;
}
export interface storeScpOptions extends scpOptions {
    storagePath?: string;
    netTransferPrefer?: string;
    netTransferPropose?: string;
    writeTransfer?: string;
    permissive?: boolean;
}
export interface shutdownScuOptions extends scuOptions {
}
export interface parseOptions {
    sourcePath: string;
    verbose?: boolean;
}
export interface recompressOptions {
    sourcePath: string;
    storagePath: string;
    writeTransfer?: string;
    verbose?: boolean;
}
export declare function echoScu(options: echoScuOptions, callback: (result: string) => void): void;
export declare function findScu(options: findScuOptions, callback: (result: string) => void): void;
export declare function getScu(options: getScuOptions, callback: (result: string) => void): void;
export declare function moveScu(options: moveScuOptions, callback: (result: string) => void): void;
export declare function storeScu(options: storeScuOptions, callback: (result: string) => void): void;
export declare function startStoreScp(options: storeScpOptions, callback: (result: string) => void): void;
export declare function shutdownScu(options: shutdownScuOptions, callback: (result: string) => void): void;
export declare function parseFile(options: parseOptions, callback: (result: string) => void): void;
export {};
