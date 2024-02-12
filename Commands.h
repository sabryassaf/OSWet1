#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <list>
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
  virtual int getPidOfCommand() const { return getpid(); }
  // virtual void prepare();
  // virtual void cleanup();
  //  TODO: Add your extra methods if needed
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
public:
  ExternalCommand(const char *cmd_line);
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
  // TODO: Add your data members
public:
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  // void prepare() override;
  // void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
  ChangeDirCommand(const char *cmd_line, char **plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
public:
  GetCurrDirCommand(const char *cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
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
    int getId() const
    {
      return this->id;
    }
    int getPid() const
    {
      return this->pid;
    }
    string &getCommand()
    {
      return jobName;
    }
    void stopJob()
    {
      finished = true;
    }
    void contJob()
    {
      finished = false;
    }
    bool isJobFinished()
    {
      return this->finished;
    }
  };

private:
  int jobId;
  // jobId is the maximum job number;
  list<JobEntry> jobs;

public:
  JobsList() : jobId(0), jobs(list<JobEntry>()) {}
  ~JobsList() = default;
  void addJob(Command *cmd, int pid)
  {
    JobEntry newJob = JobEntry(jobId + 1, pid, cmd->getCommandName());
    jobs.push_back(newJob);
    jobId++;
  }
  void printJobsList()
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
  void killAllJobs()
  {
    auto iter = jobs.begin();
    while (iter != jobs.end())
    {
      jobs.erase(iter);
      return;
      ++iter;
    }
  }
  void removeFinishedJobs()
  {
    auto iter = jobs.begin();
    while (iter != jobs.end())
    {
      if (iter->isJobFinished())
      {
        jobs.erase(iter);
      }
      ++iter;
    }
  }
  JobEntry *getJobById(int jobId)
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
  void removeJobById(int jobId)
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
  bool isEmpty()
  {
    return jobs.empty();
  }
  JobEntry *getLastJob()
  {
    return &(getJobs().back());
  }
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
  list<JobEntry> &getJobs()
  {
    return jobs;
  }
};



class ChmodCommand : public BuiltInCommand
{
public:
  ChmodCommand(const char *cmd_line);
  virtual ~ChmodCommand() = default;
  void execute() override;
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

  void printCurrentJobs()
  {
    this->shellJobs->printJobsList();
  }

  ~SmallShell();
  void executeCommand(const char *cmd_line);
  // TODO: add extra methods as needed
};

#endif // SMASH_COMMAND_H_