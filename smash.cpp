#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
    struct sigaction sa, old_sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = alarmHandler;
    if(sigaction(SIGALRM, &sa, &old_sa)==-1) {
        // TODO: this msg is not official, check what needs to be done
        perror("smash error: failed to set alarm handler");
    }



    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    while(smash.alive) {

        smash.currForegroundPID = 0;
        smash.currCmd = nullptr;

        std::string cmd_line;
        if(smash.alive)
            std::cout << smash.promptName<<"> ";
        std::getline(std::cin, cmd_line);
        if(cmd_line[0] == '#') {
            continue;
        }
        smash.executeCommand(cmd_line.c_str());
        smash.getJobs().removeFinishedJobs();


    }
    return 0;
}