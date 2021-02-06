#pragma once
#include <solana_sdk.h>

//Structures
typedef struct {
	uint8_t initialized[4]; /** initialized 0*/
	SolPubkey mauthority; /** mint authority 4*/
	uint8_t supply[8]; /** empty 36*/
	uint8_t decimals; /** supply 44*/  
	uint8_t empty[5]; /** empty 45*/  
	SolPubkey fauthority; /** freeze authority 46*/
} MintBody;

typedef struct {
	uint64_t endTime; /** endTime 0*/
	SolPubkey p1Mint; /** p1 Mint 8*/
	SolPubkey p2Mint; /** p2 Mint 40*/
	SolPubkey potTokenAccount; /**  potTokenAccount 72*/
	SolPubkey oracleAccount; /** Outcome Oracle Account 104*/	
	SolPubkey feeAccount; /** Fee Recipient Account 136*/		
	uint64_t fee; /** fee 168*/	
	uint64_t minimumBet; /** Postion of Outcome within Oracle Account  176*/				
	uint8_t overRideTime; /** close contract before time has expired,also seed value for contract  184*/
	uint8_t outcome; /** outcome result [0,1]  185*/	
} Contract;

typedef struct {
    SolPubkey mint;
    SolPubkey owner;
    uint64_t amount;
} TokenAccount;


//helpers
uint64_t LEbytesto64(uint8_t *arr){
	uint64_t number64 = (uint64_t)(
			(unsigned long)( arr[0]) << 0 |
            (unsigned long)( arr[1]) << 8 |
            (unsigned long)( arr[2]) << 16 |
            (unsigned long)( arr[3]) << 24 |
			(unsigned long)( arr[4]) << 32 |
            (unsigned long)( arr[5]) << 40 |
            (unsigned long)( arr[6]) << 48 |
            (unsigned long)( arr[7]) << 56
     );	
     return number64;
}

void BE64toBytes(uint8_t offset, uint8_t* byteArray,uint64_t number64){
	unsigned long number =  (unsigned long)number64;
	byteArray[7+offset] = (number >> 56) & 0xFF;
	byteArray[6+offset] = (number >> 48) & 0xFF;
	byteArray[5+offset] = (number >> 40) & 0xFF;
	byteArray[4+offset] = (number >> 32) & 0xFF;
    byteArray[3+offset] = (number >> 24) & 0xFF;
	byteArray[2+offset] = (number >> 16) & 0xFF;
	byteArray[1+offset] = (number >> 8) & 0xFF;
	byteArray[0+offset] = number & 0xFF;
}

SolPubkey programSigner(SolParameters *params,uint8_t seed){
	uint8_t seed1[] = { seed };
	const SolSignerSeed seeds1[] = { {seed1, SOL_ARRAY_SIZE(seed1)} };	
	SolPubkey xaddress;
	sol_create_program_address(
		seeds1,
		SOL_ARRAY_SIZE(seeds1),
		params->program_id,
		&xaddress
	);
	return xaddress;	
}

uint64_t getTime(SolParameters *params){
	SolAccountInfo *Clock = &params->ka[1];
	int offset = 32;
	uint64_t time = (uint64_t)(
			(unsigned long)(Clock->data[ offset + 0]) << 56 |
            (unsigned long)(Clock->data[ offset + 1]) << 48 |
            (unsigned long)(Clock->data[ offset + 2]) << 40 |
            (unsigned long)(Clock->data[ offset + 3]) << 32 |
			(unsigned long)(Clock->data[ offset + 4]) << 24 |
            (unsigned long)(Clock->data[ offset + 5]) << 16 |
            (unsigned long)(Clock->data[ offset + 6]) << 8 |
            (unsigned long)(Clock->data[ offset + 7])
     );	
     return time;
}

//Validators

bool CleanAccount(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];
	bool isClean = true;	
	for(int i = 0;i < Account->data_len;i++){
		if(Account->data[i] != 0){
			sol_log("Dirty Account");
			isClean = false;
			break;
		}
	}
	sol_log("account checked");		
	return isClean;
}

bool isOwner(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];
	bool result = false;
	if (SolPubkey_same(Account->owner, params->program_id)) {
		result = true;
	}
	sol_log("owner checked");		
	return result;
}

bool Pending(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];	
	Contract *contract = (Contract *)Account->data;	
	bool valid = false;
	if(contract->outcome == 0){
		valid = true;
	}
	else{
		sol_log("Outcome not pending");
	}
	sol_log("Pending outcome checked");	
	return valid;
}

bool ValidClock(SolParameters *params){
	uint8_t publicClock[32] = {
		0x6,0xa7,0xd5,0x17,0x18,0xc7,0x74,	
		0xc9,0x28,0x56,0x63,0x98,0x69,0x1d,0x5e,	
		0xb6,0x8b,0x5e,0xb8,0xa3,0x9b,0x4b,0x6d, 	
		0x5c,0x73,0x55,0x5b,0x21,0x0,0x0,0x0,0x0};
	SolAccountInfo *Clock = &params->ka[1];
	uint8_t x = 0;
	for( int i = 0;i < 32;i++){
		if( publicClock[i] != Clock->key->x[i] ){
			x++;
			break;
		}
	}
	bool isGoodClock = x == 0 ? true : false;
	sol_log("clock checked");	
	return isGoodClock;
}

bool ValidEndTime(SolParameters *params){
	uint8_t _endTime[8] = { params->data[4],params->data[5],params->data[6],params->data[7],params->data[8],params->data[9],params->data[10],params->data[11] };
	uint64_t endTime = LEbytesto64(_endTime);
	bool isValidEndTime = endTime > 0 ? true : false;
	sol_log("time checked");	
	return isValidEndTime;
}

bool ValidOracle(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];
	SolAccountInfo *OracleAccount = &params->ka[2];
	Contract *contract = (Contract *)Account->data;	
	bool valid = false;
	if( SolPubkey_same(&contract->oracleAccount,OracleAccount->key) ){
		valid = true;
	}
	if(!OracleAccount->is_signer){
		sol_log("Oracle Account has not signed tx");
		valid = false;
	}
	sol_log("oracle account checked");
	return valid;
}

bool ValidFeeAccount(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];
	SolAccountInfo *FeeAccount = &params->ka[8];
	Contract *contract = (Contract *)Account->data;	
	bool valid = false;
	if( SolPubkey_same(&contract->feeAccount,FeeAccount->key) ){
		valid = true;
	}
	else{
		sol_log("invalid fee account");
	}
	sol_log("fee account checked");
	return valid;
}

bool ValidOutcome(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];	
	SolAccountInfo *Mint = &params->ka[2];
	Contract *contract = (Contract *)Account->data;	
	bool valid = true;
	if(contract->outcome == 1 && !SolPubkey_same(Mint->key,&contract->p1Mint) ){
		//Not a valid winner
		valid = false;
	}
	if(contract->outcome == 2 && !SolPubkey_same(Mint->key,&contract->p2Mint) ){
		//Not a valid winner
		valid = false;
	}	
	if(!valid){
		sol_log("Not a valid ticket");
	}
	sol_log("outcome checked");
	return valid;
}

bool ValidMint(SolParameters *params,uint8_t seed){
	SolAccountInfo *TokenMint = &params->ka[2];
	MintBody *mint = (MintBody *)TokenMint->data;
	bool valid = false;	
	SolPubkey PS = programSigner(params,seed);
	if( SolPubkey_same(&mint->mauthority,&PS) ){
		valid = true;
	}
	else{	
		sol_log("Invalid Mint");	
		valid = false;
	}
	sol_log("Mint checked");	
	return valid;
}

bool ValidMints(SolParameters *params){
	bool valid = true;
	uint8_t TokenProgram[32] = { 
		0x6, 0xdd, 0xf6, 0xe1, 0xd7,
		0x65, 0xa1, 0x93, 0xd9, 0xcb,
		0xe1, 0x46, 0xce, 0xeb, 0x79,
		0xac, 0x1c, 0xb4, 0x85, 0xed,
		0x5f, 0x5b, 0x37, 0x91, 0x3a,
		0x8c, 0xf5, 0x85, 0x7e, 0xff,
		0x0, 0xa9};	
	SolAccountInfo *TokenMint1 = &params->ka[2];
	SolAccountInfo *TokenMint2 = &params->ka[3];	
	MintBody *mint1 = (MintBody *)TokenMint1->data;
	MintBody *mint2 = (MintBody *)TokenMint2->data;
	uint8_t seed = params->data[1];
	if(mint1->initialized[0] == 0 || mint2->initialized[0] == 0){
		sol_log("Invalid Token Mint");
		valid = false;
	}
	if(mint1->initialized[0] == 0 || mint2->initialized[0] == 0){
		sol_log("Invalid Token Mint");
		valid = false;
	}
	if((mint1->decimals < 6 || mint2->decimals < 6) || (mint1->decimals != mint2->decimals) ){
		sol_log("Invalid Decimals");
		valid = false;
	}
	if( LEbytesto64(mint1->supply) > 0 || LEbytesto64(mint2->supply) > 0 ){
		sol_log("Invalid Token Mint Supply");
		valid = false;
	}	
	if( !SolPubkey_same(&mint1->mauthority,&mint2->mauthority) ){
		sol_log("Mismatch Token Mint Authority");
		valid = false;
	}
	SolPubkey PS = programSigner(params,seed);
	if( !SolPubkey_same(&mint1->mauthority, &PS) ){
		sol_log("Invalid Mint Authority");
		valid = false;
	}	
	if( !SolPubkey_same(&mint1->fauthority,&mint2->fauthority) ){
		sol_log("Mismatch Token Freeze Authority");
		valid = false;
	}		
	for(int i = 0;i < 32;i++){
		if( mint1->fauthority.x[i] != 0){
			sol_log("Invalid Token Freeze Authority");
			valid = false;
			break;
		}
	}	
	if( !SolPubkey_same(TokenMint1->owner,TokenMint2->owner) ){
		sol_log("Mismatch Account Owner");
		valid = false;
	}	
	for(int i = 0;i < 32;i++){
		if( TokenMint1->owner->x[i] != TokenProgram[i] ){
			sol_log("Invalid Account Owner");
			valid = false;
			break;
		}
	}
	sol_log("mints checked");	
	return valid;
}

bool ValidPot(SolParameters *params,uint8_t ptk,uint8_t seed){
	SolAccountInfo *PotTokenAccount = &params->ka[ptk];
	SolPubkey PS = programSigner(params,seed);
	bool validPot = true;
	uint8_t TokenProgram[32] = { 
		0x6, 0xdd, 0xf6, 0xe1, 0xd7,
		0x65, 0xa1, 0x93, 0xd9, 0xcb,
		0xe1, 0x46, 0xce, 0xeb, 0x79,
		0xac, 0x1c, 0xb4, 0x85, 0xed,
		0x5f, 0x5b, 0x37, 0x91, 0x3a,
		0x8c, 0xf5, 0x85, 0x7e, 0xff,
		0x0, 0xa9};	
	for(int i = 0;i < 32;i++){
		if( PotTokenAccount->owner->x[i] != TokenProgram[i] ){
			sol_log("Invalid Token Program Owner");
			validPot = false;
			break;
		}
	}
	for(int i = 0;i < 32;i++){
		if( PotTokenAccount->data[32+i] != PS.x[i] ){
			sol_log("Contract Does Not Own Account");
			validPot = false;
			break;
		}
	}	
	sol_log("token pot checked");	
	return validPot;
}

bool ValidTime(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];	
	Contract *contract = (Contract *)Account->data;	
	bool valid = false;
	uint64_t now = getTime(params);
	//sol_log_64(now,contract->endTime,contract->endTime-now,0,0);
	if(now < contract->endTime && contract->outcome == 0){
		valid = true;
	}
	else{
		sol_log_64(now,0,0,0,contract->endTime);
		sol_log("Mint period has ended / Outcome Decided");
	}
	sol_log("Time checked");	
	return valid;
}

bool CanOverRide(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];	
	Contract *contract = (Contract *)Account->data;	
	bool valid = false;
	if(contract->overRideTime > 0){
		valid = true;
		sol_log("Can OverRide Contract Window");
	}
	sol_log("OverRideTime checked");	
	return valid;
}

bool BuildContract(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];
	SolAccountInfo *SystemClock = &params->ka[1];
	SolAccountInfo *TokenMint1 = &params->ka[2];
	SolAccountInfo *TokenMint2 = &params->ka[3];
	SolAccountInfo *PotTokenAccount = &params->ka[4];
	SolAccountInfo *OracleAccount = &params->ka[5];
	SolAccountInfo *FeeAccount = &params->ka[6];
	Contract *contract = (Contract *)Account->data;
	bool valid = false;
	uint8_t endTime_[8]= { params->data[17],params->data[16],params->data[15],params->data[14],params->data[13],params->data[12],params->data[11],params->data[10] };
	contract->endTime = LEbytesto64(endTime_) + getTime(params);
	contract->p1Mint = *TokenMint1->key;	
	contract->p2Mint = *TokenMint2->key;
	contract->potTokenAccount = *PotTokenAccount->key;
	contract->oracleAccount = *OracleAccount->key;
	contract->feeAccount = *FeeAccount->key;
	uint8_t minimumBet_[8]= { params->data[2],params->data[3],params->data[4],params->data[5],params->data[6],params->data[7],params->data[8],params->data[9] };
	uint8_t fee_[8]= { params->data[18],params->data[19],params->data[20],params->data[21],params->data[22],params->data[23],params->data[24],params->data[25] };
	contract->fee = LEbytesto64(fee_); 
	contract->overRideTime = params->data[26];
	contract->minimumBet = LEbytesto64(minimumBet_);
	if( contract->minimumBet > 0 && LEbytesto64(endTime_) > 0 ){
		valid = true;
	}
	//Make sure we have a complete contract
	if(Account->data_len < 186){
		valid = false;
		sol_log("Bad Account Size");
	}
	return valid;
}

bool BurnTokens(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];	
	SolAccountInfo *Mint = &params->ka[2];
	SolAccountInfo *Authority = &params->ka[5];
	SolAccountInfo *BurnAccount = &params->ka[7];				
	SolAccountInfo *TokenProgram = &params->ka[6];
	Contract *contract = (Contract *)Account->data;
	TokenAccount *burnAccount = (TokenAccount *)BurnAccount->data;	
	bool txComplete = false;
	uint8_t seed1[] = { params->data[1] };
	const SolSignerSeed seeds1[] = { {seed1, SOL_ARRAY_SIZE(seed1)} };	
	const SolSignerSeeds signer_seeds[] = { {seeds1, SOL_ARRAY_SIZE(seeds1)} };
	SolAccountMeta burnArguments[] = {
		  {BurnAccount->key,true,false},
          {Mint->key, true, false},
          {Authority->key, false, true},
    };
    uint8_t burnData[] = { 8,0,0,0,0,0,0,0,0 };
	BE64toBytes(1,burnData,burnAccount->amount); 
	const SolInstruction burnPxInstruction = {
		TokenProgram->key,
		burnArguments, SOL_ARRAY_SIZE(burnArguments),
		burnData,SOL_ARRAY_SIZE(burnData)
    };

	if( SUCCESS == sol_invoke_signed(&burnPxInstruction, params->ka,8,signer_seeds,1) ){ 
		sol_log("burn tx complete");                
		 txComplete = true;
	}
	return txComplete;
}

bool FinalizeContract(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];	
	SolAccountInfo *OracleAccount = &params->ka[2];
	Contract *contract = (Contract *)Account->data;			
	bool finalize = false;
	//Only settle this once
	if( contract->outcome == 0 ){
		contract->outcome = params->data[1];
		finalize = true;
	}
	return finalize;
}

bool Transfer2Pot(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];	
	SolAccountInfo *wagerTokenProgramAccount = &params->ka[3];
	SolAccountInfo *wagerTokenUserAccount = &params->ka[4];	
	SolAccountInfo *ProgramAuthority = &params->ka[5];
	SolAccountInfo *TokenProgram = &params->ka[6];	
	Contract *contract = (Contract *)Account->data;			
	bool txComplete = false;
	uint8_t seed1[] = { params->data[1] };
	const SolSignerSeed seeds1[] = { {seed1, SOL_ARRAY_SIZE(seed1)} };	
	const SolSignerSeeds signer_seeds[] = { {seeds1, SOL_ARRAY_SIZE(seeds1)} };
	SolAccountMeta delegateTransferArguments[] = {
          {wagerTokenUserAccount->key, true, false},
          {wagerTokenProgramAccount->key,true,false},
          {ProgramAuthority->key, false, true},
    };    
    uint8_t transferData[] = { 3,params->data[2], params->data[3], params->data[4],params->data[5], params->data[6], params->data[7], params->data[8], params->data[9] };
	const SolInstruction delegateTransferInstruction = {
		TokenProgram->key,
		delegateTransferArguments, SOL_ARRAY_SIZE(delegateTransferArguments),
		transferData, SOL_ARRAY_SIZE(transferData) 
    };    
    if( SUCCESS == sol_invoke_signed(&delegateTransferInstruction, params->ka,8,signer_seeds,1) ){ 
		sol_log("delegate transfer tx complete");   
		txComplete = true;             
	}
	else{
		sol_log("delegate transfer tx failed"); 
	}
	/////////// Pay Dues
	if(contract->fee > 0){
		SolAccountInfo *FeeAccount = &params->ka[8];
		if(!ValidFeeAccount(params)){txComplete = false;}
		else{
			SolAccountMeta feeTransferArguments[] = {
				  {wagerTokenUserAccount->key, true, false},
				  {FeeAccount->key,true,false},
				  {ProgramAuthority->key, false, true},
				};    
			uint8_t feeData[] = { 3,0,0,0,0,0,0,0,0 };
			BE64toBytes(1,feeData,contract->fee); 
			const SolInstruction feeTransferInstruction = {
				TokenProgram->key,
				feeTransferArguments, SOL_ARRAY_SIZE(feeTransferArguments),
				feeData, SOL_ARRAY_SIZE(feeData) 
			};  
			if( SUCCESS == sol_invoke_signed(&feeTransferInstruction, params->ka,9,signer_seeds,1) ){ 
				sol_log("fee transfer tx complete");   
			}
			else{
				sol_log("fee transfer tx failed"); 
				txComplete = false;             
			} 
		}	

	}
	////////////////
	return txComplete;
}

bool Transfer2User(SolParameters *params){
	SolAccountInfo *Account = &params->ka[0];		
	SolAccountInfo *TokenMint = &params->ka[2];	
	SolAccountInfo *wagerTokenProgramAccount = &params->ka[3];
	SolAccountInfo *wagerTokenUserAccount = &params->ka[4];	
	SolAccountInfo *ProgramAuthority = &params->ka[5];
	SolAccountInfo *TokenProgram = &params->ka[6];	
	SolAccountInfo *UserMintTokenAccount = &params->ka[7];	
	Contract *contract = (Contract *)Account->data;	
	MintBody *mint = (MintBody *)TokenMint->data;	
	TokenAccount *userMintTokenAccount = (TokenAccount *)UserMintTokenAccount->data;		
	bool txComplete = false;
	uint8_t seed1[] = { params->data[1] };
	const SolSignerSeed seeds1[] = { {seed1, SOL_ARRAY_SIZE(seed1)} };	
	const SolSignerSeeds signer_seeds[] = { {seeds1, SOL_ARRAY_SIZE(seeds1)} };
	SolAccountMeta transferArguments[] = {
		  {wagerTokenProgramAccount->key,true,false},		
          {wagerTokenUserAccount->key, true, false},
          {ProgramAuthority->key, false, true},
    };        
    uint8_t potAmount[] = { wagerTokenProgramAccount->data[64], wagerTokenProgramAccount->data[65], wagerTokenProgramAccount->data[66],wagerTokenProgramAccount->data[67], wagerTokenProgramAccount->data[68], wagerTokenProgramAccount->data[69], wagerTokenProgramAccount->data[70], wagerTokenProgramAccount->data[71] };
    uint8_t mintAmount[] = { mint->supply[0], mint->supply[1], mint->supply[2],mint->supply[3], mint->supply[4], mint->supply[5], mint->supply[6], mint->supply[7] };
    uint64_t burnAmount64 =	userMintTokenAccount->amount;
    uint64_t potAmount64 = LEbytesto64(potAmount);
    uint64_t mintAmount64 = LEbytesto64(mintAmount);
    unsigned long winning;
    if( contract->outcome > 2 || contract->outcome == 0 ){
		//draw or wager not accepted
		winning = (unsigned long) ( burnAmount64 / mintAmount64 );
	}
	else{
		winning = (unsigned long)( burnAmount64 * potAmount64 / mintAmount64);
	}
	//sol_log_64(burnAmount64,potAmount64,mintAmount64,winning,0);
	uint8_t transferData[] = { 3,0,0,0,0,0,0,0,0 };
	BE64toBytes(1,transferData,(uint64_t) winning);
	const SolInstruction transferInstruction = {
		TokenProgram->key,
		transferArguments, SOL_ARRAY_SIZE(transferArguments),
		transferData, SOL_ARRAY_SIZE(transferData) 
    };    
    if( SUCCESS == sol_invoke_signed(&transferInstruction, params->ka,8,signer_seeds,1) ){ 
		sol_log("transfer tx complete");   
		txComplete = true;             
	}
	else{
		sol_log("transfer error"); 
	}
	return txComplete;
}


bool Mint2User(SolParameters *params) {
	SolAccountInfo *Account = &params->ka[0];	
	SolAccountInfo *Mint = &params->ka[2];
	SolAccountInfo *Authority = &params->ka[5];
	SolAccountInfo *TokenProgram = &params->ka[6];
	SolAccountInfo *Destination = &params->ka[7];					
	Contract *contract = (Contract *)Account->data;	
	bool txComplete = false;
	uint8_t seed1[] = { params->data[1] };
	const SolSignerSeed seeds1[] = { {seed1, SOL_ARRAY_SIZE(seed1)} };	
	const SolSignerSeeds signer_seeds[] = { {seeds1, SOL_ARRAY_SIZE(seeds1)} };
	SolAccountMeta transferArguments[] = {
          {Mint->key, true, false},
          {Destination->key,true,false},
          {Authority->key, false, true},
    };
    uint8_t userBet_[] = { params->data[2], params->data[3], params->data[4],params->data[5], params->data[6], params->data[7], params->data[8], params->data[9]};
    uint64_t userBet = LEbytesto64(userBet_);
    if(userBet < contract->minimumBet){
		return false;
	}
	userBet = ( userBet / contract->minimumBet ) * 1000000;
    uint8_t transferData[] = { 7,0,0,0,0,0,0,0,0 };
    BE64toBytes(1,transferData,userBet); 
	const SolInstruction mintPxInstruction = {
		TokenProgram->key,
		transferArguments, SOL_ARRAY_SIZE(transferArguments),
		transferData,SOL_ARRAY_SIZE(transferData)
    };

	if( SUCCESS == sol_invoke_signed(&mintPxInstruction, params->ka,8,signer_seeds,1) ){ 
		sol_log("mint tx complete");                
		 txComplete = true;
	}
	return txComplete;
}


//Setup Contract Data
//Accounts [ContractAccount,SystemClock,TokenMint1,TokenMint2,PotTokenAccount,OracleAccount,feeAccount]
//Data {uint8_t 0,uint8_t seed,uint64_t minimumBet,uint64_t endtime,uint64_t fee,uin8_t overRideTime}
uint64_t setupContract(SolParameters *params){
	if( !isOwner(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidClock(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidMints(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidPot(params,4,params->data[1]) ){return ERROR_INVALID_ACCOUNT_DATA;}
	if( !ValidEndTime(params) ) {return ERROR_INVALID_ACCOUNT_DATA;}
	if( !CleanAccount(params) ) {return ERROR_INVALID_ACCOUNT_DATA;}
	if( !BuildContract(params) ) {return ERROR_INVALID_ACCOUNT_DATA;}
	return SUCCESS;
}

//Mint
//Accounts [ContractAccount,SystemClock,TokenMint1 || TokenMint2,PotTokenAccount,UserTokenAccount,ProgramAuthority,UserMintTokenAccount,TokenProgram,feeAccount]
//Data {uint8_t 1,uint8_t seed,uint64_t mintAmount}
uint64_t mintTokens(SolParameters *params){
	if( !isOwner(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidClock(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }	
	if( !ValidMint(params,params->data[1]) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidPot(params,3,params->data[1]) ){return ERROR_INVALID_ACCOUNT_DATA;}
	if( !ValidTime(params) ) { return ERROR_INVALID_ACCOUNT_DATA; } 
	if ( !Pending(params) ) { return ERROR_INVALID_ACCOUNT_DATA; } 
	if( !Transfer2Pot(params) ) {return ERROR_INVALID_ACCOUNT_DATA;}
	if( !Mint2User(params) ){return ERROR_INVALID_ACCOUNT_DATA;}
	return SUCCESS;
}

//Set Outcome
//Accounts [ContractAccount,SystemClock,OracleAccount]
//Data {uint8_t 2,uint8_t outcome}
uint64_t setOutcome(SolParameters *params){
	if( !isOwner(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidClock(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }	
	if( !ValidOracle(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( ValidTime(params) && !CanOverRide(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if ( !Pending(params) ) { return ERROR_INVALID_ACCOUNT_DATA; } 	
	if( !FinalizeContract(params) ){return ERROR_INVALID_ACCOUNT_DATA;}
	return SUCCESS;
}

//Redeem
//Accounts [ContractAccount,SystemClock,TokenMint1 || TokenMint2,PotTokenAccount,UserTokenAccount,ProgramAuthority,UserMintTokenAccount,TokenProgram]
//Data {uint8_t 3,uint8_t seed}
uint64_t redeem(SolParameters *params){
	if( !isOwner(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidClock(params) ){ return ERROR_INVALID_ACCOUNT_DATA; }	
	if( !ValidMint(params,params->data[1]) ){ return ERROR_INVALID_ACCOUNT_DATA; }
	if( !ValidPot(params,3,params->data[1]) ){return ERROR_INVALID_ACCOUNT_DATA;}
	if( ValidTime(params) ) { return ERROR_INVALID_ACCOUNT_DATA; } 
	if( !ValidOutcome(params) ) { return ERROR_INVALID_ACCOUNT_DATA; } 
	if( !Transfer2User(params) ){return ERROR_INVALID_ACCOUNT_DATA;}		
	if( !BurnTokens(params) ) {return ERROR_INVALID_ACCOUNT_DATA;}
	return SUCCESS;
}

//TO DO
//Withdraw Timeout Limit?
