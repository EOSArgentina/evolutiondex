import { ec } from 'elliptic';
import { Key, KeyType } from './eosjs-numeric';
/** Represents/stores a private key and provides easy conversion for use with `elliptic` lib */
export declare class PrivateKey {
    private key;
    constructor(key: Key);
    /** Instantiate private key from an `elliptic`-format private key */
    static fromElliptic(privKey: ec.KeyPair, keyType: KeyType): PrivateKey;
    /** Instantiate private key from an EOSIO-format private key */
    static fromString(keyString: string): PrivateKey;
    /** Export private key as `elliptic`-format private key */
    toElliptic(ecurve?: ec): ec.KeyPair;
    /** Export private key as EOSIO-format private key */
    toString(): string;
    getType(): KeyType;
}
