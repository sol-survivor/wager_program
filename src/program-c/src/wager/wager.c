/**
 * @brief C-based Wager Contract
 */
#include <solana_sdk.h>
#include "wager.h"
	
extern uint64_t entrypoint(const uint8_t *input) {
	SolAccountInfo accounts[9];
	SolParameters params = (SolParameters){.ka = accounts};

	if (!sol_deserialize(input, &params, SOL_ARRAY_SIZE(accounts))) {
		return ERROR_INVALID_ARGUMENT;
	}; 

	switch( params.data[0] ){
		case 0:
			sol_log("Setup Contract");
			return setupContract(&params);
		case 1:
			sol_log("Mint Tokens");
			return mintTokens(&params);			
		case 2:
			sol_log("Set Outcome");
			return setOutcome(&params);
		case 3:
			sol_log("Redeem");
			return redeem(&params);
		default:
			return 1;
	}
}
