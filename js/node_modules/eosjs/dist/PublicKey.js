"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var elliptic_1 = require("elliptic");
var eosjs_numeric_1 = require("./eosjs-numeric");
/** Represents/stores a public key and provides easy conversion for use with `elliptic` lib */
var PublicKey = /** @class */ (function () {
    function PublicKey(key) {
        this.key = key;
    }
    /** Instantiate public key from an EOSIO-format public key */
    PublicKey.fromString = function (publicKeyStr) {
        return new PublicKey(eosjs_numeric_1.stringToPublicKey(publicKeyStr));
    };
    /** Instantiate public key from an `elliptic`-format public key */
    PublicKey.fromElliptic = function (publicKey, keyType) {
        var x = publicKey.getPublic().getX().toArray();
        var y = publicKey.getPublic().getY().toArray();
        return new PublicKey({
            type: keyType,
            data: new Uint8Array([(y[31] & 1) ? 3 : 2].concat(x)),
        });
    };
    /** Export public key as EOSIO-format public key */
    PublicKey.prototype.toString = function () {
        return eosjs_numeric_1.publicKeyToString(this.key);
    };
    /** Export public key as `elliptic`-format public key */
    PublicKey.prototype.toElliptic = function (ecurve) {
        /** expensive to construct; so we do it only as needed */
        if (!ecurve) {
            if (this.key.type === eosjs_numeric_1.KeyType.r1 || this.key.type === eosjs_numeric_1.KeyType.wa) {
                ecurve = new elliptic_1.ec('p256');
            }
            else {
                ecurve = new elliptic_1.ec('secp256k1');
            }
        }
        return ecurve.keyPair({
            pub: new Buffer(this.key.data),
        });
    };
    PublicKey.prototype.getType = function () {
        return this.key.type;
    };
    return PublicKey;
}());
exports.PublicKey = PublicKey;
//# sourceMappingURL=PublicKey.js.map