import { ec } from 'elliptic';
import { Key, KeyType } from './eosjs-numeric';
/** Represents/stores a public key and provides easy conversion for use with `elliptic` lib */
export declare class PublicKey {
    private key;
    constructor(key: Key);
    /** Instantiate public key from an EOSIO-format public key */
    static fromString(publicKeyStr: string): PublicKey;
    /** Instantiate public key from an `elliptic`-format public key */
    static fromElliptic(publicKey: ec.KeyPair, keyType: KeyType): PublicKey;
    /** Export public key as EOSIO-format public key */
    toString(): string;
    /** Export public key as `elliptic`-format public key */
    toElliptic(ecurve?: ec): ec.KeyPair;
    getType(): KeyType;
}
