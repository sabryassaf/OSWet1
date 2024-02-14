#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_ARGUMENTS 25
using namespace std;
typedef std::vector<std::string> commandInfo;
#define SMASH SmallShell::getInstance()
const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif
// helper functions
bool isInteger(const std::string &str)
{
  std::istringstream iss(str);
  int number;
  char leftover;
  if ((iss >> number) && !(iss >> leftover))
    return true; // Successfully read an int and nothing else
  return false;
}

commandInfo convertToVector(char **CommandLine)
{
  commandInfo result;
  if (CommandLine == nullptr)
  {
    return result; // Return empty vector if the input is nullptr
  }
  for (char **current = CommandLine; *current != nullptr; ++current)
  {
    result.push_back(std::string(*current));
  }
  return result;
}

bool isComplex(const char *Commandline)
{
  string temp(Commandline);
  return ((temp.find('*' != string::npos) || (temp.find('?') != string::npos)));
}

bool _isBackgroundComamnd(const char *cmd_line);

void _removeBackgroundSign(char *cmd_line);

bool isRedirected(const char *Commandline)
{
  string temp(Commandline);
  return ((temp.find('>') != string::npos) || (temp.find('>>') != string::npos) || (temp.find('|') != string::npos) || (temp.find('|&') != string::npos));
}

char *bashArgsPreperation(const std::string &cmd)
{
  if (_isBackgroundComamnd(cmd.c_str()))
  {
    char *c = strdup(cmd.c_str());
    _removeBackgroundSign(c);
    return c;
  }
  else
    return strdup(cmd.c_str());
}

commandInfo splitCommand(const string &command, const string &splitter)
{
  // this will hold the splitted command in a vector where each elemnt will be a string that located between a beginning/splitter to end/splitter
  commandInfo splittedCommandVector;
  int start = 0;
  int end = command.find(splitter);
  // now we have the first command
  while (end != std::string::npos)
  {
    splittedCommandVector.push_back(command.substr(start, end - start));
    // now we update the start and end for the next command
    start = end + splitter.length();
    // find the next splitter
    end = command.find(splitter, start);
  }
  // the last command wont have a splitte after it, special case
  splittedCommandVector.push_back(command.substr(start, end - start));
  return splittedCommandVector;
}

//////// functions from the course
string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

Command::~Command() {}

/// ChpromptCommand
void ChprompotCommand::execute()
{
  SMASH.setPrompt(this->prompt);
}

/// showPidCommand
void ShowPidCommand::execute()
{
  pid_t pid = getpid();
  std::cout << "smash pid is " << pid << endl;
}
/// PwdCommand
void pwdCommand::execute()
{
  char *path = getcwd(NULL, 0);
  cout << path << endl;
  free(path);
}
// ChangeDirectoryCommand
void CdCommand::execute()
{

  SMASH.setLastDirectory(this->newCdd);
}
// JobsClass
void JobsList::addJob(std::string CommandName, int pid)
{
  JobEntry newJob = JobEntry(jobId + 1, pid, CommandName);
  jobs.push_back(newJob);
  jobId++;
}

void JobsList::printJobsList()
{
  removeFinishedJobs();
  auto iter = jobs.begin();
  while (iter != jobs.end())
  {
    std::cout << "[" << iter->getId() << "]"
              << " " << iter->getCommand() << std::endl;
    ++iter;
  }
}

void JobsList::killAllJobs()
{
  auto iter = jobs.begin();
  while (iter != jobs.end())
  {
    jobs.erase(iter);
    return;
    ++iter;
  }
}
int JobsList::getMaxJobId()
{
  int currentMax = 0;
  list<JobEntry> &tempList = jobs;
  auto iter = tempList.begin();
  while (iter != tempList.end())
  {
    if (iter->getId() >= currentMax)
    {
      currentMax = iter->getId();
    }
    ++iter;
  }
  return currentMax;
}
void JobsList::removeFinishedJobs()
{
  list<JobEntry> &tempList = jobs;
  auto iter = tempList.begin();
  while (iter != tempList.end())
  {
    int res = waitpid(iter->getPid(), nullptr, WNOHANG);
    if (res != 0)
    {
      iter = tempList.erase(iter);
    }
    else
    {
      ++iter;
    }
  }
  this->jobId = this->getMaxJobId();
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  auto iter = jobs.begin();
  while (iter != jobs.end())
  {
    if (iter->getId() == jobId)
    {
      return &(*iter);
    }
    ++iter;
  }
}

void JobsList::removeJobById(int jobId)
{
  auto iter = jobs.begin();
  while (iter != jobs.end())
  {
    if (iter->getId() == jobId)
    {
      jobs.erase(iter);
      return;
    }
    ++iter;
  }
}

bool JobsList::isEmpty()
{
  return jobs.empty();
}

JobsList::JobEntry *JobsList::getLastJob()
{
  return &(getJobs().back());
}
//// need to finish getLastStiooedJob
JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {}

list<JobsList::JobEntry> &JobsList::getJobs()
{
  return jobs;
}
// JobEntry
int JobsList::JobEntry::getId() const
{
  return this->id;
}

int JobsList::JobEntry::getPid() const
{
  return this->pid;
}

const string &JobsList::JobEntry::getCommand() const
{
  return jobName;
}

void JobsList::JobEntry::stopJob()
{
  finished = true;
}

void JobsList::JobEntry::continueJob()
{
  finished = false;
}

bool JobsList::JobEntry::isJobFinished() const
{
  return this->finished;
}

// JobsCommand

JobsCommand::JobsCommand(commandInfo &cmdInfoInput) : cmdInfo(cmdInfoInput)
{
}

void JobsCommand::execute()
{
  SMASH.printCurrentJobs();
}
// ForeGround Command

ForegroundCommand::ForegroundCommand(commandInfo &cmdInfo) : jobHolder(-1)
{
  if (cmdInfo.size() == 1)
  {
    if (SMASH.getJobList()->isEmpty())
    {
      SMASH.smashError("fg: jobs list is empty");
      return;
    }
    jobHolder = SMASH.getJobList()->getLastJob()->getId();
    return;
  }
  if (cmdInfo.size() > 2 || !isInteger(cmdInfo[1]))
  {
    SMASH.smashError("fg: invalid arguments");
    return;
  }
  if (!SMASH.getJobList()->getJobById(std::stoi(cmdInfo[1])))
  {
    cout << "smash error: fg: job-id " << cmdInfo[1] << " does not exist";
    return;
  }

  jobHolder = std::stoi(cmdInfo[1]);
}

void ForegroundCommand::execute()
{
  if (jobHolder == -1)
  {
    return;
  }
  int status;
  int pid = SMASH.getJobList()->getJobById(jobHolder)->getPid();
  std::string commandConcatenate = SMASH.getJobList()->getJobById(jobHolder)->getJobName();
  cout << commandConcatenate << " " << pid << endl;
  waitpid(pid, &status, 0);
}

// Quit command
QuitCommand::QuitCommand(commandInfo &cmdInfoInput) : cmdInfo(cmdInfoInput)
{
}
void QuitCommand::execute()
{
  if (cmdInfo.size() == 1)
  {
    exit(0);
  }
  auto temp = SMASH.getJobList()->getJobs();
  if (cmdInfo[1].compare("kill") == 0)
  {
    cout << "smash: sending SIGKILL signal to " << temp.size() << " jobs:" << endl;
    auto iter = temp.begin();
    while (iter != temp.end())
    {
      kill(iter->getPid(), SIGKILL);
    }
    exit(0);
  }
  exit(0);
}

// Kill Command

KillCommand::KillCommand(commandInfo &cmdInfoInput)
{
  this->cmdInfo = cmdInfoInput;
}

void KillCommand::execute()
{
  if (cmdInfo.size() <= 1 || cmdInfo.size() > 3 || !isInteger(cmdInfo[1]) || !isInteger(cmdInfo[2]))
  {
    SMASH.smashError("kill: invalid arguments");
    return;
  }
  JobsList::JobEntry *tmp = SMASH.getJobList()->getJobById(std::stoi(cmdInfo[2]));
  if (!tmp)
  {
    cout << "smash error: kill: job-id " << cmdInfo[2] << " does not exit" << endl;
    return;
  }
  int pid = tmp->getPid();
  int signal = -1 * std::stoi(cmdInfo[2]);
  int rc = kill(pid, signal);
  if (rc != 0)
  {
    perror("smash error: kill failed");
    return;
  }
  cout << "signal number " << cmdInfo[1] << " was sent to pid " << pid << endl;
  if (signal == SIGSTOP)
  {
    tmp->stopJob();
  }
  else if (signal == SIGCONT)
  {
    tmp->continueJob();
  }
}

/// External Command
void ExternalCommand::execute()
{
  int status = 0;
  pid_t pid = fork();
  if (isComplex(this->cmdLine))
  {
    if (pid > 0)
    {
      if (isBackGroundComamnd)
      {
        std::string commandConcatenate = cmdInfo[0] + " " + cmdInfo[1];
        SMASH.getJobList()->addJob(commandConcatenate, pid);
      }
      else
      {
        if (waitpid(pid, &status, WSTOPPED) == -1)
        {
          perror("smash error: waitpid failed");
        }
      }
    }
    else if (pid == 0)
    {
      if (setpgrp() == -1)
      {
        perror("smash error: setgrp() failed");
      }
      char *args[4] = {(char *)"/bin/bash", (char *)"-c", bashArgsPreperation(cmdLine), NULL};
      if (execv(args[0], args) == -1)
      {
        perror("smash error: execv failed");
      }
    }
    else
    {
      perror("smash error: fork failed");
    }
  }
  else
  {
    if (pid > 0)
    {
      if (isBackGroundComamnd)
      {
        std::string commandConcatenate = cmdInfo[0] + " " + cmdInfo[1];
        SMASH.getJobList()->addJob(commandConcatenate, pid);
      }
      else
      {
        if (waitpid(pid, &status, WSTOPPED) == -1)
        {
          perror("smash error: waitpid failed");
        }
      }
    }
    else if (pid == 0)
    {
      if (setpgrp() == -1)
      {
        perror("smash error: setgrp() failed");
      }
      char *args[4] = {(char *)"/bin/bash", (char *)"-c", bashArgsPreperation(cmdLine), NULL};
      if (execvp(args[0], args) == -1)
      {
        perror("smash error: execvp failed");
      }
    }
    else
    {
      perror("smash error: fork failed");
    }
  }
}
RedirectionCommand::RedirectionCommand(commandInfo &cmdInfoInput, const char *cmd_line) : cmdInfo(cmdInfoInput), cmdLine(cmd_line)
{
  // first of all we go over our command vector and check for either < or <<
  int indexOfOperator = 0;
  for (std::string &i : cmdInfo)
  {
    if (i.compare(">") == 0 || i.compare(">>") == 0)
    {
      this->commandOperator = i;
      break;
    }

    indexOfOperator += 1;
  }

  this->targetFile = cmdInfo[indexOfOperator + 1];

  this->splittedCommand = splitCommand(cmd_line, this->commandOperator);
}

void RedirectionCommand::prepare()
{
}
void RedirectionCommand::execute()
{
  if (this->commandOperator.compare(">") == 0)
  {
    int original_stdout_fd = dup(1);

    int temp_fd = open(this->targetFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (temp_fd == -1)
    {
      perror("smash error: open failed");
    }

    if (dup2(temp_fd, 1) == -1)
    {
      perror("smash error: open failed");
    }

    Command *cmd = SMASH.CreateCommand(splittedCommand[0].c_str());
    cmd->execute();
    delete cmd;

    if (dup2(original_stdout_fd, 1) == -1)
    {
      perror("smash error: open failed");
    }

    close(temp_fd);
  }
  else if (this->commandOperator.compare(">>") == 0)
  {
    int original_stdout_fd = dup(1);

    int temp_fd = open(this->targetFile.c_str(), O_CREAT|O_WRONLY|O_APPEND, 0666);
    if (temp_fd == -1)
    {
      perror("smash error: open failed");
    }

    if (dup2(temp_fd, 1) == -1)
    {
      perror("smash error: open failed");
    }

    Command *cmd = SMASH.CreateCommand(splittedCommand[0].c_str());
    cmd->execute();
    delete cmd;

    if (dup2(original_stdout_fd, 1) == -1)
    {
      perror("smash error: open failed");
    }

    close(temp_fd);
  }
}

void RedirectionCommand::fixFileName()
{
  // use provided _removebackground function to make sure we dont have & incase we are working with > or >> operators
  if (this->commandOperator.compare(">") == 0 || this->commandOperator.compare(">>") == 0)
  {
    // remove &
    char *tmp = strdup(this->commandOperator.c_str());
    _removeBackgroundSign(tmp);
    this->targetFile = tmp;
    return;
  }
}

///// smallShell functions
SmallShell::SmallShell() : prompt("smash")
{
  this->currentDirectory = getcwd(NULL, 0);
  this->lastDirectory = "";
  this->shellJobs = new JobsList();
}

void SmallShell::smashError(const std::string &error)
{
  cout << "smash error: " << error << endl;
}

void SmallShell::setLastDirectory(std::string &newCd)
{
  if (newCd.compare("-") == 0)
  {
    if (lastDirectory.empty())
    {
      smashError("OLDPWD not set");
      return;
    }
    if (chdir(lastDirectory.c_str()) == -1)
    {
      perror("smash error: chdir failed");
      return;
    }
    std::string temp = currentDirectory;
    currentDirectory = lastDirectory;
    lastDirectory = temp;
    return;
  }
  if (chdir(newCd.c_str()) == -1)
  {
    perror("smash error: chdir failed");
    return;
  }
  lastDirectory = currentDirectory;
  currentDirectory = newCd;
  return;
}
SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

Command *SmallShell::CreateCommand(const char *cmd_line)
{
  char *CommandLine[MAX_ARGUMENTS];
  for (int i = 0; i < MAX_ARGUMENTS; ++i)
  {
    CommandLine[i] = nullptr;
  }
  bool isRedirectedCommand = isRedirected(cmd_line);
  int words = _parseCommandLine(cmd_line, CommandLine);
  bool isBackgroundCommandInput = _isBackgroundComamnd(cmd_line);
  commandInfo commandVector = convertToVector(CommandLine);

  for (int i = 0; i < words; ++i)
  {
    free(CommandLine[i]);
  }
  if (isRedirectedCommand)
  {
    return new RedirectionCommand(commandVector, cmd_line);
  }
  if (commandVector[0].compare("chprompt") == 0)
  {
    if (commandVector.size() == 1)
    {
      return new ChprompotCommand();
    }
    return new ChprompotCommand(commandVector[1]);
  }
  if (commandVector[0].compare("showpid") == 0)
  {
    return new ShowPidCommand();
  }
  if (commandVector[0].compare("pwd") == 0)
  {
    return new pwdCommand();
  }
  if (commandVector[0].compare("cd") == 0)
  {
    if (commandVector.size() > 2)
    {
      smashError("too many arguments");
    }
    else
    {
      return new CdCommand(commandVector[1]);
    }
  }
  if (commandVector[0].compare("jobs") == 0)
  {
    return new JobsCommand(commandVector);
  }
  if (commandVector[0].compare("fg") == 0)
  {
    return new ForegroundCommand(commandVector);
  }
  if (commandVector[0].compare("quit") == 0)
  {
    return new QuitCommand(commandVector);
  }
  if (commandVector[0].compare("kill") == 0)
  {
    return new KillCommand(commandVector);
  }

  return new ExternalCommand(commandVector, isBackgroundCommandInput, cmd_line);
}

void SmallShell::executeCommand(const char *cmd_line)
{
  shellJobs->removeFinishedJobs();
  Command *cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}