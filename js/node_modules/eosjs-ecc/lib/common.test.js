"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

var _slicedToArray2 = _interopRequireDefault(require("@babel/runtime/helpers/slicedToArray"));

var _regenerator = _interopRequireDefault(require("@babel/runtime/regenerator"));

var _typeof2 = _interopRequireDefault(require("@babel/runtime/helpers/typeof"));

var _asyncToGenerator2 = _interopRequireDefault(require("@babel/runtime/helpers/asyncToGenerator"));

/* eslint-env mocha */
var assert = require('assert');

var ecc = require('.');

var wif = '5KYZdUEo39z3FPrtuX2QbbwGnNP5zTd7yyr2SC1j299sBCnWjss';
describe('Common API', function () {
  it('unsafeRandomKey',
  /*#__PURE__*/
  (0, _asyncToGenerator2["default"])(
  /*#__PURE__*/
  _regenerator["default"].mark(function _callee() {
    var pvt;
    return _regenerator["default"].wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            _context.next = 2;
            return ecc.unsafeRandomKey();

          case 2:
            pvt = _context.sent;
            assert.equal((0, _typeof2["default"])(pvt), 'string', 'pvt');
            assert(/^5[HJK]/.test(wif)); // assert(/^PVT_K1_/.test(pvt)) // todo

          case 5:
          case "end":
            return _context.stop();
        }
      }
    }, _callee);
  })));
  it('seedPrivate', function () {
    assert.equal(ecc.seedPrivate(''), wif); // assert.equal(ecc.seedPrivate(''), 'PVT_K1_2jH3nnhxhR3zPUcsKaWWZC9ZmZAnKm3GAnFD1xynGJE1Znuvjd')
  });
  it('privateToPublic', function () {
    // const pub = 'PUB_K1_859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2Ht7beeX'
    var pub = 'EOS859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2HqhToVM';
    assert.equal(ecc.privateToPublic(wif), pub);
  });
  it('isValidPublic', function () {
    var keys = [[true, 'PUB_K1_859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2Ht7beeX'], [true, 'EOS859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2HqhToVM'], [false, 'MMM859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2HqhToVM'], [false, 'EOS859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2HqhToVm', 'EOS'], [true, 'PUB859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2HqhToVM', 'PUB'], [false, 'PUB859gxfnXyUriMgUeThh1fWv3oqcpLFyHa3TfFYC4PK2HqhToVm', 'PUB']];

    for (var _i = 0, _keys = keys; _i < _keys.length; _i++) {
      var key = _keys[_i];

      var _key = (0, _slicedToArray2["default"])(key, 3),
          valid = _key[0],
          pubkey = _key[1],
          prefix = _key[2];

      assert.equal(valid, ecc.isValidPublic(pubkey, prefix), pubkey);
    }
  });
  it('isValidPrivate', function () {
    var keys = [[true, '5KYZdUEo39z3FPrtuX2QbbwGnNP5zTd7yyr2SC1j299sBCnWjss'], [false, '5KYZdUEo39z3FPrtuX2QbbwGnNP5zTd7yyr2SC1j299sBCnWjsm']];

    for (var _i2 = 0, _keys2 = keys; _i2 < _keys2.length; _i2++) {
      var key = _keys2[_i2];
      assert.equal(key[0], ecc.isValidPrivate(key[1]), key[1]);
    }
  });
  it('hashs', function () {
    var hashes = [// ['sha1', 'da39a3ee5e6b4b0d3255bfef95601890afd80709'],
    ['sha256', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855']];

    for (var _i3 = 0, _hashes = hashes; _i3 < _hashes.length; _i3++) {
      var hash = _hashes[_i3];
      assert.equal(ecc[hash[0]](''), hash[1]);
      assert.equal(ecc[hash[0]](Buffer.from('')), hash[1]);
    }
  });
  it('signatures', function () {
    var pvt = ecc.seedPrivate('');
    var pubkey = ecc.privateToPublic(pvt);
    var data = 'hi';
    var dataSha256 = ecc.sha256(data);
    var sigs = [ecc.sign(data, pvt), ecc.signHash(dataSha256, pvt)];

    for (var _i4 = 0, _sigs = sigs; _i4 < _sigs.length; _i4++) {
      var sig = _sigs[_i4];
      assert(ecc.verify(sig, data, pubkey), 'verify data');
      assert(ecc.verifyHash(sig, dataSha256, pubkey), 'verify hash');
      assert.equal(pubkey, ecc.recover(sig, data), 'recover from data');
      assert.equal(pubkey, ecc.recoverHash(sig, dataSha256), 'recover from hash');
    }
  });
});
describe('Common API (initialized)', function () {
  it('initialize', function () {
    return ecc.initialize();
  });
  it('randomKey', function () {
    var cpuEntropyBits = 1;
    ecc.key_utils.addEntropy(1, 2, 3);
    var pvt = ecc.unsafeRandomKey().then(function (pvt) {
      assert.equal((0, _typeof2["default"])(pvt), 'string', 'pvt');
      assert(/^5[HJK]/.test(wif)); // assert(/^PVT_K1_/.test(pvt))
    });
  });
});