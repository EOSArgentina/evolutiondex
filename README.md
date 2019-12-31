# evolutiondex
Smart contract that allows to create completely decentralized trading pairs.
It is like Ethereum's Uniswap but with some nice extra features:

(...explanation of connectors and pair-token...)

1- It can host arbitrary pairs in only one contract.

2- Each pair with its corresponding token has some settings that can be configured at initialization.

a) Fee. An initial value is selected at initialization. In addition and optionally, a name of a smart
contract is chosen. The fee might be controlled in the future by that smart contract. One good algorithm
for deciding the fee is through voting of the pair-token holders.

b) Weights. The weights of the connectors are fixed at initialization.
