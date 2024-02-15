#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;
#define SMASH SmallShell::getInstance()

void ctrlCHandler(int sig_num)
{

  if (SMASH.getCurrentRunningPid() == -1)
  {
    return;
  }
  cout << "smash:got ctrl-C" << endl;
  if (kill(SMASH.getCurrentRunningPid(), SIGKILL) == -1)
  {
    perror("smash error: kill failed");
  }
  else
  {
    std::cout << "smash: process " << SmallShell::getInstance().getCurrentRunningPid() << " was killed" << std::endl;
  }
  SMASH.setCurrentRunningPid(-1);
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
