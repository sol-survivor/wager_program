import {
  establishConnection,
  establishPayer,
  loadProgram,
  getConfig
} from './wager_world';

const WagerClient = require('./wager');

async function main() {
	// Establish connection to the cluster
	await establishConnection();

	// Determine who pays for the fees
	await establishPayer();

	// Load the program if not already loaded
	let firstRun = await loadProgram();
	if(firstRun){ 
		console.log("first run waiting...");
		let wait =async()=>{ return new Promise((resolve,reject)=>{setTimeout(resolve,10000)}) }; 
		await wait();
	}
	// Create a wager client
	let config = await getConfig();
	let wc = new WagerClient(config);

	//setup contract
	let [ contractAccount ,mintAccount1,mintAccount2,contractWagerTokenAccount,wagerTokenMint ] = await wc.setupContract();
	let startTime = new Date().getTime()/1000;
	await wc.viewContractData() 

	//mint position 1
	let Amount = 8;
	let [ payerWagerTokenAccount,exists,creationIx ] = await wc.getFeePayerWagerTokenAccount();
	await wc.mintPx(contractAccount.publicKey,mintAccount1.publicKey,payerWagerTokenAccount,contractWagerTokenAccount,wagerTokenMint,Amount);
	
	//close contract
	let waitTime = Math.ceil(wc.endTime - (new Date().getTime()/1000 - startTime) + 1);
	console.log("waiting ",waitTime,"s to close contract");
	await wc.sleep( waitTime  );
	await wc.closeContract(contractAccount.publicKey);
	
	//view contract output
	await wc.viewContractData();
	
	//redeem
	await wc.redeemContract(contractAccount.publicKey,wc.mintAccounts[0].publicKey,payerWagerTokenAccount,contractWagerTokenAccount,wagerTokenMint,Amount);
}

main().then(
  () => process.exit(),
  err => {
    console.error(err);
    process.exit(-1);
  },
);
