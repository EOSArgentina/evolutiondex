"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

var _slicedToArray2 = _interopRequireDefault(require("@babel/runtime/helpers/slicedToArray"));

var _typeof2 = _interopRequireDefault(require("@babel/runtime/helpers/typeof"));

var assert = require('assert');

var ecurve = require('ecurve');

var BigInteger = require('bigi');

var secp256k1 = ecurve.getCurveByName('secp256k1');

var hash = require('./hash');

var keyUtils = require('./key_utils');

var G = secp256k1.G;
var n = secp256k1.n;
module.exports = PublicKey;
/**
  @param {string|Buffer|PublicKey|ecurve.Point} public key
  @param {string} [pubkey_prefix = 'EOS']
*/

function PublicKey(Q) {
  var pubkey_prefix = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'EOS';

  if (typeof Q === 'string') {
    var publicKey = PublicKey.fromString(Q, pubkey_prefix);
    assert(publicKey != null, 'Invalid public key');
    return publicKey;
  } else if (Buffer.isBuffer(Q)) {
    return PublicKey.fromBuffer(Q);
  } else if ((0, _typeof2["default"])(Q) === 'object' && Q.Q) {
    return PublicKey(Q.Q);
  }

  assert.equal((0, _typeof2["default"])(Q), 'object', 'Invalid public key');
  assert.equal((0, _typeof2["default"])(Q.compressed), 'boolean', 'Invalid public key');

  function toBuffer() {
    var compressed = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : Q.compressed;
    return Q.getEncoded(compressed);
  }

  var pubdata; // cache
  // /**
  //     @todo secp224r1
  //     @return {string} PUB_K1_base58pubkey..
  // */
  // function toString() {
  //     if(pubdata) {
  //         return pubdata
  //     }
  //     pubdata = `PUB_K1_` + keyUtils.checkEncode(toBuffer(), 'K1')
  //     return pubdata;
  // }

  /** @todo rename to toStringLegacy
   * @arg {string} [pubkey_prefix = 'EOS'] - public key prefix
  */

  function toString() {
    var pubkey_prefix = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 'EOS';
    return pubkey_prefix + keyUtils.checkEncode(toBuffer());
  }

  function toUncompressed() {
    var buf = Q.getEncoded(false);
    var point = ecurve.Point.decodeFrom(secp256k1, buf);
    return PublicKey.fromPoint(point);
  }
  /** @deprecated */


  function child(offset) {
    console.error('Deprecated warning: PublicKey.child');
    assert(Buffer.isBuffer(offset), "Buffer required: offset");
    assert.equal(offset.length, 32, "offset length");
    offset = Buffer.concat([toBuffer(), offset]);
    offset = hash.sha256(offset);
    var c = BigInteger.fromBuffer(offset);
    if (c.compareTo(n) >= 0) throw new Error("Child offset went out of bounds, try again");
    var cG = G.multiply(c);
    var Qprime = Q.add(cG);
    if (secp256k1.isInfinity(Qprime)) throw new Error("Child offset derived to an invalid key, try again");
    return PublicKey.fromPoint(Qprime);
  }

  function toHex() {
    return toBuffer().toString('hex');
  }

  return {
    Q: Q,
    toString: toString,
    // toStringLegacy,
    toUncompressed: toUncompressed,
    toBuffer: toBuffer,
    child: child,
    toHex: toHex
  };
}
/**
  @param {string|Buffer|PublicKey|ecurve.Point} pubkey - public key
  @param {string} [pubkey_prefix = 'EOS']
*/


PublicKey.isValid = function (pubkey) {
  var pubkey_prefix = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'EOS';

  try {
    PublicKey(pubkey, pubkey_prefix);
    return true;
  } catch (e) {
    return false;
  }
};

PublicKey.fromBinary = function (bin) {
  return PublicKey.fromBuffer(new Buffer(bin, 'binary'));
};

PublicKey.fromBuffer = function (buffer) {
  return PublicKey(ecurve.Point.decodeFrom(secp256k1, buffer));
};

PublicKey.fromPoint = function (point) {
  return PublicKey(point);
};
/**
    @arg {string} public_key - like PUB_K1_base58pubkey..
    @arg {string} [pubkey_prefix = 'EOS'] - public key prefix
    @return PublicKey or `null` (invalid)
*/


PublicKey.fromString = function (public_key) {
  var pubkey_prefix = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'EOS';

  try {
    return PublicKey.fromStringOrThrow(public_key, pubkey_prefix);
  } catch (e) {
    return null;
  }
};
/**
    @arg {string} public_key - like PUB_K1_base58pubkey..
    @arg {string} [pubkey_prefix = 'EOS'] - public key prefix

    @throws {Error} if public key is invalid

    @return PublicKey
*/


PublicKey.fromStringOrThrow = function (public_key) {
  var pubkey_prefix = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 'EOS';
  assert.equal((0, _typeof2["default"])(public_key), 'string', 'public_key');
  var match = public_key.match(/^PUB_([A-Za-z0-9]+)_([A-Za-z0-9]+)$/);

  if (match === null) {
    // legacy
    var prefix_match = new RegExp("^" + pubkey_prefix);

    if (prefix_match.test(public_key)) {
      public_key = public_key.substring(pubkey_prefix.length);
    }

    return PublicKey.fromBuffer(keyUtils.checkDecode(public_key));
  }

  assert(match.length === 3, 'Expecting public key like: PUB_K1_base58pubkey..');

  var _match = (0, _slicedToArray2["default"])(match, 3),
      keyType = _match[1],
      keyString = _match[2];

  assert.equal(keyType, 'K1', 'K1 private key expected');
  return PublicKey.fromBuffer(keyUtils.checkDecode(keyString, keyType));
};

PublicKey.fromHex = function (hex) {
  return PublicKey.fromBuffer(new Buffer(hex, 'hex'));
};

PublicKey.fromStringHex = function (hex) {
  return PublicKey.fromString(new Buffer(hex, 'hex'));
};