
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Commands.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for(std::string s; iss >> s; ) {
        args[i] = (char*)malloc(s.length()+1);
        memset(args[i], 0, s.length()+1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h
void ChPromptCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(arguments.size() == 1){
        smash.changePromptName();
    }else if(arguments.size() == 2){
        smash.changePromptName(arguments[1]);
    }else{
        //TODO Error Handling
    }
}
void ShowPidCommand::execute() {
    if(arguments.size() > 1)
        return;
    cout<<"smash pid is "<<SmallShell::getInstance().GetPID()<<endl;
}
void GetCurrDirCommand::execute() {
    if(arguments.size() > 1)
        return;
    cout<<get_current_dir_name()<<endl;
}

void ChangeDirCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(arguments.size() == 2){

        if(arguments[1] == "-"){ //(cd -) command
            if(smash.getLastPWD() == nullptr)
                cout<<"smash error: cd: OLDPWD not set"<<endl;
            int res = chdir((*smash.getLastPWD()).c_str());
            if(res == -1){
                //TODO ERROR HANDLING
                return;
            }
            smash.setLastPWD(get_current_dir_name());
        }else{ // cd (path) command
            int res = chdir(arguments[1].c_str());
            if(res == -1){
                //TODO ERROR HANDLING
                return;
            }
            smash.setLastPWD(get_current_dir_name());
        }
    }else{
        cout<<"smash error: cd: too many arguments"<<endl;
    }
}

//JobList
void JobsList::addJob(Command *cmd, bool isStopped) {

    JobEntry new_job(cmd,isStopped ? BACKGROUND : STOPPED);
    Jobs.push_back(new_job);
}
void JobsList::removeFinishedJobs() {
    int status;

    for (unsigned int i = 0; i <Jobs.size() ; ++i) {
        pid_t return_pid = waitpid(Jobs[i].process_ID,&status,WNOHANG);
        if(return_pid == -1){
            //TODO : ERROR
        } else if(return_pid == Jobs[i].process_ID){
            Jobs.erase(Jobs.begin()+i,Jobs.begin()+i+1);
            i--;
        }
    }
}
void JobsList::printJobsList() {
    removeFinishedJobs();
    for(JobEntry j : Jobs){
        cout<<"["<<j.jobID<<"] "<<j.cmd->print_cmd()<<" : "<<j.process_ID<<" "<<difftime(time(nullptr),j.entryTime);
        if(j.st == STOPPED)
            cout<<" (stopped)"<<endl;
        else
            cout<<endl;
    }
}
JobsList::JobEntry * JobsList::getJobById(int jobId) {
    for (unsigned int i = 0; i < Jobs.size(); ++i) {
        if(Jobs[i].jobID == jobId)
            return &Jobs[i];
    }
    return nullptr;
}
JobsList::JobEntry* JobsList::getJobByPID(pid_t pid) {
    for (unsigned int i = 0; i < Jobs.size(); ++i) {
        if(Jobs[i].process_ID == pid)
            return &Jobs[i];
    }
    return nullptr;
}
void JobsCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.getJobs().printJobsList();
}
void JobsList::removeJobById(int jobId) {
    for (unsigned int i = 0; i < Jobs.size(); ++i) {
        if(Jobs[i].jobID == jobId){
            Jobs.erase(Jobs.begin()+i,Jobs.begin()+i+1);
            break;
        }
    }

}
JobsList::JobEntry * JobsList::getLastStoppedJob() {
    for (unsigned int i = Jobs.size()-1; i>=0; ++i) {
        if (Jobs[i].st == STOPPED)
            return &Jobs[i];
    }
    return nullptr;
}
void KillCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(stoi(arguments[1]) > 31 || stoi(arguments[1])< 1 || arguments.size() != 2){
        cout<<"smash error: kill: invalid arguments"<<endl;
    }
    JobsList::JobEntry* job = smash.getJobs().getJobById(jobId);
    if(job){
        //TODO error handling in KILL
        kill(job->process_ID,signal);
    } else {
        cout<<"smash error: kill: job-id "<<jobId<<" does not exist"<<endl;
    }
}

void ForegroundCommand::execute() {

    SmallShell& smash = SmallShell::getInstance();
    smash.getJobs().removeFinishedJobs();
    JobsList::JobEntry* job = smash.getJobs().getJobById(jobID);
    if(!job){
        cout<<"smash error: fg: job-id "<<job->jobID<<" does not exist"<<endl;
        return;
    }
    if(arguments.size() >2){
        cout<<"smash error: fg: invalid arguments"<<endl;
        return;
    }else if (arguments.size() == 2){
        kill(job->process_ID,SIGCONT);
        int status;
        waitpid(job->process_ID,&status,WCONTINUED);
        cout<<job->cmd->print_cmd()<<" : "<<job->process_ID<<endl;
    }else if(arguments.size() == 1){
        if(smash.getJobs().Jobs.empty()){
            cout<<"smash error: fg: jobs list is empty"<<endl;
            return;
        }
        *job=smash.getJobs().Jobs.back();
        kill(job->process_ID,SIGCONT);
        int status;
        waitpid(job->process_ID,&status,WCONTINUED);
        cout<<job->cmd->print_cmd()<<" : "<<job->process_ID<<endl;
    }
    smash.getJobs().removeJobById(job->jobID);
}

void BackgroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.getJobs().removeFinishedJobs();
    JobsList::JobEntry* job = smash.getJobs().getJobById(jobID);
    if(!job){
        cout<<"smash error: fg: job-id "<<job->jobID<<" does not exist"<<endl;
        return;
    }
    if(arguments.size()>2){
        cout<<"smash error: bg: invalid arguments"<<endl;
        return;
    }
    if(arguments.size()==2){
        if(job->st == JobsList::BACKGROUND){
            cout<<"smash error: bg: job-id "<<job->jobID<<" is already running in the background"<<endl;
            return;
        }
        
        kill(job->process_ID,SIGCONT);
        cout<<job->cmd->print_cmd()<<" : "<<job->process_ID<<endl;
    }else if(arguments.size() == 1){
        job = smash.getJobs().getLastStoppedJob();
        if(!job){
            cout<<"smash error: bg: there is no stopped job to resume"<<endl;
            return;
        }
        kill(job->process_ID,SIGCONT);
        cout<<job->cmd->print_cmd()<<" : "<<job->process_ID<<endl;
    }
    job->st = JobsList::BACKGROUND;
}
void QuitCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    JobsList jobs = smash.getJobs();
    if(arguments.size() >=2){
        if(arguments[1] == "kill"){
            for (unsigned int i = 0; i <jobs.Jobs.size() ; ++i) {
                cout<<jobs.Jobs[i].process_ID<<": "<<jobs.Jobs[i].cmd->print_cmd()<<endl;
                kill(jobs.Jobs[i].process_ID,SIGKILL);
                alive = false;
            }
        }else{
            //TODO:: ERROR HANDLING
        }
    }else if(arguments.size() == 1){
        alive = false;
    }

}
SmallShell::SmallShell(): jobs() {
// TODO: add your implementation
    promptName = "smash";
    lastPWD = nullptr;
    PID = getpid();

}

SmallShell::~SmallShell() {
// TODO: add your implementation
    delete lastPWD;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command::Command(const char *cmd_line) {
    string s = string(cmd_line);
    string temp;
    for (unsigned int i = 0; i < s.size(); ++i) {
        if (s[i] != ' ') {
            temp.push_back(s[i]);
        } else {
            arguments.push_back(temp);
            temp.clear();
            while (i < s.size() && s[i] == ' ') {
                i++;
            }
        }
    }
}


Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:
    //cmd_s command without spaces
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));


    if (firstWord.compare("chprompt") == 0) {
        return new ChPromptCommand();
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand();
    } else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand();
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(); //TODO: saweha
    }else if(firstWord.compare("jobs") == 0){
        return new JobsCommand();
    }else if(firstWord.compare("kill") == 0){
        return new KillCommand();
    }else if(firstWord.compare("fg") == 0){
        return new ForegroundCommand();
    }else if(firstWord.compare("bg") == 0){
        return new BackgroundCommand();
    }else if(firstWord.compare("quit") == 0){
		return new QuitCommand();
	}
    else {
        return new ExternalCommand(cmd_line);
    }

    return nullptr;
}
void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);

    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}
//TODO : CHECK IF ARGUMENTS IS VALID (ASCII IS NUMBER).