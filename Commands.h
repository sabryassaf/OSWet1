#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <list>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iomanip>
#include <queue>
#include <iostream>
#include <sstream>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using std::list;
using std::string;
typedef std::vector<std::string> commandInfo;

class Command
{
protected:
  string commandName;

public:
  Command(){};
  Command(const char *cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  virtual string getCommandName() const { return this->commandName; }
  virtual int getPidOfCommand() const
  {
    return getpid();
  }
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(){};
  BuiltInCommand(const char *cmd_line);
  virtual ~BuiltInCommand(){};
};

class ChprompotCommand : public BuiltInCommand
{
private:
  std::string prompt;

public:
  ChprompotCommand(std::string &promptNew)
  {
    this->prompt = promptNew;
    this->commandName = "Chprompt";
  }
  ChprompotCommand()
  {
    this->prompt = "smash";
    this->commandName = "Chprompt";
  }
  virtual ~ChprompotCommand() = default;
  void execute() override;
};
class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand() { this->commandName = "pid"; };
  virtual ~ShowPidCommand() = default;
  void execute() override;
};

class pwdCommand : public BuiltInCommand
{
public:
  pwdCommand(){

  };
  virtual ~pwdCommand() = default;
  void execute() override;
};
class CdCommand : public BuiltInCommand
{
private:
  std::string newCdd;

public:
  CdCommand(const std::string &newCd)
  {
    this->newCdd = newCd;
    this->commandName = "cd";
  };
  void execute() override;
  virtual ~CdCommand() = default;
};

class ForegroundCommand : public BuiltInCommand
{
private:
  int jobHolder;
  bool validFgCommand;

public:
  ForegroundCommand(commandInfo &cmdInfo);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand
{
private:
  commandInfo cmdInfo;

public:
  JobsCommand(commandInfo &cmdInfo);
  virtual ~JobsCommand() = default;
  void execute() override;
};

class QuitCommand : public BuiltInCommand
{
private:
  commandInfo cmdInfo;

public:
  QuitCommand(commandInfo &cmdinfo);
  virtual ~QuitCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
private:
  commandInfo cmdInfo;

public:
  KillCommand(commandInfo &cmdInfo);
  virtual ~KillCommand() {}
  void execute() override;
};

class ExternalCommand : public Command
{
private:
  commandInfo cmdInfo;
  bool isBackGroundComamnd;
  const char *cmdLine;

public:
  ExternalCommand(commandInfo &cmdInfoInput, bool isBackGround, const char *cmdLineInput) : cmdInfo(cmdInfoInput), isBackGroundComamnd(isBackGround), cmdLine(cmdLineInput){};
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  // TODO: Add your data members
public:
  PipeCommand(const char *cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  commandInfo cmdInfo;
  commandInfo splittedCommand;
  const char *cmdLine;
  std::string commandOperator;
  std::string targetFile;
  std::string Lcommand;

public:
  explicit RedirectionCommand(commandInfo &cmdInfoInput, const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  void fixFileName();
  void prepare();
  // void cleanup() override;
};

class JobEntry;

class JobsList;

class JobsList
{
public:
  class JobEntry
  {
  private:
    int id;
    // proccess id
    int pid;
    std::string jobName;
    bool finished;

  public:
    JobEntry(int id, int pid, const string &name) : id(id), pid(pid), jobName(name), finished(false) {}
    int getId() const;
    int getPid() const;
    const string &getCommand() const;
    std::string getJobName() { return this->jobName; };
    void stopJob();
    void continueJob();
    bool isJobFinished() const;
  };

private:
  int jobId;
  // jobId is the maximum job number;
  list<JobEntry> jobs;

public:
  JobsList() : jobId(0), jobs(list<JobEntry>()) {}
  ~JobsList() = default;
  void addJob(std::string commandName, int pid);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  bool isEmpty();
  int getMaxJobId();
  JobEntry *getLastJob();
  list<JobEntry> &getJobs();
};

class ChmodCommand : public BuiltInCommand
{
private:
  commandInfo cmdInfo;
  int mode;
  std::string filePath;

public:
  ChmodCommand(commandInfo &cmdInfo);
  virtual ~ChmodCommand() = default;
  void execute() override;
};

/// BONUS: TIMEDOUT CLASS
class TimedJob {
    int TJId;
    int timeout;
    time_t deathTime;
public:
    TimedJob(int jobIDInput, int timeoutInput) : TJId(jobIDInput), timeout(timeoutInput),
                                                        deathTime(time(nullptr)+timeoutInput){};
    bool operator()(const TimedJob& lhs, const TimedJob& rhs);
    int getTimedJobId() const {return TJId;}
    time_t getDeathTime() const {return deathTime;}
};

class TimeOutCommand : public Command {
    static std::priority_queue<TimedJob, std::vector<TimedJob>, TimedJob> timeQ;
    string cmdTInfo;
    int timeout;
    bool isBackGround;
public:
    TimeOutCommand() {}
    TimeOutCommand(commandInfo& cmdInfo, const char* origin);
    virtual ~TimeOutCommand() {}
    void execute() override;
    static void setAlarm();
    static void consumeAlarm();

};

class SmallShell
{
private:
  // TODO: Add your data members
  std::string prompt;
  std::string lastDirectory;
  std::string currentDirectory;
  JobsList *shellJobs;
  SmallShell();
  int currentrunningPid;
  int currentRunningJob;

public:
  Command *CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  void smashError(const std::string &error);
  JobsList *getJobList() { return shellJobs; }

  void setPrompt(std::string &newName) { this->prompt = newName; };
  std::string &getPrompt() { return this->prompt; };

  void setLastDirectory(std::string &newCd);

  void setCurrentRunningPid(int pid) { this->currentrunningPid = pid; }
  int getCurrentRunningPid() const { return this->currentrunningPid; }

  void setCurrentRunningJob(int jobid){this->currentRunningJob = jobid;};
  int getCurrentRunningJob() const { return this->currentRunningJob; }

  void printCurrentJobs()
  {
    this->shellJobs->printJobsList();
  }

  ~SmallShell();
  void executeCommand(const char *cmd_line);
};

#endif // SMASH_COMMAND_H_