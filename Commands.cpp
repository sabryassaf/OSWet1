#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#define MAX_ARGUMENTS 20
using namespace std;
typedef std::vector<std::string> commandInfo;
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

SmallShell::SmallShell() : prompt("smash")
{
  this->currentDirectory = getcwd(NULL, 0);
  this->lastDirectory = "";
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
    chdir(lastDirectory.c_str());
    lastDirectory = currentDirectory;
    return;
  }
  lastDirectory = currentDirectory;
  chdir(newCd.c_str());
  currentDirectory = newCd;
  return;
}
SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

Command::~Command() {}

void ChprompotCommand::execute()
{
  SmallShell::getInstance().setPrompt(this->prompt);
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

void ShowPidCommand::execute()
{
  pid_t pid = getpid();
  std::cout << "smash pid is " << pid << endl;
}

void pwdCommand::execute()
{
  char *path = getcwd(NULL, 0);
  cout << path << endl;
  free(path);
}

void CdCommand::execute()
{
  
  SmallShell::getInstance().setLastDirectory(this->newCdd);
}
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  char *CommandLine[MAX_ARGUMENTS];
  for (int i = 0; i < MAX_ARGUMENTS; ++i)
  {
    CommandLine[i] = nullptr;
  }
  int words = _parseCommandLine(cmd_line, CommandLine);
  commandInfo commandVector = convertToVector(CommandLine);
  for (int i = 0; i < words; ++i)
  {
    free(CommandLine[i]);
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
    return new CdCommand(commandVector[1]);
  }

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{

  Command *cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}