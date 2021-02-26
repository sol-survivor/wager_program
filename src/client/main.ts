import {
  establishConnection,
  establishPayer,
  loadProgram,
  getConfig,
  getPlayers
} from './wager_world';

const WagerClient = require('./wager');

async function p1Win(){
	let pass = false;
	let balanceBeforeMint = 0;
	let balanceAfterMint = 0;
	let balanceAfterRedemption = 0;
	// Create a wager client
	let config = await getConfig(false);
	let wc = new WagerClient(config);

	//setup contract
	let [ contractAccount ,mintAccount1,mintAccount2,contractWagerTokenAccount,wagerTokenMint ] = await wc.setupContract();
	let startTime = new Date().getTime()/1000;
	await wc.viewContractData() 

	//mint position 1
	let Amount = 8;
	let [ payerWagerTokenAccount,exists,creationIx ] = await wc.getFeePayerWagerTokenAccount();
	balanceBeforeMint = await wc.getBalance(payerWagerTokenAccount);
	await wc.mintPx(1,Amount);
	balanceAfterMint = await wc.getBalance(payerWagerTokenAccount);
	
	//close contract
	let waitTime = Math.ceil(wc.endTime - (new Date().getTime()/1000 - startTime));
	console.log("waiting ",waitTime,"s to close contract");
	await wc.sleep( waitTime  );
	await wc.closeContract(1);
	
	//view contract output
	await wc.viewContractData();
	
	//redeem
	console.log('redeeming');
	await wc.redeemContract(1);
	await wc.sleep(2);
	balanceAfterRedemption = await wc.getBalance(payerWagerTokenAccount);

	//result
	if(balanceBeforeMint === balanceAfterRedemption && balanceAfterMint === (balanceBeforeMint - (Amount * Math.pow(10,6))) ){
		pass = true;
	}
	console.log("P1 Balance History:",balanceBeforeMint,"-->",balanceAfterMint,"-->",balanceAfterRedemption);
	return pass;
}

async function p2Win(){
	let pass = false;
	let balanceBeforeMint = 0;
	let balanceAfterMint = 0;
	let balanceAfterRedemption = 0;
	// Create a wager client
	let config = await getConfig(false);
	let wc = new WagerClient(config);

	//setup contract
	let [ contractAccount ,mintAccount1,mintAccount2,contractWagerTokenAccount,wagerTokenMint ] = await wc.setupContract();
	let startTime = new Date().getTime()/1000;
	await wc.viewContractData() 

	//mint position 1
	let Amount = 8;
	let [ payerWagerTokenAccount,exists,creationIx ] = await wc.getFeePayerWagerTokenAccount();
	balanceBeforeMint = await wc.getBalance(payerWagerTokenAccount);
	await wc.mintPx(1,Amount);
	balanceAfterMint = await wc.getBalance(payerWagerTokenAccount);
	
	//close contract
	let waitTime = Math.ceil(wc.endTime - (new Date().getTime()/1000 - startTime));
	console.log("waiting ",waitTime,"s to close contract");
	await wc.sleep( waitTime  );
	await wc.closeContract(2);
	
	//view contract output
	await wc.viewContractData();
	
	//redeem
	try { await wc.redeemContract(1); }
	catch(e){
		console.log("invalid redemption attempt");
	}	
	balanceAfterRedemption = await wc.getBalance(payerWagerTokenAccount);

	//result
	if(balanceBeforeMint > balanceAfterRedemption && balanceAfterMint === (balanceBeforeMint - (Amount * Math.pow(10,6))) ){
		pass = true;
	}
	console.log("P1 Balance History:",balanceBeforeMint,"-->",balanceAfterMint,"-->",balanceAfterRedemption);
	return pass;
}

async function drawRedemption(){
	let pass = false;
	let balanceBeforeMint = 0;
	let balanceAfterMint = 0;
	let balanceAfterRedemption = 0;
	// Create a wager client
	let config = await getConfig(false);
	let wc = new WagerClient(config);

	//setup contract
	let [ contractAccount ,mintAccount1,mintAccount2,contractWagerTokenAccount,wagerTokenMint ] = await wc.setupContract();
	let startTime = new Date().getTime()/1000;
	await wc.viewContractData() 

	//mint position 1
	let Amount = 8;
	let [ payerWagerTokenAccount,exists,creationIx ] = await wc.getFeePayerWagerTokenAccount();
	balanceBeforeMint = await wc.getBalance(payerWagerTokenAccount);
	await wc.mintPx(1,Amount);
	balanceAfterMint = await wc.getBalance(payerWagerTokenAccount);
	
	//close contract
	let waitTime = Math.ceil(wc.endTime - (new Date().getTime()/1000 - startTime));
	console.log("waiting ",waitTime,"s to close contract");
	await wc.sleep( waitTime  );
	await wc.closeContract(3);
	
	//view contract output
	await wc.viewContractData();
	
	//redeem
	await wc.redeemContract(1);
	await wc.sleep(2);
	balanceAfterRedemption = await wc.getBalance(payerWagerTokenAccount);

	//result
	if(balanceBeforeMint === balanceAfterRedemption && balanceAfterMint === (balanceBeforeMint - (Amount * Math.pow(10,6))) ){
		pass = true;
	}
	console.log("P1 Balance History:",balanceBeforeMint,"-->",balanceAfterMint,"-->",balanceAfterRedemption);
	return pass;
}

async function timeoutRedemption(){
	let pass = false;
	let balanceBeforeMint = 0;
	let balanceAfterMint = 0;
	let balanceAfterRedemption = 0;
	// Create a wager client
	let config = await getConfig(false);
	let wc = new WagerClient(config);

	//setup contract
	let [ contractAccount ,mintAccount1,mintAccount2,contractWagerTokenAccount,wagerTokenMint ] = await wc.setupContract();
	let startTime = new Date().getTime()/1000;
	await wc.viewContractData() 

	//mint position 1
	let Amount = 8;
	let [ payerWagerTokenAccount,exists,creationIx ] = await wc.getFeePayerWagerTokenAccount();
	balanceBeforeMint = await wc.getBalance(payerWagerTokenAccount);
	await wc.mintPx(1,Amount);
	balanceAfterMint = await wc.getBalance(payerWagerTokenAccount);
	
	//close contract
	let waitTime = Math.ceil(wc.endTime - (new Date().getTime()/1000 - startTime));
	console.log("waiting ",waitTime,"s for contract timeout");
	await wc.sleep( waitTime  );
	
	//view contract output
	await wc.viewContractData();
	
	//redeem
	await wc.redeemContract(1);
	await wc.sleep(2);
	balanceAfterRedemption = await wc.getBalance(payerWagerTokenAccount);

	//result
	if(balanceBeforeMint === balanceAfterRedemption && balanceAfterMint === (balanceBeforeMint - (Amount * Math.pow(10,6))) ){
		pass = true;
	}
	console.log("P1 Balance History:",balanceBeforeMint,"-->",balanceAfterMint,"-->",balanceAfterRedemption);
	return pass;
}

async function p1WinMultiParty(){
	let pass = false;
	let balanceBeforeMint = 0;
	let balanceAfterMint = 0;
	let balanceAfterRedemption = 0;
	// Create a wager client
	let config = await getConfig(true);
	config.endTime = 15;
	let wc = new WagerClient(config);

	//setup contract
	let [ contractAccount ,mintAccount1,mintAccount2,contractWagerTokenAccount,wagerTokenMint ] = await wc.setupContract();
	let startTime = new Date().getTime()/1000;
	await wc.viewContractData() 

	//mint position 1
	let Amount = 50;
	let [ payerWagerTokenAccount,exists,creationIx ] = await wc.getFeePayerWagerTokenAccount();
	balanceBeforeMint = await wc.getBalance(payerWagerTokenAccount);
	await wc.mintPx(1,Amount);
	balanceAfterMint = await wc.getBalance(payerWagerTokenAccount);
	
	//mint other positions
	let temp = wc.feePayer
	let players : any;
	let pwta;
	players = getPlayers();
	let side1 = [9,60,20];
	let balancesAfter = [];
	let balancesBefore = [];	
	for(let i = 0;i < side1.length;i++){
		wc.feePayer = players[i];
		[ pwta ] = await wc.getFeePayerWagerTokenAccount();
		balancesBefore.push( await wc.getBalance(pwta) );
		await wc.mintPx(1,side1[i]);
	}
	let side2 = [33,22,5,1];
	for(let i = 0;i < side2.length;i++){
		wc.feePayer = players[i+3];
		await wc.mintPx(2,side2[i]);
	}
	wc.feePayer = temp
	//
	
	//close contract
	let waitTime = Math.ceil(wc.endTime - (new Date().getTime()/1000 - startTime));
	console.log("waiting ",waitTime,"s to close contract");
	await wc.sleep( waitTime  );
	await wc.closeContract(1);
	
	//view contract output
	await wc.viewContractData();
	
	//redeem
	console.log('redeeming');
	await wc.redeemContract(1);
	await wc.sleep(2);
	balanceAfterRedemption = await wc.getBalance(payerWagerTokenAccount);
	for(let i = 0;i < side1.length;i++){
		wc.feePayer = players[i];
		[ pwta ] = await wc.getFeePayerWagerTokenAccount();
		await wc.redeemContract(1);
		balancesAfter.push( await wc.getBalance(pwta) );
	}
	//result
	if(balanceAfterRedemption === 71942446 && balanceAfterMint === 0){ pass = true;}
	if( (balancesAfter[0] - balancesBefore[0]) !== 3949640 ){ pass = false; }
	if( (balancesAfter[1] - balancesBefore[1]) !== 26330935 ){ pass = false; }
	if( (balancesAfter[2] - balancesBefore[2]) !== 8776979 ){ pass = false; }
	console.log("P1 Balance History:",balanceBeforeMint,"-->",balanceAfterMint,"-->",balanceAfterRedemption);
	console.log("Other Balance History:",balancesBefore,"-->",(balancesAfter));
	return pass;
}


async function main() {
	let testFunctions = [p1Win,p2Win,drawRedemption,timeoutRedemption,p1WinMultiParty]
	let passed = 0;
	// Establish connection to the cluster
	await establishConnection();

	// Determine who pays for the fees
	await establishPayer();

	// Load the program if not already loaded
	await loadProgram();

	// Run Tests
	for (let i = 0;i < testFunctions.length;i++){
		if(await testFunctions[i]()){
			passed++;
		}
	}
	console.log("Passed ",passed,"/",testFunctions.length," Tests");
}

main().then(
  () => process.exit(),
  err => {
    console.error(err);
    process.exit(-1);
  },
);
