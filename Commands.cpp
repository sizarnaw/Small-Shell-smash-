
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Commands.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fstream>
#include <fcntl.h>


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
        smash.changePromptName("smash");
    }else if(arguments.size() >= 2){
        smash.changePromptName(arguments[1]);
    }
}
void ShowPidCommand::execute() {

    cout<<"smash pid is "<<SmallShell::getInstance().GetPID()<<endl;
}
void GetCurrDirCommand::execute() {

    cout<<get_current_dir_name()<<endl;
}

void ChangeDirCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    string prev = get_current_dir_name();
    if(arguments.size() <= 2){

        if(arguments[1] == "-"){ //(cd -) command
            if(smash.getLastPWD() == nullptr) {
                cout << "smash error: cd: OLDPWD not set" << endl;
                return;
            }
            int res = chdir((*smash.getLastPWD()).c_str());
            if(res == -1){
                //TODO ERROR HANDLING
                return;
            }
            smash.setLastPWD(prev);
        }else{ // cd (path) command

            int res = chdir(arguments[1].c_str());
            if(res == -1){
                cout << "smash error: chdir failed: No such file or directory" << endl;
                return;
            }
            smash.setLastPWD(prev);
        }
    }else{
        cout<<"smash error: cd: too many arguments"<<endl;
    }
}
//JobList
void JobsList::addJob(Command *cmd,pid_t pid, status isStopped) {
    int jobID = Jobs.empty() ? 1 : Jobs.back().jobID + 1;

    JobEntry new_job(cmd,pid,isStopped, jobID);
    Jobs.push_back(new_job);
}
void JobsList::removeFinishedJobs() {
    int status;
    for (unsigned int i = 0; i <Jobs.size() ; ++i) {
        pid_t return_pid = waitpid(Jobs[i].process_ID,&status,WNOHANG );

        if(return_pid == -1){
            cout << "errorr here" <<endl;
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
JobEntry * JobsList::getJobById(int jobId) {
    for (unsigned int i = 0; i < Jobs.size(); ++i) {
        if(Jobs[i].jobID == jobId)
            return &Jobs[i];
    }
    return nullptr;
}
JobEntry* JobsList::getJobByPID(pid_t pid) {
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
JobEntry * JobsList::getLastStoppedJob() {
    int i = Jobs.size()-1;
        while(i >= 0){
        cout<<i<<endl;
        if (Jobs[i].st == STOPPED)
            return &Jobs[i];
        i--;
    }
    return nullptr;
}
void KillCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    if(signal > 31 || signal < 1 || arguments.size() != 3){
        perror("kill: invalid arguments");
    }
    JobEntry* job = smash.getJobs().getJobById(jobId);
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
    JobEntry* job = smash.getJobs().getJobById(jobID);

    if(!job && arguments.size() > 1){
        cout<<"smash error: fg: job-id "<< jobID <<" does not exist"<<endl;
        return;
    }

    if(arguments.size() >2){
        cout<<"smash error: fg: invalid arguments"<<endl;
        return;
    }else if (arguments.size() == 2){
        cout<<job->cmd->print_cmd()<<" : "<<job->process_ID<<endl;
        int status;
        int currPid = job->process_ID;
        kill(job->process_ID,SIGCONT);
        smash.currForegroundPID = job->process_ID;
        smash.currCmd = job->cmd;
        smash.getJobs().removeJobById(job->jobID);

        waitpid(currPid ,&status,WUNTRACED);
    }else if(arguments.size() == 1){

        if(smash.getJobs().Jobs.empty()){
            cout<<"smash error: fg: jobs list is empty"<<endl;
            return;
        }

        job = smash.getJobs().getLastStoppedJob();
        if(!job){
            job = smash.getJobs().getLastJob();
        }


        cout<<job->cmd->print_cmd()<<" : "<<job->process_ID<<endl;

        kill(job->process_ID,SIGCONT);

        int status;
        int currPid = job->process_ID;
        smash.currForegroundPID = job->process_ID;
        smash.currCmd = job->cmd;

        smash.getJobs().removeJobById(job->jobID);

        waitpid(currPid ,&status,WUNTRACED);

    }
}

void BackgroundCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.getJobs().removeFinishedJobs();
    JobEntry* job = smash.getJobs().getJobById(jobID);
    if(!job){
        cout<<"smash error: fg: job-id "<<jobID<<" does not exist"<<endl;
        return;
    }
    if(arguments.size()>2){
        cout<<"smash error: bg: invalid arguments"<<endl;
        return;
    }
    if(arguments.size()==2){
        if(job->st == BACKGROUND){
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
    job->st = BACKGROUND;
}

void HeadCommand::execute() {
    if(arguments.size() < 2){
        //TODO: ERROR HAndling
    }
    if(arguments.size() >3)
        return;
    ifstream file;
    file.open(file_name);
    if(!file.good()){
        //TODO: Error handling
    }
    string out;
    for(int i =0; i <size; i ++){
        if(file.eof()) {
            break;
        }
        getline(file,out);
        cout<< out<< endl;
    }
    file.close();
}
void QuitCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    JobsList jobs = smash.getJobs();
    if(arguments.size() >=2){
        if(arguments[1] == "kill"){
            for (unsigned int i = 0; i <jobs.Jobs.size() ; ++i) {
                cout<<jobs.Jobs[i].process_ID<<": "<<jobs.Jobs[i].cmd->print_cmd()<<endl;
                kill(jobs.Jobs[i].process_ID,SIGKILL);
                smash.alive = false;
            }
        }else{

            //TODO:: ERROR HANDLING
        }
    }else if(arguments.size() == 1){
        smash.alive = false;
    }

}
char* getNewCMD(const char* cmd){
    int i =0;
    while(cmd[i] && cmd[i] != '&') {
        i++;
    }
    char* new_cmd = new char[i - 1];
    strcpy(new_cmd,cmd);
    _removeBackgroundSign(new_cmd);
    return new_cmd;
}

void ExternalCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    int pid = fork();
    if(pid>0){
        if(BG) {
            smash.getJobs().addJob(this,pid, BACKGROUND);
        }else{
            int status;
            smash.currForegroundPID = pid;
            waitpid(pid,&status,WUNTRACED);
        }

    }
    else if(pid == 0){
        setpgrp();
        char path[10];
        strcpy(path,"/bin/bash");
        char flag[3];
        strcpy(flag,"-c");
        char* new_cmd = getNewCMD(cmd);
        char*  argv[4] = {path,flag,new_cmd, nullptr};

        execv(path,argv);
        delete new_cmd;


    }

}
const char* converttochar(vector<string> args) {
    string cmd;
    int index = 0;
    while(args[index] != ">" && args[index] != ">>") {
        cmd += args[index];
        cmd += " ";
        index++;
    }

    return cmd.c_str();
}

void PipeCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    string firstCommand, secondCommand;

    unsigned int i = 0;
    for(; i < arguments.size(); i++){
        if(arguments[i] == "|" || arguments[i] == "|&")
            break;
        firstCommand += arguments[i];
        firstCommand += " ";
    }
    i++;
    for(; i < arguments.size(); i++){
        secondCommand += arguments[i];
        secondCommand += " ";
    }

    int myPipe[2];
    pipe(myPipe);
    int stdInCopy = dup(0);
    int stdOutCopy = dup(1);
    int stdErrCopy = dup(2);

    pid_t pid = fork();
    if(pid == 0){
        //child
        setpgrp();
        close(myPipe[0]); // closing the writing channel
        close(1);
        dup2(myPipe[1],operation == PIPE ? 1 : 2);
        smash.executeCommand(firstCommand.c_str());
        exit(1);
    } else {
        //parent
        close(myPipe[1]); //closing the reading channel
        close(0);
        dup2(myPipe[0], 0);
        smash.executeCommand(secondCommand.c_str());
    }

    close(myPipe[0]);
    close(myPipe[1]);

    dup2(stdInCopy, 0);
    dup2(stdOutCopy, 1);
    dup2(stdErrCopy, 2);
}


void RedirectionCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    int fd;
    if(operation == OVERRIDE) {
        fd = open(arguments.back().c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
    }else{
        fd = open(arguments.back().c_str(), O_CREAT | O_RDWR |O_APPEND, 0644);
    }
    if(fd == -1) {
        cout << "ERROR" << endl; //TODO: ERROR HANGLING
        return;
    }

    int stdoutcopy = dup(1);
    close(1);
    dup2(fd,1);
    string cmd;
    int index = 0;
    while(arguments[index] != ">" && arguments[index] != ">>") {
        cmd += arguments[index];
        cmd += " ";
        index++;
    }
    smash.executeCommand(cmd.c_str());

    close(fd);
    dup2(stdoutcopy,1);
}
SmallShell::SmallShell(): jobs() {
// TODO: add your implementation
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
        if (s[i] != ' ' ) {
            temp.push_back(s[i]);
        } else {
            arguments.push_back(temp);
            temp.clear();
            while (i < s.size() && s[i] == ' ') {
                i++;
            }
            i--;
        }
    }
    if(!temp.empty())
        arguments.push_back(temp);
}
/*
 * 1 = >
 * 2 = >>
 * 3 = |
 * 4 = |&
 */
int checkSpecial(const char* cmd_line){
    int index = 0;
    while(cmd_line[index]){
        if(cmd_line[index] == '>'){
            if(cmd_line[index+1] == '>') {
                return 2;
            }else{
                return 1;
            }
        }
        if(cmd_line[index] == '|'){
            if(cmd_line[index + 1] == '&')
                return 4;
            return 3;
        }
        index++;
    }
    return 0;
}
Command * SmallShell::CreateCommand(const char* cmd_line) {
    // For example:
    //cmd_s command without spaces
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    int special = checkSpecial( cmd_line);
    if(special == 1 || special == 2){
        return new RedirectionCommand(cmd_line,special);
    } else if(special == 3 || special == 4) {
        return new PipeCommand(cmd_line, special);
    }

    if (firstWord.compare("chprompt") == 0) {
        return new ChPromptCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line); //TODO: saweha
    }else if(firstWord.compare("jobs") == 0){
        return new JobsCommand(cmd_line);
    }else if(firstWord.compare("kill") == 0){
        return new KillCommand(cmd_line);
    }else if(firstWord.compare("fg") == 0){
        return new ForegroundCommand(cmd_line);
    }else if(firstWord.compare("bg") == 0){
        return new BackgroundCommand(cmd_line);
    }else if(firstWord.compare("quit") == 0){
		return new QuitCommand(cmd_line);
	}else if(firstWord.compare("head") == 0){
        return new HeadCommand(cmd_line);
    }
    else  {

        return new ExternalCommand(cmd_line,_isBackgroundComamnd(cmd_line));
    }

}
void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here

     Command* cmd = CreateCommand(cmd_line);
     
     if(cmd) {
        currCmd = cmd;
         cmd->execute();
     }
     else
         currCmd = nullptr;
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}
//TODO : CHECK IF ARGUMENTS IS VALID (ASCII IS NUMBER).
