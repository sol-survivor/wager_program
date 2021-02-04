#include <criterion/criterion.h>
#include "wager.c"


uint64_t sol_create_program_address(){return 0;}
uint64_t sol_invoke_signed_c(){ return 0;}

  
Test(wager, sanity) {
  uint8_t instruction_data[] = {};
  SolPubkey program_id = {.x = {
                              1,
                          }};
  SolPubkey key = {.x = {
                       2,
                   }};
  uint64_t lamports = 1;
  uint8_t data[] = {0, 0, 0, 0};


  SolAccountInfo accounts[] = {{
      &key,
      &lamports,
      sizeof(data),
      data,
      &program_id,
      0,
      true,
      true,
      false,
  }};
  SolParameters params = {accounts, sizeof(accounts), instruction_data, sizeof(instruction_data), &program_id};
  //TODO
  

}
