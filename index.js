"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.parseFile = exports.shutdownScu = exports.startScp = exports.storeScu = exports.moveScu = exports.getScu = exports.findScu = exports.echoScu = void 0;
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
function startScp(options, callback) {
    addon.startScp(JSON.stringify(options), callback);
}
exports.startScp = startScp;
function shutdownScu(options, callback) {
    addon.shutdownScu(JSON.stringify(options), callback);
}
exports.shutdownScu = shutdownScu;
function parseFile(options, callback) {
    addon.parseFile(JSON.stringify(options), callback);
}
exports.parseFile = parseFile;
