#include "Commands.h"

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
    try
    {
        size_t pos;
        int num = std::stoi(str, &pos);
        // Check if the whole string was converted
        if (pos == str.length())
        {
            return true;
        }
    }
    catch (const std::invalid_argument &e)
    {
    }
    return false;
}

// convert the input commandline into a vector of strings
commandInfo convertToVector(char **CommandLine)
{
    commandInfo result;
    // check for null input inorder not to try and access null pointers
    if (CommandLine == nullptr)
    {
        return result;
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
    // check if anyof the complex signs appear in our command line vector
    return ((temp.find("*") != string::npos) || (temp.find("?") != string::npos));
}

// define those funcion names to use them
bool _isBackgroundComamnd(const char *cmd_line);

void _removeBackgroundSign(char *cmd_line);

bool isRedirected(const char *Commandline)
{
    // check if the command is redirected, i/o pipe
    string temp(Commandline);
    return ((temp.find(">") != string::npos) || (temp.find(">>") != string::npos) || (temp.find("|") != string::npos) ||
            (temp.find("|&") != string::npos));
}

// incase of a background command we need to strip the & before sending it to bash
char *bashArgsPreperation(const std::string &cmd)
{
    // check if it is a background command = has & at end
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
    // push the new job to our job lists with job id = max current job id + 1
    JobEntry newJob = JobEntry(jobId + 1, pid, CommandName);
    jobs.push_back(newJob);
    jobId++;
}

void JobsList::printJobsList()
{
    // iterate over the jobs and print them as requested
    removeFinishedJobs();
    auto iter = jobs.begin();
    while (iter != jobs.end())
    {
        std::cout << "[" << iter->getId() << "]"
                  << " " << iter->getJobName() << endl;
        //           << " " << iter->getCommand() << std::endl;
        // std::cout << commandLineString << std::endl;
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
        // iterate over the current jobs and update the max job id
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
        // check each job status and remove it if its done, res = 0 means the job still running
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
    return nullptr;
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

ForegroundCommand::ForegroundCommand(commandInfo &cmdInfo) : jobHolder(0), validFgCommand(false)
{
    if (cmdInfo.size() > 1)
    {

        if (isInteger(cmdInfo[1]))
        {
            if (!SMASH.getJobList()->getJobById(std::stoi(cmdInfo[1])))
            {
                std::string error = "smash error: fg: job-id " + cmdInfo[1] + " does not exist";
                cerr << error << endl;
                return;
            }
        }
    }
    if (cmdInfo.size() == 1)
    {
        if (SMASH.getJobList()->isEmpty())
        {
            cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        // incase a job id wasnt provided with fg we use the last job inserted
        jobHolder = SMASH.getJobList()->getLastJob()->getId();
        validFgCommand = true;
        return;
    }
    // incase the fg was provided with data, check if data is valid
    if (cmdInfo.size() > 2 || !isInteger(cmdInfo[1]))
    {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    // check if the job id is in our jobs list
    if (!SMASH.getJobList()->getJobById(std::stoi(cmdInfo[1])))
    {
        std::string error = "smash error: fg: job-id " + cmdInfo[1] + " does not exist";
        cerr << error << endl;
        return;
    }
    // if we reach here means the job id provided with the command meets all the requirements so we save it in jobholder as integer
    jobHolder = std::stoi(cmdInfo[1]);
    validFgCommand = true;
}

void ForegroundCommand::execute()
{
    // if we have errors we would return in the constructor before changing the jobHolder from how it was in the initialization list
    if (!this->validFgCommand)
    {
        return;
    }
    int status;
    // get the pid fro the job
    int pid = SMASH.getJobList()->getJobById(jobHolder)->getPid();
    std::string commandConcatenate = SMASH.getJobList()->getJobById(jobHolder)->getJobName();
    // print job name with pid
    cout << commandConcatenate << " " << pid << endl;
    SMASH.setCurrentRunningPid(pid);
    SMASH.setCurrentRunningJob(jobHolder);
    // wait for job to finish
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("smash error: waitpid failed");
    }
    SMASH.setCurrentRunningPid(-1);
    SMASH.setCurrentRunningJob(-1);
}

// Quit command
QuitCommand::QuitCommand(commandInfo &cmdInfoInput) : cmdInfo(cmdInfoInput)
{
}

void QuitCommand::execute()
{
    // quit all
    if (cmdInfo.size() == 1)
    {
        exit(0);
    }
    // if we have more than 1 argument make sure that the second one is kill
    auto temp = SMASH.getJobList()->getJobs();
    if (cmdInfo[1].compare("kill") == 0)
    {
        cout << "smash: sending SIGKILL signal to " << temp.size() << " jobs:" << endl;
        auto iter = temp.begin();
        while (iter != temp.end())
        {
            cout << iter->getPid() << ": " << iter->getCommand() << endl;
            kill(iter->getPid(), SIGKILL);
            iter++;
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
    bool jobIdValid = false;
    if (cmdInfo.size() < 3 || !isInteger(cmdInfo[2]))
    {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }

    for (auto &job : SMASH.getJobList()->getJobs())
    {
        if (job.getId() == std::stoi(cmdInfo[2]))
        {
            jobIdValid = true;
            break;
        }
    }
    if (!jobIdValid)
    {
        std::string tmpStr = "smash error: kill: job-id " + cmdInfo[2] + " does not exist";
        std::cerr << "smash error: kill: job-id " << cmdInfo[2] << " does not exist" << std::endl;
        return;
    }
    JobsList::JobEntry *tmp = SMASH.getJobList()->getJobById(std::stoi(cmdInfo[2]));
    int pid = tmp->getPid();
    if (isInteger(cmdInfo[1]))
    {
        int signal = -1 * std::stoi(cmdInfo[1]);
        if (signal < 0)
        {
            std::cerr << "smash error: kill: invalid arguments" << std::endl;
            return;
        }
        int rc = kill(pid, signal);
        // test if kill failed
        if (rc != 0)
        {
            perror("smash error: kill failed");
            return;
        }
        cout << "signal number " << signal << " was sent to pid " << pid << endl;
        // check which signal was sent to the job and update its status
        if (signal == SIGSTOP)
        {
            tmp->stopJob();
        }
        else if (signal == SIGCONT)
        {
            tmp->continueJob();
        }
    }
    else
    {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }
}

/// External Command
void ExternalCommand::execute()
{
    int status = 0;
    int pid = fork();
    if (isComplex(this->cmdLine))
    {
        if (pid > 0)
        {
            if (isBackGroundComamnd)
            {
                std::string commandConcatenate = cmdInfo[0] + " " + cmdInfo[1];
                SMASH.getJobList()->addJob(cmdLine, pid);
            }
            else
            {
                SMASH.setCurrentRunningPid(pid);
                SMASH.setCurrentRunningJob(-1);
                if (waitpid(pid, &status, WSTOPPED) == -1)
                {
                    perror("smash error: waitpid failed");
                    return;
                }
                SMASH.setCurrentRunningPid(-1);
                SMASH.setCurrentRunningJob(-1);
            }
            return;
        }
        else if (pid == 0)
        {
            if (setpgrp() == -1)
            {
                perror("smash error: setpgrp failed");
                return;
            }
            char *args[4] = {"/bin/bash", "-c", bashArgsPreperation(cmdLine), nullptr};
            if (execv(args[0], args) == -1)
            {
                perror("smash error: execv failed");
                return;
            }
            return;
        }
        else
        {
            perror("smash error: fork failed");
            return;
        }
    }
    else
    {
        if (pid > 0)
        {
            if (isBackGroundComamnd)
            {
                std::string commandConcatenate = cmdInfo[0] + " " + cmdInfo[1];
                SMASH.getJobList()->addJob(this->cmdLine, pid);
            }
            else
            {
                SMASH.setCurrentRunningPid(pid);
                SMASH.setCurrentRunningJob(-1);
                if (waitpid(pid, &status, WSTOPPED) == -1)
                {
                    perror("smash error: waitpid failed");
                    return;
                }
                SMASH.setCurrentRunningPid(-1);
                SMASH.setCurrentRunningJob(-1);
            }
            return;
        }
        else if (pid == 0)
        {
            if (setpgrp() == -1)
            {
                perror("smash error: setpgrp failed");
                return;
            }

            char *args[4] = {"/bin/bash", "-c", bashArgsPreperation(cmdLine), nullptr};
            if (execvp(args[0], args) == -1)
            {
                perror("smash error: execvp failed");
                exit(1);
            }
        }
        else
        {
            perror("smash error: fork failed");
            return;
        }
    }
}

// Redirection Command
RedirectionCommand::RedirectionCommand(commandInfo &cmdInfoInput, const char *cmd_line) : cmdInfo(cmdInfoInput),
                                                                                          cmdLine(cmd_line)
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

        int temp_fd = open(this->targetFile.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
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

// Chmod Command
ChmodCommand::ChmodCommand(commandInfo &commandInfoInput)
{
    // test for invalid arguments
    if (commandInfoInput.size() != 3 || !isInteger(commandInfoInput[1]) || commandInfoInput[1].length() > 4)
    {
        perror("chmod: invalid arguments");
        return;
    }

    // convert from octal to regular base int
    this->mode = std::stoi(commandInfoInput[1], nullptr, 8);
    // save filePath
    this->filePath = commandInfoInput[2];
}

void ChmodCommand::execute()
{
    // check if chmod command works and perform it
    if (chmod(this->filePath.c_str(), this->mode))
    {
        perror("smash error: chmod failed");
    }
}

///// smallShell functions
SmallShell::SmallShell() : prompt("smash")
{
    this->currentDirectory = getcwd(NULL, 0);
    this->lastDirectory = "";
    this->shellJobs = new JobsList();
    this->currentRunningJob = -1;
    this->currentrunningPid = -1;
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
            std::cerr << "smash error: cd: OLDPWD not set" << endl;
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
    if (words == 0)
    {
        return nullptr;
    }
    bool isBackgroundCommandInput = _isBackgroundComamnd(cmd_line);
    commandInfo commandVector = convertToVector(CommandLine);
    std::string cmdAsString = std::string(cmd_line);

    for (int i = 0; i < words; ++i)
    {
        if (CommandLine[i])
            free(CommandLine[i]);
    }
    if (cmdAsString.find("|") != string::npos)
    {
        return new PipeCommand(cmd_line);
        // if (commandVector[0].compare("timeout") == 0) {
        //     return new TimeOutCommand(commandVector, cmd_line);
        // }
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
            std::cerr << "smash error: cd: too many arguments" << endl;
            return nullptr;
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
    if (commandVector[0].compare("chmod") == 0)
    {
        return new ChmodCommand(commandVector);
    }
    this->getJobList()->removeFinishedJobs();
    return new ExternalCommand(commandVector, isBackgroundCommandInput, cmd_line);
}

void SmallShell::executeCommand(const char *cmd_line)
{
    shellJobs->removeFinishedJobs();
    Command *cmd = CreateCommand(cmd_line);
    if (cmd)
    {
        cmd->execute();
    }
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

/// BONUS: TIMEOUT FUNCTION :)

// bool TimedJob::operator()(const TimedJob &rhs, const TimedJob &lhs) {
//     bool diffTime = ((difftime(lhs.deathTime, rhs.deathTime) >= 0));
//     return diffTime;
// }

// ////////// SOMETHING IS WRONG NEED TO BE FIXED.. HELP!!
// std::priority_queue<TimedJob, std::vector<TimedJob>, TimedJob> TimeOutCommand::timeQ;

// TimeOutCommand::TimeOutCommand(commandInfo &cmdInfo, const char *origin) {
//     timeout = atoi(cmdInfo[1].c_str());
//     isBackGround = _isBackgroundComamnd(cmdInfo[cmdInfo.size() - 1].c_str());
//     cmdTInfo = origin;
// }

// void TimeOutCommand::execute()
// {
//     int status = 0;
//     int pid = fork();

//     if (pid == 0)
//     {
//         setpgrp();
//         char* args[] = { (char*) "/bin/bash", (char*) "-c", bashArgsPreperation(cmdTInfo), nullptr };
//         if (execvp(args[0], args) == -1) {
//             perror("smash error: execv failed");
//         }
//     }
//     else if (pid > 0)
//     {
//         SMASH.getJobList()->addJob(cmdTInfo, pid);
//         ///WHY MAROON DIDN'T USE THE PID
//         timeQ.push(TimedJob(pid, timeout));
//         if (timeQ.top().getTimedJobId() == pid) {
//             TimeOutCommand::setAlarm();
//         }
//         if (!(_isBackgroundComamnd(cmdTInfo.c_str()))) {
//             if (waitpid(pid, &status, WSTOPPED) == -1) {
//                 perror("smash error: waitpid failed");
//             }
//         }
//     }
//     else {
//         perror("smash error: fork failed");
//     }
// }

// void TimeOutCommand::setAlarm() {
//     if (!(timeQ.empty())) {
//         alarm(difftime(timeQ.top().getDeathTime(), time(nullptr)));
//     }
// }

// void TimeOutCommand::consumeAlarm() {
//     if (!timeQ.empty()) {
//         int jobId = SMASH.getJobList()->getJobById(timeQ.top().getTimedJobId())->getId();
//         string cmd = SMASH.getJobList()->getJobById(timeQ.top().getTimedJobId())->getCommand();
//         int status = -1;
//         pid_t cpid = waitpid(jobId, &status, WNOHANG | WSTOPPED);

//         if (cpid != 0) {
//             timeQ.pop();
//             SMASH.getJobList()->removeJobById(jobId);
//             setAlarm();
//             return;
//         }
//         if (kill(jobId, SIGKILL) != 0) {
//             perror("smash error: Kill failed");
//         }

//         cout << "smash: " << cmd << " timed out!" << std::endl;
//         timeQ.top();
//         SMASH.getJobList()->removeJobById(jobId);
//         TimeOutCommand::setAlarm();
//     }
// }

void PipeCommand::execute()
{
    bool errorCH = true;
    int tmpPipe[2];
    std::string line = this->cmdLine;
    string firstCmd = line.substr(0, line.find_first_of('|'));
    string secCmd = line.substr(line.find_first_of('|') + 2, std::string::npos);
    if (line.find('&') == string::npos)
    {
        errorCH = false;
    }
    if (pipe(tmpPipe) == -1)
    {
        perror("smash error: pipe failed");
        return;
    }
    int LSon = fork();
    if (LSon == 0)
    {
        if (setpgrp() == -1)
        {
            perror("smash error: setpgrp failed");
            return;
        }
        if (errorCH)
        {
            if (dup2(tmpPipe[1], 2) == -1)
            {
                perror("smash error: dup2 failed");
                return;
            }
        }
        else
        {

            if (dup2(tmpPipe[1], 1) == -1)
            {
                perror("smash error: dup2 failed");
                return;
            }
        }
        close(tmpPipe[0]);
        close(tmpPipe[1]);
        SMASH.getInstance().executeCommand(firstCmd.c_str());
        exit(1);
    }
    int RSon = fork();
    if (RSon == 0)
    {
        if (setpgrp() == -1)
        {
            perror("smash error: setpgrp failed");
            return;
        }

        if (dup2(tmpPipe[0], 0) == -1)
        {
            perror("smash error: dup2 failed");
            return;
        }
        close(tmpPipe[0]);
        close(tmpPipe[1]);
        SMASH.getInstance().executeCommand(secCmd.c_str());
        exit(1);
    }
    close(tmpPipe[0]);
    close(tmpPipe[1]);
    waitpid(LSon, nullptr, 0);
    waitpid(RSon, nullptr, 0);
}
