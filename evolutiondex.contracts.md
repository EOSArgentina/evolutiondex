<h1 class="contract">openext</h1>

---
spec_version: "0.2.0"

title: Open extended Balance

summary: 'Open a zero quantity extended balance for {{nowrap user}}'
---

{{ram_payer}} agrees to establish a zero quantity extended balance for {{user}} 
for the {{ext_symbol}} extended symbol. 

If {{user}} does not have an extended balance for {{ext_symbol}}, {{ram_payer}} will be designated as the RAM payer of {{user}}'s extended balance for {{ext_symbol}}. As a result, RAM will be deducted from {{ram_payer}}’s resources to create the necessary records.

The authorization of {{ram_payer}} is required.

<h1 class="contract">closeext</h1>

---
spec_version: "0.2.0"

title: Close Extended Balance

summary: 'Close {{nowrap user}}’s extended balance'
---

{{user}} agree to close their extended balance for {{ext_symbol}}.

If the extended balance is nonzero, it will be transfered to {{to}} 
before closing it, with a memo equal to {{memo}}.
This transfer action corresponds to the contract 
{{extended_symbol_to_contract ext_symbol}}. In order to function properly,
it is necessary that the contract of {{ext_symbol}} permanently has a transfer action that satisfies the conditions (1), (2), (6) of the present contract's transfer action.

RAM will be refunded to the RAM payer of {{user}}'s extended balance for {{ext_symbol}}.

The authorization of {{user}} is required.

<h1 class="contract">ontransfer</h1>

---
spec_version: "0.2.0"

title: On transfer

summary: 'Deposit or exchange on a transfer from {{nowrap user}}'
---

This action is executed as a response to a notification of a transfer action
with the input {{from}}, {{to}}, {{quantity}}, {{memo}} in that order. 

The default response is to deposit {{quantity}} to {{from}}'s extended balance 
for the extended symbol formed by the tranfer's contract and the symbol
{{asset_to_symbol quantity}}. This is only possible if such extended balance previously
exists.

If {{memo}} starts with "deposit to:", the account {{from}} will be replaced by
the subsequent content of {{memo}} whenever it is possible.

If {{memo}} starts with "exchange:", the subsequent content of the memo is expected
to have the form "EVOTOKEN,min_expected_asset,optional memo".
An exchange operation will be processed with this data, following the same
conversion rules as in the exchange action for the input {{from}}, {{EVOTOKEN}}, {{quantity}}, {{min_expected_asset}}. If the output asset is at least equal
to {{min_expected_asset}}, it will be transfered from this contract
to {{user}}, with {{optional memo}} as memo.

In order to function properly, it is necessary that both pool contracts associated to {{EVOTOKEN}}, permanently have a transfer action that satisfies the conditions (1), (2), (6) of the present contract's transfer action.


<h1 class="contract">withdraw</h1>

---
spec_version: "0.2.0"

title: Withdraw

summary: 'Withdraw funds from extended balance'
---
{{user}} agree to withdraw the extended balance {{to_withdraw}} from their account.

In order to function properly, it is necessary that {{extended_symbol_to_contract to_withdraw}} permanently has a transfer action that satisfies the conditions (1), (2), (6) of the present contract's transfer action.


<h1 class="contract">inittoken</h1>

---
spec_version: "0.2.0"

title: Initialize token

summary: 'Initializes an evotoken by setting initial pair of token pools'
---

{{user}} agrees to initialize a pair token with symbol {{new_symbol}}, with the following initial parameters: pool1 = {{initial_pool1}}, pool2 = {{initial_pool2}}, fee = {{initial_fee}}, fee_contract = {{fee_contract}}. The extended assets {{initial_pool1}} and {{initial_pool2}} will be deducted from the corresponding extended balances of {{user}}.

In order to function properly, it is necessary that both {{extended_symbol_to_contract initial_pool1}} and  {{extended_symbol_to_contract initial_pool2 }} permanently have a transfer action that satisfies the conditions (1), (2), (6) of the present contract's transfer action.

<h1 class="contract">addliquidity</h1>

---
spec_version: "0.2.0"

title: Add liquidity

summary: '{{nowrap user}} buys an evotoken by adding liquidity to pools'
---

{{user}} agrees to buy the asset {{to_buy}} by paying no more than {{max_asset1}} and {{max_asset2}} to be deducted from {{user}}'s extended balances and added to the token
{{asset_to_symbol_code to_buy}} pools. The amount {{to_buy}} is minted by the contract.

The exact amount of the first exteded asset to be paid by {{user}} is computed as
x + y, where

x = pool1 * {{to_buy}} / supply, up to the precision of the symbol of pool1 rounded upward.
y = x * fee / 10000, up to the same precision as x, again rounded upward.
The variables pool1, supply, fee denote the parameters pool1, supply, fee associated to the token {{asset_to_symbol_code to_buy}} respectively, at the moment of operation.

The exact amount of the second extended asset to be paid by {{user}} is computed
analogously.

Authorization of {{user}} is required.
The operation is executed only if the amounts to be paid by {{user}} are at most
those indicated by {{user}}. 

<h1 class="contract">remliquidity</h1>

---
spec_version: "0.2.0"

title: Remove liquidity

summary: '{{nowrap user}} sells an evotoken, removing liquidity from pools'
---
{{user}} agrees to sell the asset {{to_sell}} by receiving at least {{min_asset1}} and {{min_asset2}} to be added to {{user}}'s extended balances and removed from the token
{{asset_to_symbol_code to_sell}} pools. The asset {{to_sell}} is retired
from circulation by the contract.

The exact amount of the first exteded asset to be received by {{user}} is computed as

pool1 * {{to_sell}} / supply, up to the precision of the symbol of pool1 rounded downward.

The variables pool1, supply denote the parameters pool1, supply associated to the token {{asset_to_symbol_code to_sell}} respectively, at the moment of operation.

The exact amount of the second extended asset to be received by {{user}} is computed
analogously. 

Authorization of {{user}} is required.
The operation is executed only if the amounts to be received by {{user}} are at least
those indicated by {{user}}. 


<h1 class="contract">exchange</h1>

---
spec_version: "0.2.0"

title: Exchange

summary: 'Exchange token through a specific pair'
---

{{user}} agree to susbstract the exact amount {{ext_asset_in}} and to add
at least {{min_expected}} to their extended balances. 
At the same time these (possibly negative) extended assets will be respectively added to and substracted from the corresponding pools associated to the token {{through}}.

The exact amount (possibly negative) to be added to {{users}}'s extended balance is computed as x + y, where

x = pool_out * {{ext_asset_in}} / (pool_in + {{ext_asset_in}}), up to the precision of the symbol of pool_out rounded downward.
y = x * fee / 10000, up to the same precision as x, again rounded downward.

The variable pool_in denotes the extended asset pool1 or pool2 associated to the token {{through}}; namely, the one whose extended symbol matches that of {{ext_asset_in}},
The variable pool_out is the extended asset pool1 or pool2, the one that is not pool_in.
The variable fee is the integer fee associated to the token {{through}}.
The values of these three variables must be taken at the moment of operation.

Authorization of {{user}} is required.
The operation is executed only if the extended asset to be added to {{user}} is at least
that indicated by {{user}}. 

<h1 class="contract">changefee</h1>

---
spec_version: "0.2.0"

title: Change fee

summary: 'Change the fee value associated to a pair'
---
The account fee_contract associated to the token {{pair_token}} authorizes
to change the fee parameter associated to the same token, to the value {{newfee}}.

<h1 class="contract">close</h1>

---
spec_version: "0.2.0"

title: Close Token Balance

summary: 'Close {{nowrap owner}}’s zero quantity balance'

icon: @ICON_BASE_URL@/@TOKEN_ICON_URI@
---

{{owner}} agree to close their zero quantity balance for the {{symbol_to_symbol_code symbol}} token.

RAM will be refunded to the RAM payer of the {{symbol_to_symbol_code symbol}} token balance for {{owner}}.


<h1 class="contract">open</h1>

---
spec_version: "0.2.0"

title: Open Token Balance

summary: 'Open a zero quantity balance for {{nowrap owner}}'
---

{{ram_payer}} agrees to establish a zero quantity balance for {{owner}} for the {{symbol_to_symbol_code symbol}} token.

If {{owner}} does not have a balance for {{symbol_to_symbol_code symbol}}, {{ram_payer}} will be designated as the RAM payer of the {{symbol_to_symbol_code symbol}} token balance for {{owner}}. As a result, RAM will be deducted from {{ram_payer}}’s resources to create the necessary records.


<h1 class="contract">transfer</h1>

---
spec_version: "0.2.0"

title: Transfer Tokens

summary: 'Send {{nowrap quantity}} from {{nowrap from}} to {{nowrap to}}'
---

(1) {{from}} sends the asset {{quantity}} to {{to}}. {{to}} receives exactly {{quantity}}.The authorization of {{from}} is required.

(2) {{#if memo}} There is a memo attached to the transfer stating: {{memo}}
{{/if}}

(3) If {{from}} is not already the RAM payer of their {{asset_to_symbol_code quantity}} token balance, {{from}} will be designated as such. As a result, RAM will be deducted from {{from}}’s resources to refund the original RAM payer.

(4) If {{to}} does not have a balance for {{asset_to_symbol_code quantity}}, {{from}} will be designated as the RAM payer of the {{asset_to_symbol_code quantity}} token balance for {{to}}. As a result, RAM will be deducted from {{from}}’s resources to create the necessary records.

(5) The account {{from}} is notified.
(6) The account {{to}} is notified.
(7) The account fee_contract corresponding to the token {{asset_to_symbol_code quantity}} is notified.