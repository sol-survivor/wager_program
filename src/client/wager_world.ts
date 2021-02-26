/* eslint-disable @typescript-eslint/no-unsafe-assignment */
/* eslint-disable @typescript-eslint/no-unsafe-member-access */
/* eslint-disable @typescript-eslint/no-unsafe-call */
/* eslint-disable @typescript-eslint/ban-ts-comment */

import {
  Account,
  Connection,
  BpfLoader,
  BPF_LOADER_PROGRAM_ID,
  PublicKey,
  LAMPORTS_PER_SOL,
  SystemProgram,
  TransactionInstruction,
  Transaction,
  sendAndConfirmTransaction,
} from '@solana/web3.js';
import fs from 'mz/fs';
import {
  Token,
  TOKEN_PROGRAM_ID
} from "@solana/spl-token";
// @ts-ignore
import BufferLayout from 'buffer-layout';

import {url, urlTls} from './util/url';
import {Store} from './util/store';
import {newAccountWithLamports} from './util/new-account-with-lamports';
const { Numberu64 } = require("@solana/spl-token-swap");

/**
 * Connection to the network
 */
let connection: Connection;

/**
 * Connection to the network
 */
let payerAccount: Account;
let players: Account[];
/**
 * Wager program id
 */
let programId: PublicKey;

/**
 * The public key of the account
 */
let greetedPubkey: PublicKey;

const pathToProgram = 'dist/program/wager.so';

/**
 * Layout of the dummy account data
 */
const dummyAccountDataLayout = BufferLayout.struct([BufferLayout.u32(''),]);

/**
 * Establish a connection to the cluster
 */
export async function establishConnection(): Promise<void> {
  connection = new Connection(url, 'singleGossip');
  const version = await connection.getVersion();
  console.log('Connection to cluster established:', url, version);
}

/**
 * Establish an account to pay for everything
 */
export async function establishPayer(): Promise<void> {
  if (!payerAccount) {
    let fees = 0;
    const {feeCalculator} = await connection.getRecentBlockhash();

    // Calculate the cost to load the program
    const data = await fs.readFile(pathToProgram);
    const NUM_RETRIES = 500; // allow some number of retries
    fees +=
      feeCalculator.lamportsPerSignature *
        (BpfLoader.getMinNumSignatures(data.length) + NUM_RETRIES) +
      (await connection.getMinimumBalanceForRentExemption(data.length));

    // Calculate the cost to fund the greeter account
    fees += await connection.getMinimumBalanceForRentExemption(
      dummyAccountDataLayout.span,
    );

    // Calculate the cost of sending the transactions
    fees += feeCalculator.lamportsPerSignature * 9900000; // wag

    // Fund a new payer via airdrop
    payerAccount = await newAccountWithLamports(connection, fees);
    players = [];
    for(let i = 0;i < 7;i++){
		players.push(await newAccountWithLamports(connection, fees));
	}
  }

  const lamports = await connection.getBalance(payerAccount.publicKey);
  console.log('Using account',payerAccount.publicKey.toBase58(),'containing',
  lamports / LAMPORTS_PER_SOL,'Sol to pay for fees',);
}

/**
 * Load the wager BPF program if not already loaded
 */
export async function loadProgram(): Promise<boolean> {
  const store = new Store();

  // Check if the program has already been loaded
  try {
    const config = await store.load('config.json');
    programId = new PublicKey(config.programId);
    await connection.getAccountInfo(programId);
    console.log('Program already loaded to account', programId.toBase58());
    return false;
  } catch (err) {
	  console.log(pathToProgram,err);
    // try to load the program
  }

  // Load the program
  console.log('Loading wager program...');
  const data = await fs.readFile(pathToProgram);
  const programAccount = new Account();
  let ldx = await BpfLoader.load(
    connection,
    payerAccount,
    programAccount,
    data,
    BPF_LOADER_PROGRAM_ID,
  );
  programId = programAccount.publicKey;
  console.log(ldx,'WAGER Program loaded to ', programId.toBase58());
  // Save this info for next time
  await store.save('config.json', {
    url: urlTls,
    programId: programId.toBase58(),
  });
  return true;
}


const SPL_ASSOCIATED_TOKEN_ACCOUNT_PROGRAM_ID = new PublicKey("ATokenGPvbdGVxr1b2hvZbsiqW5xWH25efTNsLJA8knL");

const SYSVAR_RENT_PUBKEY = new PublicKey("SysvarRent111111111111111111111111111111111");

const findAssociatedTokenAccountPublicKey = async (
  ownerPublicKey: PublicKey,
  tokenMintPublicKey: PublicKey
) =>
  (
    await PublicKey.findProgramAddress(
      [
        ownerPublicKey.toBuffer(),
        TOKEN_PROGRAM_ID.toBuffer(),
        tokenMintPublicKey.toBuffer()
      ],
      SPL_ASSOCIATED_TOKEN_ACCOUNT_PROGRAM_ID
    )
  )[0];
  
const createIx = (
  funderPubkey: PublicKey,
  associatedTokenAccountPublicKey: PublicKey,
  ownerPublicKey: PublicKey,
  tokenMintPublicKey: PublicKey
) =>
  new TransactionInstruction({
    programId: SPL_ASSOCIATED_TOKEN_ACCOUNT_PROGRAM_ID,
    data: Buffer.from([]),
    keys: [
      { pubkey: funderPubkey, isSigner: true, isWritable: true },
      {
        pubkey: associatedTokenAccountPublicKey,
        isSigner: false,
        isWritable: true
      },
      { pubkey: ownerPublicKey, isSigner: false, isWritable: false },
      { pubkey: tokenMintPublicKey, isSigner: false, isWritable: false },
      { pubkey: SystemProgram.programId, isSigner: false, isWritable: false },
      { pubkey: TOKEN_PROGRAM_ID, isSigner: false, isWritable: false },
      { pubkey: SYSVAR_RENT_PUBKEY, isSigner: false, isWritable: false }
    ]
  });


async function getProgramAuthority(buf:any):Promise<[any,any]>{
	let seed = buf!== false ? buf : Buffer.from("1");
	let addr;
	try{
		addr = await PublicKey.createProgramAddress(
			[seed],
			programId
		);	
	}
	catch(e){
		console.warn("bumping seed..........");
		seed[0]++;
		return getProgramAuthority(seed);
	}
	return [addr,seed];
}

//Seperate Wager Token
export async function createWagerMint(multiparty:boolean): Promise<[PublicKey,PublicKey]> {
	let mintAccount = new Account();
	let dummyTokenAccount  = new PublicKey("TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA");
	let payerWagerTokenAccount = new PublicKey("TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA");
	let tokenProgram = new PublicKey("TokenkegQfeZyiNwAJbNbGKPFXCWuBvf9Ss623VQ5DA");
	let [contractSigner,seed] = await getProgramAuthority(false);
	console.log("Wager Mint:",mintAccount.publicKey.toBase58(),"WagerAuthority:",payerAccount.publicKey.toBase58());	
	let balance = await connection.getBalance(payerAccount.publicKey);
	console.log(balance);
	let decimals = 6;		
	try{
		//Create Mint
		  let createAccIx = SystemProgram.createAccount({
			  fromPubkey: payerAccount.publicKey,
			  newAccountPubkey: mintAccount.publicKey,
			  lamports: await connection.getMinimumBalanceForRentExemption(82,"singleGossip"),
			  space: 82,
			  programId: tokenProgram
		});

		let initMintIx = Token.createInitMintInstruction(
			tokenProgram,
			mintAccount.publicKey,
			decimals,
			payerAccount.publicKey,
			null
		);	
		let ix = new Transaction().add(createAccIx).add(initMintIx);
		let tx = await sendAndConfirmTransaction(
			connection,
			ix,
			[payerAccount,mintAccount],
			{
			  commitment: 'singleGossip',
			  preflightCommitment: 'singleGossip',
			},
		  );	  
	  console.log("Mint initialized:",tx);
	  //Create associated program address for payer
	  payerWagerTokenAccount = await findAssociatedTokenAccountPublicKey(payerAccount.publicKey,mintAccount.publicKey);
	  const assocIx = createIx(
		  payerAccount.publicKey,
		  payerWagerTokenAccount,
		  payerAccount.publicKey,
		  mintAccount.publicKey
		);
		let tx2 = await sendAndConfirmTransaction(
			connection,
			new Transaction().add(assocIx),
			[payerAccount],
			{
			  commitment: 'singleGossip',
			  preflightCommitment: 'singleGossip',
			},
		  );
		 console.log("Wager Payer Token Account:",payerWagerTokenAccount.toBase58());
		 ////Mint tokens to player account
		 let amount = 50;
		 let amount64 = new Numberu64(amount * Math.pow(10,6)).toBuffer();
		 let doubleAmount64 = new Numberu64(amount*2 * Math.pow(10,6)).toBuffer();		 
		 let minttoix = new TransactionInstruction({
			keys: [
				{pubkey:mintAccount.publicKey, isSigner: false, isWritable:true},
				{pubkey:payerWagerTokenAccount, isSigner:false, isWritable:true},			
				{pubkey:payerAccount.publicKey, isSigner:true, isWritable:false},
			],
			programId:tokenProgram,
			data: Buffer.concat ([Buffer.from([7]), amount64,seed ])
		});
		//additional players
		let transaction = new Transaction();
		let pwta;
		let createPWTA;
		if(multiparty){
			for (let i = 0;i < players.length;i++){
				pwta = await findAssociatedTokenAccountPublicKey(players[i].publicKey,mintAccount.publicKey);
				createPWTA = createIx(
				  players[i].publicKey,
				  pwta,
				  players[i].publicKey,
				  mintAccount.publicKey
				);
				await sendAndConfirmTransaction(
					connection,
					new Transaction().add(createPWTA),
					[players[i]],
					{
					  commitment: 'singleGossip',
					  preflightCommitment: 'singleGossip',
					},
				  );
				transaction.add(new TransactionInstruction({
						keys: [
						{pubkey:mintAccount.publicKey, isSigner: false, isWritable:true},
						{pubkey:pwta, isSigner:false, isWritable:true},			
						{pubkey:payerAccount.publicKey, isSigner:true, isWritable:false},
					],
					programId:tokenProgram,
					data: Buffer.concat ([Buffer.from([7]), doubleAmount64,seed ])
				}));
			}
		}
		///////////
		transaction.add(minttoix);
		let wagerMint = await sendAndConfirmTransaction(
			connection,
			transaction,
			[payerAccount],
			{
			  commitment: 'singleGossip',
			  preflightCommitment: 'singleGossip',
			},
		  );
		 console.log("Wager Mint complete",wagerMint);
		  return [payerWagerTokenAccount,mintAccount.publicKey]
	}
	catch(e){
		console.error(e);
	}
	return [payerWagerTokenAccount,mintAccount.publicKey]
}

export async function getConfig(multiparty:boolean){
	let [ pywt,potMint] = await createWagerMint(multiparty);
	let contractLifeTimeInSeconds = 5;
	return {
		connection,
		endTime: contractLifeTimeInSeconds,
		fee:0,
		feeAccount:pywt,
		feePayer:payerAccount,
		minimumBet:1,
		oracleAccount:payerAccount,
		override:0,
		potMint,
		programId,
	}
}

export function getPlayers():Account[]{
	return players
}
