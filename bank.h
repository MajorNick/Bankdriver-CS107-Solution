#ifndef _BANK_H
#define _BANK_H



typedef struct Bank {
  unsigned int numberBranches;
  struct       Branch  *branches;
  struct       Report  *report;
  int finishedWorkers;
  int numWorkers;
  pthread_mutex_t waitForWorkers;
  pthread_cond_t stopWorkers;
  pthread_mutex_t waitForReporter; 
} Bank;

#include "account.h"

int Bank_Balance(Bank *bank, AccountAmount *balance);

Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);



#endif /* _BANK_H */
