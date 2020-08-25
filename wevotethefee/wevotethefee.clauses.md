<h1 class="clause">UserAgreement</h1>

The contract wevotethefee is designed to govern the fee values of those pair tokens from the contract evolutiondex that have wevotethefee as their fee_contract.
Users in possession of such pair tokens can vote for one of the following values:
1,2,3,5,7,10,15,20,30,50,75,100,150,200,300. One unit of fee value is equal to 0.01%.
The contract wevotethefee will change the fee value of a valid pair token when the action updatefee is applied by anyone. The new fee will be equal to the weighted median of the votes. The weight of each vote is the corresponding balance of the pair token. Note: evolutiondex notifies the fee_contract each time a user's pair token balance is modified.