"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var elliptic_1 = require("elliptic");
var eosjs_numeric_1 = require("./eosjs-numeric");
/** Represents/stores a private key and provides easy conversion for use with `elliptic` lib */
var PrivateKey = /** @class */ (function () {
    function PrivateKey(key) {
        this.key = key;
    }
    /** Instantiate private key from an `elliptic`-format private key */
    PrivateKey.fromElliptic = function (privKey, keyType) {
        return new PrivateKey({
            type: keyType,
            data: privKey.getPrivate().toBuffer(),
        });
    };
    /** Instantiate private key from an EOSIO-format private key */
    PrivateKey.fromString = function (keyString) {
        return new PrivateKey(eosjs_numeric_1.stringToPrivateKey(keyString));
    };
    /** Export private key as `elliptic`-format private key */
    PrivateKey.prototype.toElliptic = function (ecurve) {
        /** expensive to construct; so we do it only as needed */
        if (!ecurve) {
            if (this.key.type === eosjs_numeric_1.KeyType.r1 || this.key.type === eosjs_numeric_1.KeyType.wa) {
                ecurve = new elliptic_1.ec('p256');
            }
            else {
                ecurve = new elliptic_1.ec('secp256k1');
            }
        }
        return ecurve.keyFromPrivate(this.key.data);
    };
    /** Export private key as EOSIO-format private key */
    PrivateKey.prototype.toString = function () {
        return eosjs_numeric_1.privateKeyToString(this.key);
    };
    PrivateKey.prototype.getType = function () {
        return this.key.type;
    };
    return PrivateKey;
}());
exports.PrivateKey = PrivateKey;
//# sourceMappingURL=PrivateKey.js.map