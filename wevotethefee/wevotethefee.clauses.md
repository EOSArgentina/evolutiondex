<h1 class="clause">UserAgreement</h1>

The contract wevotethefee is designed to govern the fee values of trading pairs from the contract evolutiondex.
Users in possession of pool tokens can vote for one of the following values:
10,15,20,30,50,75,100. One unit of fee value is equal to 0.01%.
The fee value set by wevotethefee will be equal to the weighted median of the votes. The weight of each vote is the corresponding balance of the pool token. Note: evolutiondex notifies this contract each time a user's pool token balance is modified.