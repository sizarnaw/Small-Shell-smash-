#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
#include <unistd.h>
#include <vector>
#include <string>
#include <time.h>
#include <string.h>
#include "assert.h"
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define WHITESPACE  " \t\n\r\f\v"

using namespace std;
class Command {
// TODO: Add your data members
protected:

    vector<string> arguments;
public:
    enum type{BG,FG};
    Command(const char* cmd_line);
    virtual ~Command()=default;
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

    BuiltInCommand(const char* cmd_line): Command(cmd_line){

    }
     ~BuiltInCommand()=default;
};

class ExternalCommand : public Command {
public:
    const char* cmd;
    bool BG;
    char**  arr_arg;
    ExternalCommand(const char* cmd_line,bool isBG): Command(cmd_line),cmd(cmd_line),BG(isBG){
        arr_arg = (char**)malloc(arguments.size());
        for (unsigned int i = 0; i < arguments.size(); ++i) {
            arr_arg[i] = (char*)malloc(arguments[i].size());
            strcpy(arr_arg[i],arguments[i].c_str());
        }
    }
     ~ExternalCommand()=default;
    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
    enum op {PIPE, PIPEERROR};
    op operation;
    const char* cmd_line;
public:
    PipeCommand(const char* cmd_line, int op) : Command(cmd_line), cmd_line(cmd_line){
        operation = op == 3 ? PIPE : PIPEERROR;
    }
    virtual ~PipeCommand() {}
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
    enum op{APPEND,OVERRIDE};
    op operation;
    const char* cmd_line;
public:
     RedirectionCommand(const char* cmd_line, int op): Command(cmd_line),cmd_line(cmd_line){
         operation = op == 1 ? OVERRIDE : APPEND;
    }
     ~RedirectionCommand()= default;
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};
//(4)
class ChangeDirCommand : public BuiltInCommand {
    //+
public:
// TODO: Add your data members public:
    ChangeDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
     ~ChangeDirCommand()=default;
    void execute() override;
};
//(3)
class GetCurrDirCommand : public BuiltInCommand {
public:
    //+
    GetCurrDirCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
     ~GetCurrDirCommand()=default;
    void execute() override;
};
//(2)
class ShowPidCommand : public BuiltInCommand {
public:
    //+
    ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
     ~ShowPidCommand()=default;
    void execute() override;
};
//(1)
class ChPromptCommand : public BuiltInCommand{
    //+
private:

public:
    ChPromptCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
     ~ChPromptCommand()=default;
    void execute() override;
};

class JobsList;
//(9)
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    QuitCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
     ~QuitCommand()=default;
    void execute() override;
};
enum status{STOPPED,BACKGROUND};
class JobEntry {
public: // TODO: Add your data members
    int jobID;
    Command* cmd;
    pid_t process_ID;
    time_t entryTime;


    status st;

    JobEntry(Command* cmd,pid_t pid,status sta, int jobID):jobID(jobID), cmd(cmd), process_ID(pid){
        entryTime = time(nullptr);
        st = sta;
        /*jobID = 1;
        if(!Jobs.empty()) {
            jobID = Jobs.back().jobID + 1;
        }
        entryTime = time(nullptr);
        process_ID = getpid();*/
    }
};


class JobsList {
public:

    // TODO: Add your data members
     vector<JobEntry> Jobs;


public:
    JobsList() = default;
    ~JobsList() = default;
    void addJob(Command* cmd,pid_t pid ,status isStopped=STOPPED);
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
    JobsCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
     ~JobsCommand()= default;
    void execute() override;
};
//(6)
class KillCommand : public BuiltInCommand {
    //+
    // TODO: Add your data members
    int jobId;
    int signal;
public:
    KillCommand(const char* cmd_line): BuiltInCommand(cmd_line),jobId(stoi(arguments[2])),signal(stoi(arguments[1].substr(1))){}
     ~KillCommand()=default;
    void execute() override;
};
//(7)
class ForegroundCommand : public BuiltInCommand {
    //+
    // TODO: Add your data members
    int jobID;
public:
    ForegroundCommand(const char* cmd_line): BuiltInCommand(cmd_line),jobID(arguments.size()>1 ? stoi(arguments[1]) : 0){}
     ~ForegroundCommand()=default;
    void execute() override;
};
//(8)
class BackgroundCommand : public BuiltInCommand {
    //+
    // TODO: Add your data members
    int jobID;
public:
    BackgroundCommand(const char* cmd_line): BuiltInCommand(cmd_line),jobID(arguments.size()>1 ? stoi(arguments[1]) : 0){};
     ~BackgroundCommand()=default;
    void execute() override;
};

class HeadCommand : public BuiltInCommand {
public:
    int size = 10;
    string file_name;
    HeadCommand(const char* cmd_line): BuiltInCommand(cmd_line){

        if(arguments.size()> 2) {
            assert(arguments[1][0] == '-');
            arguments[1].erase(arguments[1].begin());
            size =  stoi(arguments[1]);
        }
        file_name = arguments.size() > 2 ? arguments[2] : arguments[1];
    }
     ~HeadCommand()=default;
    void execute() override;
};


class SmallShell {
private:
    // TODO: Add your data members


    string* lastPWD;
    int PID;
    JobsList jobs;
    SmallShell();
public:
    Command* currCmd = nullptr;
    pid_t currForegroundPID;
    string promptName = "smash";
    bool alive = true;
    Command *CreateCommand(const char* cmd_line);
    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    void changePromptName(string name){

       promptName.clear();
       int i=0;
       while(name[i]){
           promptName.push_back(name[i]);
           i++;
       }

    }
    int GetPID(){
        return PID;
    }
    ~SmallShell();
    const string* getLastPWD(){
        return lastPWD;
    }
    void setLastPWD(string s){
        if(lastPWD == nullptr){
            lastPWD = new string(s);
        }else{
            lastPWD->clear();
            for (char i : s) {
                lastPWD->push_back(i);
            }
        }
    }
    JobsList& getJobs(){
        return jobs;
    }
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
