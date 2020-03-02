# evolutiondex
This is a smart contract that allows to create continuous
liquidity pools for any pair of tokens. It facilitates the decentralized
exchange and offers an interesting financial position for the
liquidity providers.
Evolutiondex follows the line initiated by
Bancor and Uniswap, but with some design improvements that we explain below.

1- For each registered pair there will be a standard token backed by 
the assets in the corresponding pair of pools. These new tokens can be traded
anywhere, facilitating the access and management of the investment position.
From now on we will call these tokens "evotokens".

In evolutiondex, each trading pair has a fee that might be variable. The utility
of the evotokens is the ability to collect fees from those exchanging through
that pair or adding liquidity. The action of removing liquidity (selling evotokens)
is free of charge.

2- Initial fee and fee governance. An initial fee is set at initialization of each 
trading pair. In addition and optionally, a name of a smart contract is chosen: 
the fee contract. The fee might be controlled in the future by that smart contract.
One good choice for the fee governance is through voting of the evotoken holders.
However, in some cases, a well known dictator might be a good choice as well.

# From the perspective of liquidity providers

The fee governance is an important tool, since the optimal fee parameter
is expected to change according to the mood of the market. For example, 
high volatility suggests high fee. Another relevant factor is the competition from 
other exchange opportunities. Without the ability to change the fee, liquidity
providers might want to withdraw their funds from one place to the other, thus
discouraging them to invest in the first place. Therefore a dynamic fee offers
a practical way to benefit from the giant exchange activity in the cryptocurrency
space. 

# From the perspective of exchangers

Continuous liquidity pools offer the advantages of decentralized exchange.
There is no need to trust funds to an institution. The prices are computed
algorithmically according to the available liquidity.