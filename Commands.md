**NOTE: In this example we use the PAIR EOS/PESO, which creates the Evotoken EOSPESO. The EOS token is located in the eosio.token cotract and the PESO is a token created for demostrative purposes on the Kylin Testnet, located in the contract pesoevotoken. You would need to replace these variables depending on your trading pairs.** 

Open a channel in the contract: - this channel will store your trading tokens. You need to create 1 channel for each token you plan to trade -

    cleos push action evolutiondex openext '["YOUR_ACCOUNT", "YOUR_ACCOUNT", {"contract":"eosio.token", "sym":"4,EOS"}]' -p YOUR_ACCOUNT

Open a channel for the second token you wish to trade in evodex:

    cleos push action evolutiondex openext '["YOUR_ACCOUNT", "YOUR_ACCOUNT", {"contract":"pesoevotoken", "sym":"4,PESO"}]' -p YOUR_ACCOUNT

Close the contract's channel for a specific token:

cleos push action evolutiondex closeext '["YOUR_ACCOUNT", {"contract":"eosio.token", "sym":"4,EOS"}]' -p YOUR_ACCOUNT

Fill your account with the desired tokens:

    cleos push action eosio.token transfer '["YOUR_ACCOUNT", "evolutiondex", "100.0000 EOS", "memo"]' -p YOUR_ACCOUNT

    cleos push action pesoevotoken transfer '["YOUR_ACCOUNT", "evolutiondex", "100.0000 PESO", "pesitos"]' -p YOUR_ACCOUNT

Check your open channels and balances:

    cleos get table evolutiondex YOUR_ACCOUNT evodexacnts

Withdraw funds from your opened channels:

    cleos -v push action evolutiondex withdraw '["YOUR_ACCOUNT", {"contract":"eosio.token", "quantity":"1.0000 EOS"}]' -p YOUR_ACCOUNT

Open the EOS/PESO Evotoken: - add liquidity to the Pair Pool, set the initial fee for the trading pair and the fee controller -

    cleos push action evolutiondex inittoken '["YOUR_ACCOUNT", "4,EOSPESO", {"contract":"pesoevotoken", "quantity":"1 PESO"}, {"contract":"eosio.token", "quantity":"1.0000 EOS"}, 10, "YOUR_ACCOUNT"]' -p YOUR_ACCOUNT

Check your Evotokens balance:

    cleos get table evolutiondex YOUR_ACCOUNT accounts

Add more liquidity to a pool: - set the exact amount of evotoken to obtain, in this case 1.0000 EOSPESO, 
and the maximum you are willing to pay of each token of the pair. -

    cleos push action evolutiondex addliquidity '["YOUR_ACCOUNT", "1.0000 EOSPESO", {"contract":"pesoevotoken", "quantity":"2.0000 PESO"},{"contract":"eosio.token", "quantity":"2.0000 EOS"}]' -p YOUR_ACCOUNT

Sell your evotokens and retire liquidity: - the amount of evotoken is exact and the other two are minima required. -

    cleos push action evolutiondex remliquidity '["YOUR_ACCOUNT", "1.0000 EOSPESO", {"contract":"pesoevotoken", "quantity":"0.1000 PESO"},{"contract":"eosio.token", "quantity":"1.0000 EOS"}]' -p YOUR_ACCOUNT

Exchange your tokens: - The negative input is the amount you want to obtain from the exchange while the positive
one is what you are willing to pay. The order of the input tokens must match the order at initialization.
The first amount will be exact, and the second will be taken as a maximum allowed by the user. -

    cleos push action evolutiondex exchange '["YOUR_ACCOUNT", "4,EOSPESO", {"contract":"pesoevotoken", "quantity":"-0.1000 PESO"},{"contract":"eosio.token", "quantity":"1.0000 EOS"}]' -p YOUR_ACCOUNT

Transfer your Evotokens to another account:

    cleos push action evolutiondex transfer '["YOUR_ACCOUNT", "argentinaeos", "0.0001 EOSPESO", "ITS ALIVE"]' -p YOUR_ACCOUNT

See Evotoken Stats:

    cleos get table evolutiondex EOSPESO stat
