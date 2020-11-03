**NOTE: In this example we use the PAIR EOS/PESO, which creates the evotoken EOSPESO. We assume an EOS token located in the eosio.token contract as usual, and a PESO token located in the contract pesocontract. You would need to replace these variables depending on your trading pairs.** 

First, let us describe the single actions of the smart contract.

Open a channel in the contract. This channel will store your trading tokens. You need to create one channel for each token you plan to trade. The second input below is the ram payer, and the authorizer must be the ram payer.

    cleos push action evolutiondex openext '["YOUR_ACCOUNT", "YOUR_ACCOUNT", {"contract":"eosio.token", "sym":"4,EOS"}]' -p YOUR_ACCOUNT

Open a channel for the second token you wish to trade in evodex:

    cleos push action evolutiondex openext '["YOUR_ACCOUNT", "YOUR_ACCOUNT", {"contract":"pesocontract", "sym":"4,PESO"}]' -p YOUR_ACCOUNT

Close the contract's channel for a specific token. In case there are funds there,
it returns them to the account "TO".

    cleos push action evolutiondex closeext '["YOUR_ACCOUNT", "TO", {"contract":"eosio.token", "sym":"4,EOS"}, "memo"]' -p YOUR_ACCOUNT

Fill your account with the desired tokens:

    cleos push action eosio.token transfer '["YOUR_ACCOUNT", "evolutiondex", "100.0000 EOS", "memo"]' -p YOUR_ACCOUNT

    cleos push action pesocontract transfer '["YOUR_ACCOUNT", "evolutiondex", "100.0000 PESO", "pesitos"]' -p YOUR_ACCOUNT

Check your open channels and balances:

    cleos get table evolutiondex YOUR_ACCOUNT evodexacnts

Withdraw funds from your opened channels, to the account "TO":

    cleos push action evolutiondex withdraw '["YOUR_ACCOUNT", "TO", {"contract":"eosio.token", "quantity":"1.0000 EOS"}, "memo"]' -p YOUR_ACCOUNT

Create the EOS/PESO evotoken. Set the initial liquidity, the initial fee for the trading pair and the fee controller.

    cleos push action evolutiondex inittoken '["YOUR_ACCOUNT", "4,EOSPESO", {"contract":"eosio.token", "quantity":"1.0000 EOS"}, {"contract":"pesocontract", "quantity":"1.0000 PESO"}, 10, "wevotethefee"]' -p YOUR_ACCOUNT

Check your evotokens balance:

    cleos get table evolutiondex YOUR_ACCOUNT accounts

Add more liquidity to a pool. Set the exact amount of evotoken to obtain, in this case 
1.5000 EOSPESO, and the maximum you are willing to pay of each token of the pair.

    cleos push action evolutiondex addliquidity '["YOUR_ACCOUNT", "1.5000 EOSPESO", 
    "2.0000 EOS", "2.0000 PESO"]' -p YOUR_ACCOUNT

Sell your evotokens and retire liquidity. The amount of evotoken is exact and the other two are minima required.

    cleos push action evolutiondex remliquidity '["YOUR_ACCOUNT", "1.0000 EOSPESO", 
    "0.1000 PESO", "1.0000 EOS"]' -p YOUR_ACCOUNT

Exchange your tokens.
There two methods. The first one is to do a transfer to the contract with a memo starting with "exchange:" and followed by the details of your operation, with the format "EVOTOKN, min_expected_asset, memo". Blank spaces before EVOTOKN, min_expected_asset and memo are ignored. The amount to be obtained by the user will be computed by the contract and executed only if it is at least min_expected_asset. 

    cleos push action eosio.token '["YOUR_ACCOUNT", "evolutiondex", "1.0000 EOS", "exchange: EOSPESO, 0.1000 PESO, memo for the transfer]' -p YOUR_ACCOUNT

The other method operates between funds already deposited in the contract. The structure
of the input is account, evotoken, extended_asset to pay (exact), asset to receive (limiting).

    cleos push action evolutiondex exchange '["YOUR_ACCOUNT", "EOSPESO", 
    {"contract":"eosio.token", "quantity":"1.0000 EOS"}, "0.1000 PESO"]' -p YOUR_ACCOUNT

It is also possible to set the exact amount to obtain and limit the amount to pay.
To do this, use negative amounts. The following example means that you want to receive exactly 0.1000 PESO and pay at most 1.0000 EOS. 

    cleos push action evolutiondex exchange '["YOUR_ACCOUNT", "EOSPESO", 
    {"contract":"eosio.token", "quantity":"-0.1000 PESO"}, "-1.0000 EOS"]' -p YOUR_ACCOUNT

Transfer your evotokens to another account:

    cleos push action evolutiondex transfer '["YOUR_ACCOUNT", "argentinaeos", "0.0001 EOSPESO", "ITS ALIVE"]' -p YOUR_ACCOUNT

See evotoken stats:

    cleos get table evolutiondex EOSPESO stat

In many practical cases, users will prefer to run many actions in a single transaction.
For example, if you want to add liquidity, you will probably prefer to close the accounts in the contract evolutiondex corresponding to the external tokens, to avoid spending RAM. To that end, you may run:

    cleos push transaction addliquidity.json

where the file addliquidity.json contains:

    {
        "actions":
        [
        {
            "account": "evolutiondex",
            "name": "openext",
            "authorization": [{"actor": "YOUR_ACCOUNT","permission": "active"}],
            "data": {
                "user": "YOUR_ACCOUNT",
                "payer": "YOUR_ACCOUNT",
                "ext_symbol": {"contract":"eosio.token", "sym":"4,EOS"}
            }
        },{
            "account": "evolutiondex",
            "name": "openext",
            "authorization": [{"actor": "YOUR_ACCOUNT","permission": "active"}],
            "data": {
                "user": "YOUR_ACCOUNT",
                "payer": "YOUR_ACCOUNT",
                "ext_symbol": {"contract":"pesocontract", "sym":"4,PESO"}
            }
        },{
            "account": "eosio.token",
            "name": "transfer",
            "authorization": [{"actor": "YOUR_ACCOUNT","permission": "active"}],
            "data": {
                "from": "YOUR_ACCOUNT",
                "to": "evolutiondex",
                "quantity": "2.0000 EOS",
                "memo": ""
            }
        },{
            "account": "pesocontract",
            "name": "transfer",
            "authorization": [{"actor": "YOUR_ACCOUNT","permission": "active"}],
            "data": {
                "from": "YOUR_ACCOUNT",
                "to": "evolutiondex",
                "quantity": "2.0000 PESO",
                "memo": ""
            }
        },{
            "account": "evolutiondex",
            "name": "addliquidity",
            "authorization": [{"actor": "YOUR_ACCOUNT","permission": "active"}],
            "data": {
                "user": "YOUR_ACCOUNT",
                "to_buy": "1.5000 EOSPESO",
                "max_asset1": "2.0000 EOS",
                "max_asset2": "2.0000 PESO",                
            }
        },{
            "account": "evolutiondex",
            "name": "closeext",
            "authorization": [{"actor": "YOUR_ACCOUNT","permission": "active"}],
            "data": {
                "user": "YOUR_ACCOUNT",
                "to": "TO",
                "ext_symbol": {"contract":"pesocontract", "sym":"4,PESO"},
                "memo": ""
            }
        },{
            "account": "evolutiondex",
            "name": "closeext",
            "authorization": [{"actor": "YOUR_ACCOUNT","permission": "active"}],
            "data": {
                "user": "YOUR_ACCOUNT",
                "to": "TO",                
                "ext_symbol": {"contract":"eosio.token", "sym":"4,EOS"},
                "memo": ""
            }
        }
        ]
    }    

The same idea applies to the operations of removing liquidity and inittoken.
Typically, a graphical user interface will perform this kind of multiaction transactions.

The fee value will be governed by the liquidity providers using the
smart contract wevotethefee.
Check the commands of wevotethefee [here](wevotethefee/vote_commands.md).