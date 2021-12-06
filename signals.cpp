#include <iostream>
#include <signal.h>
#include "signals.h"
#include "unistd.h"
#include "Commands.h"


using namespace std;

void ctrlZHandler(int sig_num) {
    // TODO: Add your implementation
    SmallShell& smash = SmallShell::getInstance();

    pid_t pid = smash.currForegroundPID;

    cout << "smash: got ctrl-Z" << endl;
    if(pid == 0)
        return;

    int res = kill(pid,SIGSTOP);
    if(res == 0) {

        smash.getJobs().addJob(smash.currCmd, pid, STOPPED);
        cout << "smash: process " << pid << " was stopped" << endl;
    } else {

        perror("smash error: kill failed");
    }

}

void ctrlCHandler(int sig_num) {
    SmallShell& smash = SmallShell::getInstance();
    pid_t pid = smash.currForegroundPID;
    cout << "smash: got ctrl-C" << endl;
    if(pid == 0)
        return;
    int res = kill(pid, SIGKILL);
    if(res == 0){
        cout << "smash: process "<< pid <<" was killed" << endl;
    }
}

void alarmHandler(int sig_num,siginfo_t* info,void* context) {
    cout <<"smash got an alarm"<<endl;
    SmallShell& smash = SmallShell::getInstance();
    pid_t pid = info->si_pid;
    JobEntry* job = smash.getJobs().getJobByPID(pid);
    if(!job)
        return;

    int res = kill(pid,SIGKILL);
    if(res == 0){
        cout << job->cmd->getCmdLine()<< " timed out!" << endl;
    }
}

