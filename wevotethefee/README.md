**Single action examples for the contract wevotethefee** 

Open a fee table for the evotoken EVO:

    cleos push action wevotethefee openfeetable '["YOUR_ACCOUNT", "EVO"]' -p YOUR_ACCOUNT

Vote the fee value 0.3% for the evotoken EVO.

    cleos push action wevotethefee votefee '["YOUR_ACCOUNT", "EVO", "30"]' -p YOUR_ACCOUNT

Update the fee value for the evotoken EVO. This will set the fee value to the median
of the votes. Anyone can execute this action. Consider running this action each time
you send a vote, in the same transaction.

    cleos push action wevotethefee updatefee '["EVO"]' -p YOUR_ACCOUNT

Close your vote for the evotoken EVO:

    cleos push action wevotethefee closevote '["YOUR_ACCOUNT", "EVO"]' -p YOUR_ACCOUNT

