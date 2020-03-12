"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var BN = require("bn.js");
var eosjs_numeric_1 = require("./eosjs-numeric");
/** Represents/stores a Signature and provides easy conversion for use with `elliptic` lib */
var Signature = /** @class */ (function () {
    function Signature(signature) {
        this.signature = signature;
    }
    /** Instantiate Signature from an EOSIO-format Signature */
    Signature.fromString = function (sig) {
        return new Signature(eosjs_numeric_1.stringToSignature(sig));
    };
    /** Instantiate Signature from an `elliptic`-format Signature */
    Signature.fromElliptic = function (ellipticSig, keyType) {
        var r = ellipticSig.r.toArray();
        var s = ellipticSig.s.toArray();
        var eosioRecoveryParam;
        if (keyType === eosjs_numeric_1.KeyType.k1) {
            eosioRecoveryParam = ellipticSig.recoveryParam + 27;
            if (ellipticSig.recoveryParam <= 3) {
                eosioRecoveryParam += 4;
            }
        }
        else if (keyType === eosjs_numeric_1.KeyType.r1 || keyType === eosjs_numeric_1.KeyType.wa) {
            eosioRecoveryParam = ellipticSig.recoveryParam;
        }
        var sigData = new Uint8Array([eosioRecoveryParam].concat(r, s));
        return new Signature({
            type: keyType,
            data: sigData,
        });
    };
    /** Export Signature as `elliptic`-format Signature
     *  NOTE: This isn't an actual elliptic-format Signature, as ec.Signature is not exported by the library.
     *  That's also why the return type is `any`.  We're *actually* returning an object with the 3 params
     *  not an ec.Signature.
     *  Further NOTE: @types/elliptic shows ec.Signature as exported; it is *not*.  Hence the `any`.
     */
    Signature.prototype.toElliptic = function () {
        var lengthOfR = 32;
        var lengthOfS = 32;
        var r = new BN(this.signature.data.slice(1, lengthOfR + 1));
        var s = new BN(this.signature.data.slice(lengthOfR + 1, lengthOfR + lengthOfS + 1));
        var ellipticRecoveryBitField;
        if (this.signature.type === eosjs_numeric_1.KeyType.k1) {
            ellipticRecoveryBitField = this.signature.data[0] - 27;
            if (ellipticRecoveryBitField > 3) {
                ellipticRecoveryBitField -= 4;
            }
        }
        else if (this.signature.type === eosjs_numeric_1.KeyType.r1 || this.signature.type === eosjs_numeric_1.KeyType.wa) {
            ellipticRecoveryBitField = this.signature.data[0];
        }
        var recoveryParam = ellipticRecoveryBitField & 3;
        return { r: r, s: s, recoveryParam: recoveryParam };
    };
    /** Export Signature as EOSIO-format Signature */
    Signature.prototype.toString = function () {
        return eosjs_numeric_1.signatureToString(this.signature);
    };
    /** Export Signature in binary format */
    Signature.prototype.toBinary = function () {
        return this.signature.data;
    };
    Signature.prototype.getType = function () {
        return this.signature.type;
    };
    return Signature;
}());
exports.Signature = Signature;
//# sourceMappingURL=Signature.js.map