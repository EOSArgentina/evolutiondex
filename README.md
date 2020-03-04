# evolutiondex


This is a smart contract for EOSIO that allows to create continuous
liquidity pools for any pair of tokens in the chain. It facilitates the decentralized
exchange and offers an interesting financial position for the
liquidity providers.
Evolutiondex follows the line initiated by
Bancor and Uniswap, but with some design improvements that we explain below.

1- Evotokens. For each registered pair there will be a standard token backed by
the assets in the corresponding pair of pools. These new tokens can be freely
transferred, facilitating the access and management of the investment position.
From now on we will call these tokens "evotokens".

In evolutiondex, each trading pair has an associated fee value that might be variable.
The value of evotokens rises as they collect fees from those exchanging through
that pair or those adding more liquidity. The action of removing liquidity
(selling evotokens) is free of charge.

2- Initial fee and fee governance. A fee value is set at initialization of each
trading pair. In addition and optionally, a name of a smart contract
is chosen: the fee contract. The fee value might be controlled in the future by that smart contract.
One good choice for the fee governance is through voting of the evotoken holders.
However, in some cases, a well known dictator might be a good choice as well.
Interestingly, the evolutiondex smart contract is designed in such a way that the specific method of fee governance for each evotoken is decoupled from the rest of the functionality.


**On the formulas determining prices**

We chose to follow the usual criterion: after an exchange operation
for a given pair, the product of the amounts in the pools of the
corresponding pair must remain equal before the fee is charged. After
the fee, that product will rise accordingly. All the rounding errors will favour the pools
in order to avoid the gaming of the system. This criterion completely determines
the price behaviour. Notice that whenever the amount to exchange is small compared to the
pool sizes the price is approximately equal to the quotient between the amounts in the pools.

When adding or removing liquidity, the (again standard) criterion is to
keep fixed the ratios between minted evotokens and the amounts in the pools that back
their value. Actually a correction is in order for the case of adding liquidity:
the fee charged will increase the value of the evotoken afterwards.


**Some considerations from the perspective of liquidity providers**

Being a liquidity provider is a financial position that deserves a
careful analysis. It is necessary to understand the exposition to
gains and losses for the different future scenarios. This will be addressed
in a different article.
Fee governance is an important tool, since the optimal fee parameter
is expected to change according to the mood of the market. For example,
high volatility suggests high fee. Another relevant factor is the competition from
other exchange opportunities. Without the ability to change the fee, liquidity
providers might want to move their funds from one place to the other, thus
discouraging them to invest in the first place. Therefore a dynamic fee offers
a practical way to benefit from the large exchange activity in the cryptocurrency
space.

**From the perspective of exchangers**

Continuous liquidity pools offer the advantages of decentralized exchange.
There is no need to trust funds to an institution. The prices are computed
algorithmically according to the available liquidity. High liquidity
will translate to low price slippage (this is the price variation as
the exchanged amount varies).
Conversely, if the liquidity pools are small, there will be considerable
slippage and the exchange will only be practical for tiny amounts.


**References:** artículos de Bancor, de Uniswap, y Eos Argentina (2018).
