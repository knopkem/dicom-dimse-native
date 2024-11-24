"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.echoScu = echoScu;
exports.findScu = findScu;
exports.getScu = getScu;
exports.moveScu = moveScu;
exports.storeScu = storeScu;
exports.startStoreScp = startStoreScp;
exports.shutdownScu = shutdownScu;
exports.parseFile = parseFile;
exports.recompress = recompress;
var addon = require('bindings')('dcmtk.node');
;
;
;
;
;
;
;
;
;
function echoScu(options, callback) {
    addon.echoScu(JSON.stringify(options), callback);
}
function findScu(options, callback) {
    addon.findScu(JSON.stringify(options), callback);
}
function getScu(options, callback) {
    addon.getScu(JSON.stringify(options), callback);
}
function moveScu(options, callback) {
    addon.moveScu(JSON.stringify(options), callback);
}
function storeScu(options, callback) {
    addon.storeScu(JSON.stringify(options), callback);
}
function startStoreScp(options, callback) {
    addon.startScp(JSON.stringify(options), callback);
}
function shutdownScu(options, callback) {
    addon.shutdownScu(JSON.stringify(options), callback);
}
function parseFile(options, callback) {
    addon.parseFile(JSON.stringify(options), callback);
}
function recompress(options, callback) {
    addon.recompress(JSON.stringify(options), callback);
}
