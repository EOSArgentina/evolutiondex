import { ec } from 'elliptic';
import { Key, KeyType } from './eosjs-numeric';
/** Represents/stores a Signature and provides easy conversion for use with `elliptic` lib */
export declare class Signature {
    private signature;
    constructor(signature: Key);
    /** Instantiate Signature from an EOSIO-format Signature */
    static fromString(sig: string): Signature;
    /** Instantiate Signature from an `elliptic`-format Signature */
    static fromElliptic(ellipticSig: ec.Signature, keyType: KeyType): Signature;
    /** Export Signature as `elliptic`-format Signature
     *  NOTE: This isn't an actual elliptic-format Signature, as ec.Signature is not exported by the library.
     *  That's also why the return type is `any`.  We're *actually* returning an object with the 3 params
     *  not an ec.Signature.
     *  Further NOTE: @types/elliptic shows ec.Signature as exported; it is *not*.  Hence the `any`.
     */
    toElliptic(): any;
    /** Export Signature as EOSIO-format Signature */
    toString(): string;
    /** Export Signature in binary format */
    toBinary(): Uint8Array;
    getType(): KeyType;
}
