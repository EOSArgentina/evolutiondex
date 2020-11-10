**Single action examples for the contract wevotethefee** 

Open a fee table for the evotoken EVO:

    cleos push action wevotethefee openfeetable '["YOUR_ACCOUNT", "EVO"]' -p YOUR_ACCOUNT

Vote the fee value 0.3% for the evotoken EVO.

    cleos push action wevotethefee votefee '["YOUR_ACCOUNT", "EVO", "30"]' -p YOUR_ACCOUNT

Remark: the possible fee values in this version are 10, 15, 20, 30, 50, 75, 100. 
Values less than 10 or greater than 100 will be rejected. Intermediate values will be rounded upwards. In future versions, other values might be possible as well.

The fee value will automatically update to the median of the current votes
each time a vote is entered, a voter's pool token balance is modified or
a vote is closed.

Close your vote for the evotoken EVO:

    cleos push action wevotethefee closevote '["YOUR_ACCOUNT", "EVO"]' -p YOUR_ACCOUNT
