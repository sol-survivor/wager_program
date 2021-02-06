***************
THIS SOFTWARE IS PROVIDED AS IS

THIS SOFTWARE HAS NOT BEEN AUDITED

THIS SOFTWARE DOES NOT COME WITH A WARRANTY 
***************

**
If using this in production with large sums at risk ensure you DO NOT RESUSE PROGRAMS, but instead deploy new programs as needed.
**


# Wager Program

This project demonstrates how to use the [wager program](https://github.com/sol-survivor/wager_program/blob/main/src/program-c/src/wager/wager.c) on the Solana blockchain.

The project comprises of:

* An on-chain program
* A client that can setup /mint positions /close /redeem winnings


## Quick Start

The following dependencies are required to build and run this example, depending on your OS, they may already be installed:

- Install node
- Install npm
- Install docker
- Install the latest Rust stable from https://rustup.rs/
- Install Solana v1.4.7 or later from https://docs.solana.com/cli/install-solana-cli-tools

If this is your first time using Docker or Rust, these [Installation Notes](README-installation-notes.md) might be helpful.

### Start local Solana cluster

This example connects to a local Solana cluster by default.

Enable on-chain program logs:
```bash
$ export RUST_LOG=solana_runtime::system_instruction_processor=trace,solana_runtime::message_processor=debug,solana_bpf_loader=debug,solana_rbpf=debug
```

Start a local Solana cluster:
```bash
$ npm run localnet:update
$ npm run localnet:up
```

View the cluster logs:
```bash
$ npm run localnet:logs
```

Note: To stop the local Solana cluster later:
```bash
$ npm run localnet:down
```

### Build the on-chain program

There is only a C version of the on-chain program.

```bash
$ npm run build:program-c
```

### Run the client

```bash
$ npm run start
```

#### Not seeing the expected output?

- Ensure you've [started the local cluster](#start-local-solana-cluster) and [built the on-chain program](#build-the-on-chain-program).
- Ensure Docker is running.  You might try bumping up its resource settings, 8 GB of memory and 3 GB of swap should help.
- The client output should include program log messages that indicate why the program failed.
  - `program log: <message>`
- Inspect the Solana cluster logs looking for any failed transactions or failed on-chain programs
  - Expand the log filter and restart the cluster to see more detail
    - ```bash
      $ npm run localnet:down
      $ export RUST_LOG=solana_runtime::native_loader=trace,solana_runtime::system_instruction_processor=trace,solana_runtime::bank=debug,solana_bpf_loader=debug,solana_rbpf=debug
      $ npm run localnet:up

### Customizing the Wager Program

To customize the wager, make changes to the files under `/src`.  If you change any files under `/src/program-c` you will need to rebuild the on-chain program

Now when you re-run `npm run start`, you should see the results of your changes.

## Learn about the client

The [client](https://github.com/sol-survivor/wager_program/blob/main/src/client/wager.js) is written in JavaScript using:
- [Solana web3.js SDK](https://github.com/solana-labs/solana-web3.js)
- [Solana web3 API](https://solana-labs.github.io/solana-web3.js)


### Using the Wager Client

```
let WagerClient = require('./wager');
let contractLifeTimeInSeconds = 5;
let config = {
	connection,
	endTime: contractLifeTimeInSeconds,
	feeAccount:pywt, //Token Account of the Token Mint Address for the fee payer
	feePayer:payerAccount,
	minimumBet:1,
	oracleAccount:payerAccount,
	override:0,
	potMint, // SRM,FTT,etc Mint Address
	programId,
}
let wc = new WagerClient(config);



```

#### Setting up a new contract
```
let [ contractAccount ,mintAccount1,mintAccount2,contractWagerTokenAccount,wagerTokenMint ] = await wc.setupContract();

```

#### Minting a position

```
let Amount = 8;
await wc.mintPx(contractAccount.publicKey,mintAccount1.publicKey,payerWagerTokenAccount,contractWagerTokenAccount,wagerTokenMint,Amount);

```

#### Setting the outcome of a contract

```
await wc.closeContract(oracleAccount.publicKey);
```

#### Redeeming tokens from the Pot when the winner is from position 1
```
await wc.redeemContract(1);

```

#### View Contract Data
```
await wc.viewContractData();

```

## TO DO

- Normalize variable names
- Stream line wager client function calls
- Add tests 
- Improve readme



