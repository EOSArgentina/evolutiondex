<h1 class="clause">UserAgreement</h1>

In order to function properly, the external tokens to be operated by this contract must permanently have a transfer action with input variables {{from}}, {{to}}, {{quantity}}, {{memo}} satisfying the following three conditions:
* {{from}} sends the asset {{quantity}} to {{to}}. {{to}} receives exactly {{quantity}}. The authorization of {{from}} is required.
* {{#if memo}} There is a memo attached to the transfer stating: {{memo}} {{/if}}
* The account {{to}} is notified.

The user agrees that the present contract is not responsible for errors ocurred as a consequence of using a token not satisfying the above conditions.