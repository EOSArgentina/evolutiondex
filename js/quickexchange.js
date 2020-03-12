const { Api, JsonRpc, RpcError } = require('eosjs');
const { JsSignatureProvider } = require('eosjs/dist/eosjs-jssig');      // development only
const fetch = require('node-fetch');                                    // node only; not needed in browsers
const { TextEncoder, TextDecoder } = require('util');                   // node only; native TextEncoder/Decoder

const defaultPrivateKey = "5JvA9M2LEZJEfvtv2LtsBRoAmeMiYVBnrormDRfAg2ScQG3S5tT"; // ali
const signatureProvider = new JsSignatureProvider([defaultPrivateKey]);

const rpc = new JsonRpc('http://127.0.0.1:8888', { fetch });
const api = new Api({ rpc, signatureProvider, textDecoder: new TextDecoder(), textEncoder: new TextEncoder() });

const auth = [{actor: 'ali', permission: 'active',}];


(async () => {
    const result = await api.transact({
        actions: [
        {
            account: 'evolutiondex',
            name: 'openext',
            authorization: auth,
            data: {
                user: 'ali',
                payer: 'ali',
                ext_symbol: {"contract":"eosio.token", "sym":"4,EOS"},
            },
        },{
            account: 'evolutiondex',
            name: 'openext',
            authorization: auth,
            data: {
                user: 'ali',
                payer: 'ali',
                ext_symbol: {"contract":"voice4", "sym":"4,VOICE"},
            },
        },{
            account: 'eosio.token',
            name: 'transfer',
            authorization: auth,
            data: {
                from: 'ali',
                to: 'evolutiondex',
                quantity: '1.0000 EOS',
                memo: '',
            },
        },{
            account: 'voice4',
            name: 'transfer',
            authorization: auth,
            data: {
                from: 'ali',
                to: 'evolutiondex',
                quantity: '1.0000 VOICE',
                memo: '',
            },
        },{
            account: 'evolutiondex',
            name: 'exchange',
            authorization: auth,
            data: {
                user: 'ali',
                through: '4,EVO',
                ext_asset1: {"contract":"eosio.token", "quantity":"1.0000 EOS"},
                ext_asset2: {"contract":"voice4", "quantity":"-10.0000 VOICE"}, 
            },
        },{
            account: 'evolutiondex',
            name: 'closeext',
            authorization: auth,
            data: {
                user: 'ali',
                ext_symbol: {"contract":"eosio.token", "sym":"4,EOS"},
            },
        },{
            account: 'evolutiondex',
            name: 'closeext',
            authorization: auth,
            data: {
                user: 'ali',
                ext_symbol: {"contract":"voice4", "sym":"4,VOICE"},
            },
        }
        ]
    }, {
        blocksBehind: 3,
        expireSeconds: 30,
    });
    console.dir(result);
})();
