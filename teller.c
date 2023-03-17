#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include "branch.h"

/*
 * deposit money into an account
 */
int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  BranchID b = AccountNum_GetBranchID(accountNum);



  pthread_mutex_lock(&account->lock);

pthread_mutex_lock(&bank->branches[b].lock);


  
  Account_Adjust(bank,account, amount, 1);

  pthread_mutex_unlock(&account->lock);

  pthread_mutex_unlock(&bank->branches[b].lock);

  DPRINTF('t', ("END_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);


  if (account == NULL) {


    return ERROR_ACCOUNT_NOT_FOUND;
  }

   BranchID b  = AccountNum_GetBranchID(accountNum);
   
    pthread_mutex_lock(&account->lock);

 pthread_mutex_lock(&bank->branches[b].lock);

  if (amount > Account_Balance(account)) {

    pthread_mutex_unlock(&account->lock);
    pthread_mutex_unlock(&bank->branches[b].lock);

    return ERROR_INSUFFICIENT_FUNDS;
  }
  

  Account_Adjust(bank,account, -amount, 1);

pthread_mutex_unlock(&account->lock);
    pthread_mutex_unlock(&bank->branches[b].lock);
    
    
      DPRINTF('t', ("END_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  assert(amount >= 0);
  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }


  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
 

  if (amount > Account_Balance(srcAccount)) {
 
    return ERROR_INSUFFICIENT_FUNDS;
  }
  if (srcAccountNum>dstAccountNum){
    pthread_mutex_lock(&dstAccount->lock);
    pthread_mutex_lock(&srcAccount->lock);
  }else{
    if(srcAccountNum==dstAccountNum){
        pthread_mutex_lock(&srcAccount->lock);

    }
      else{
    pthread_mutex_lock(&srcAccount->lock);
    pthread_mutex_lock(&dstAccount->lock);
      }
  }




  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);
  BranchID  b1 = AccountNum_GetBranchID(srcAccountNum);
  BranchID  b2 = AccountNum_GetBranchID(dstAccountNum);

  if (updateBranch == 0){
    //  pthread_mutex_lock(&(bank->branches[b1].lock));
  }else{
    if (b1>b2){
      
      pthread_mutex_lock(&(bank->branches[b2].lock));
      pthread_mutex_lock(&(bank->branches[b1].lock));
    }else{
      pthread_mutex_lock(&(bank->branches[b1].lock));
      pthread_mutex_lock(&(bank->branches[b2].lock));
    }
  }


 
 
  Account_Adjust(bank, srcAccount, -amount, updateBranch);
  Account_Adjust(bank, dstAccount, amount, updateBranch);
      if (updateBranch == 0){
     // pthread_mutex_unlock(&(bank->branches[b1].lock));
  }else{

      pthread_mutex_unlock(&(bank->branches[b2].lock));
      pthread_mutex_unlock(&(bank->branches[b1].lock));
  }  
if(srcAccountNum == dstAccountNum){
  pthread_mutex_unlock(&srcAccount->lock);
}else{
  pthread_mutex_unlock(&srcAccount->lock);
  pthread_mutex_unlock(&dstAccount->lock);
}
    DPRINTF('t', ("END_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));

  return ERROR_SUCCESS;
}

