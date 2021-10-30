"use strict";
exports.__esModule = true;
exports.recompress = exports.parseFile = exports.shutdownScu = exports.startStoreScp = exports.storeScu = exports.moveScu = exports.getScu = exports.findScu = exports.echoScu = void 0;
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
exports.echoScu = echoScu;
function findScu(options, callback) {
    addon.findScu(JSON.stringify(options), callback);
}
exports.findScu = findScu;
function getScu(options, callback) {
    addon.getScu(JSON.stringify(options), callback);
}
exports.getScu = getScu;
function moveScu(options, callback) {
    addon.moveScu(JSON.stringify(options), callback);
}
exports.moveScu = moveScu;
function storeScu(options, callback) {
    addon.storeScu(JSON.stringify(options), callback);
}
exports.storeScu = storeScu;
function startStoreScp(options, callback) {
    addon.startScp(JSON.stringify(options), callback);
}
exports.startStoreScp = startStoreScp;
function shutdownScu(options, callback) {
    addon.shutdownScu(JSON.stringify(options), callback);
}
exports.shutdownScu = shutdownScu;
function parseFile(options, callback) {
    addon.parseFile(JSON.stringify(options), callback);
}
exports.parseFile = parseFile;
function recompress(options, callback) {
    addon.recompress(JSON.stringify(options), callback);
}
exports.recompress = recompress;
