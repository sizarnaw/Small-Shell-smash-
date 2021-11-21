#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include <unistd.h>
#include <vector>
#include <string>
#include <time.h>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define WHITESPACE  " \t\n\r\f\v"
bool alive = true;
using namespace std;
class Command {
// TODO: Add your data members
protected:
    vector<string> arguments;
public:
    Command(const char* cmd_line);
    virtual ~Command();
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    string print_cmd(){
        string res;
        int size = arguments.size();
        for (int i = 0; i <size ; ++i) {
            res+=arguments[i];
            res+=" ";
        }
        res.pop_back();
        return res;
    }
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand()=default;
    virtual ~BuiltInCommand()=default;
	BuiltInCommand(const char* cmd):Command(cmd){}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char* cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};
//(4)
class ChangeDirCommand : public BuiltInCommand {
    //+
public:
// TODO: Add your data members public:
    ChangeDirCommand();
    virtual ~ChangeDirCommand() {}
    void execute() override;
};
//(3)
class GetCurrDirCommand : public BuiltInCommand {
public:
    //+
    GetCurrDirCommand();
    virtual ~GetCurrDirCommand()=default;
    void execute() override;
};
//(2)
class ShowPidCommand : public BuiltInCommand {
public:
    //+
    ShowPidCommand()=default;
    virtual ~ShowPidCommand()=default;
    void execute() override;
};
//(1)
class ChPromptCommand : public BuiltInCommand{
    //+
private:
    string promptName;
public:
    ChPromptCommand()=default;
    virtual ~ChPromptCommand()=default;
    void execute() override;
};

class JobsList;
//(9)
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    QuitCommand();
    virtual ~QuitCommand() {}
    void execute() override;
};




class JobsList {
public:
    enum status{STOPPED,BACKGROUND};
    class JobEntry {
    public: // TODO: Add your data members
        int jobID;
        Command* cmd;
        pid_t process_ID;
        time_t entryTime;
        status st;

        JobEntry(Command* cmd,status st):cmd(cmd),st(st){
            jobID = 1;
            if(!Jobs.empty()) {
                jobID = Jobs.back().jobID + 1;
            }
            entryTime = time(nullptr);
            process_ID = getpid();
        }
    };
    // TODO: Add your data members
    static vector<JobEntry> Jobs;


public:
    JobsList() = default;
    ~JobsList() = default;
    void addJob(Command* cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    JobEntry* getJobByPID(pid_t pid);
    void removeJobById(int jobId);
    JobEntry * getLastJob(int* lastJobId);
    JobEntry *getLastStoppedJob();
    // TODO: Add extra methods or modify exisitng ones as needed
};
//(5)
class JobsCommand : public BuiltInCommand {
    //+
    // TODO: Add your data members
public:
    JobsCommand() = default;
    virtual ~JobsCommand()= default;
    void execute() override;
};
//(6)
class KillCommand : public BuiltInCommand {
    //+
    // TODO: Add your data members
    int jobId;
    int signal;
public:
    KillCommand():jobId(stoi(arguments[2])),signal(stoi(arguments[1])){}
    virtual ~KillCommand() {}
    void execute() override;
};
//(7)
class ForegroundCommand : public BuiltInCommand {
    //+
    // TODO: Add your data members
    int jobID;
public:
    ForegroundCommand():jobID(arguments.size()>1 ? stoi(arguments[1]) : 0){}
    virtual ~ForegroundCommand() {}
    void execute() override;
};
//(8)
class BackgroundCommand : public BuiltInCommand {
    //+
    // TODO: Add your data members
    int jobID;
public:
    BackgroundCommand():jobID(arguments.size()>1 ? stoi(arguments[1]) : 0){};
    virtual ~BackgroundCommand() {}
    void execute() override;
};

class HeadCommand : public BuiltInCommand {
public:
    HeadCommand(const char* cmd_line);
    virtual ~HeadCommand() {}
    void execute() override;
};


class SmallShell {
private:
    // TODO: Add your data members
    string promptName;
    string* lastPWD;
    int PID;
    JobsList jobs;
    SmallShell();
public:
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    void changePromptName(string name = "smash"){
        promptName = name;
    }
    int GetPID(){
        return PID;
    }
    ~SmallShell();
    const string* getLastPWD(){
        return lastPWD;
    }
    void setLastPWD(string s){
        *lastPWD = s;
    }
    JobsList& getJobs(){
        return jobs;
    }
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
