#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command
{
  // TODO: Add your data members
public:
  Command(){};
  Command(const char *cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
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
  }
  ChprompotCommand()
  {
    this->prompt = "smash";
  }
  virtual ~ChprompotCommand() = default;
  void execute() override;
};
class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand()
  {
  };
  virtual ~ShowPidCommand() = default;
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
  };
  void execute() override;
  virtual ~CdCommand() = default;
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

class JobsList;
class QuitCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
  QuitCommand(const char *cmd_line, JobsList *jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsList
{
public:
  class JobEntry
  {
    // TODO: Add your data members
  };
  // TODO: Add your data members
public:
  JobsList();
  ~JobsList();
  void addJob(Command *cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  JobsCommand(const char *cmd_line, JobsList *jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  ForegroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand
{
public:
  ChmodCommand(const char *cmd_line);
  virtual ~ChmodCommand() = default;
  void execute() override;
};

class pwdCommand : public BuiltInCommand
{
public:
  pwdCommand()
  {

  };
  virtual ~pwdCommand() = default;
  void execute() override;
};

class SmallShell
{
private:
  // TODO: Add your data members
  std::string prompt;
  std::string lastDirectory;
  std::string currentDirectory;
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

  void setPrompt(std::string &newName) { this->prompt = newName; };
  std::string &getPrompt() { return this->prompt; };

  void setLastDirectory(std::string &newCd);

  ~SmallShell();
  void executeCommand(const char *cmd_line);
  // TODO: add extra methods as needed
};

#endif // SMASH_COMMAND_H_
