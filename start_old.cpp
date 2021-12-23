#include <stdio.h>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <filesystem>
#include "Request.h" // this is a class for cmd's

using namespace std;

int main (){
    // get $USER
    char* USER = getlogin();

    // get $PATH
    char* PATH = get_current_dir_name();

    while (true){
        // make copies of stdout/stdin
        dup2(0,3);
        dup2(1,4);

        // get time
        time_t t = time(nullptr);
        string TIME = ctime(&t);
        TIME[TIME.length() - 1] = '\0'; // this is used to remove endl char
        cout << USER << ": " << TIME << " $ ";

        // check for exit command
        string inputline;
        getline (cin, inputline);   // get a line from standard input
        if (inputline == string("exit")){
            cout << "Bye!! End of shell" << endl;
            break;
        }

        // stringstream initialization
        stringstream ss(inputline);
        
        // create word holder
        string word;

        // create vector to store requests
        vector<Request> requestVector = {};

        // get requests
        while (ss >> word) {
            
            // create a request obj
            Request r = Request();
            r.set_cmd(word);

            // get args
            while (ss >> word) {
                if (word == "|") {
                    break; // return to parent-while to make a new request
                } else if (word.at(0) == '\'' || word.at(0) == '\"') { // if there are quotes
                    // store qtype
                    char qType = word.at(0);
                    // convert stringstream to string
                    string fullLine = ss.str();
                    // find open quote of "" OR ''
                    size_t openQ = fullLine.find_first_of(qType);
                    // remove open quate and everyghing before it
                    fullLine.erase(0,openQ+1);
                    // find close quote of "" OR ''
                    size_t closeQ = fullLine.find_first_of(qType);
                    // get the echo
                    string quote = fullLine.substr(0,closeQ);
                    // add argument
                    r.add_arg(quote);
                    // delete quote
                    fullLine.erase(0, closeQ + 1);
                    // put the updated fullLine back into stringstream
                    ss.str("");
                    ss.clear();
                    ss << fullLine;
                } else if (word == ">" || word == "<") { // ioRequest
                    string op = word;
                    string filename = "";
                    ss >> filename;
                    r.add_ioRequest(op, filename);
                }
                    else {
                    // ARG
                    r.add_arg(word);
                }
            }

            // add the request to requestVector
            requestVector.push_back(r);
        }

        // //used for debugging
        // for (int i = 0; i < requestVector.size(); i ++) {
        //     cout << "Request" << "(" << i << ")" << ": ";
        //     cout << "(cmd: " << requestVector[i].get_cmd() << ") ";
        //     cout << "(args: ";
        //     vector<string> outVect = requestVector[i].get_args();
        //     for (int i = 0; i < outVect.size(); i ++) {
        //         cout << outVect[i] << " ";
        //     }
        //     cout << ") ";
        //     cout << "(path: " << requestVector[i].get_cmdPATH() << ") ";
        //     cout << "(quote: " << requestVector[i].get_quote() << ") " << endl;
        //     vector<io> ioR = requestVector[i].get_ioRequests();
        //     cout << "ioRequests: " << endl;
        //     for (int i = 0; i < ioR.size(); i ++) {
        //         if (ioR[i].op == ">") {
        //             cout << "Requested program output to " <<  ioR[i].filename << endl;
        //         }

        //         if (ioR[i].op == "<") {
        //             cout << "Requested program input from " <<  ioR[i].filename << endl;
        //         }
                
        //     }
        // }

        // initial fork
        int pid = fork ();
        if (pid == 0) { //child process

            // if there are pipes
            if (requestVector.size() > 1) {
                // iterate though all requests two at a time
                for (int i = 0; i < requestVector.size() - 1; i++) { // stop one before
                    // make sure standard in and out are good  
                    dup2(3,0);
                    dup2(4,1);

                    // get commands
                    
                    char* command1 = (char*) (requestVector[i].get_cmd()).c_str();

                    // get args
                    vector<string> argVector1 = requestVector[i].get_args();
                    char* args1 [] = {command1};
                    int argsSize1 = 1;

                    // set args
                    for (int i = 0; i < argVector1.size(); i++) {
                        // add arguments to the args array
                        args1[argsSize1] = (char*) argVector1[i].c_str();

                        // increment argsSize
                        argsSize1++;
                    }

                    // add NULL to args
                    args1[argsSize1] = NULL;

                    // get commands
                    char* command2 = (char*) (requestVector[i+1].get_cmd()).c_str();

                    // get args
                    vector<string> argVector2 = requestVector[i+1].get_args();
                    char* args2 [] = {command2};
                    int argsSize2 = 1;

                    // set args
                    for (int i = 0; i < argVector2.size(); i++) {
                        // add arguments to the args array
                        args2[argsSize2] = (char*) argVector2[i].c_str();

                        // increment argsSize
                        argsSize2++;
                    }

                    // add NULL to args
                    args2[argsSize2] = NULL;

                    // establish pipe
                    int fds[2];
                    pipe(fds);

                    // get input out requests if any (command 1)
                    vector<io> ioRequests1 = requestVector[i].get_ioRequests();
                    bool io1 = false;
                    if (ioRequests1.size() > 0) {
                        io1 = true;
                    }

                    // get in/out redirecs if any (command2)
                    vector<io> ioRequests2 = requestVector[i+1].get_ioRequests();
                    bool io2 = false;
                    if (ioRequests2.size() > 0) {
                        io2 = true;
                    }

                    int cpid= fork();
                    if (cpid == 0) { // child reads from pipe

                        // if io1 exists
                        if (io1) {
                            // fork this process
                            int ioRfork1 = fork();
                            if (ioRfork1 == 0) {

                                // iterate though ioRequests1
                                for (int i = 0; i < ioRequests1.size(); i ++) {
                                    // if request input from file
                                    if (ioRequests1[i].op == "<") { // program input is a file

                                        // right is a file --> open it
                                        char* f = (char*) ioRequests1[i].filename.c_str();
                                        int fd = open (f, O_RDONLY);
                                        cout << "Opened " << ioRequests1[i].filename << " fd(" << fd << ")" << endl;
                                        
                                        // redirect file descriptor
                                        dup2(fd, 0);
                                    }                   

                                    // if request output to file
                                    if (ioRequests1[i].op == ">") { //output of pgm goes to file

                                        // right is a file --> open it
                                        char* f = (char*) ioRequests1[i].filename.c_str();
                                        int fd = open (f, O_RDWR|O_CREAT|O_TRUNC, 0700);
                                        cout << "Created " << ioRequests1[i].filename << " fd(" << fd << ")" << endl;

                                        // redirect file descriptor
                                        dup2(fd, 1);
                                    }
                                }

                                // set command
                                command1 = (char*) (requestVector[i].get_cmd()).c_str();

                                // after looping exec to the file
                                execvp(command1, args1);

                            } else  {
                                waitpid(ioRfork1, 0, 0);
                            }
                        }

                        if (i != requestVector.size() - 1) { // on every command except last                     
                            dup2(fds[1], 1);
                            close(fds[0]);
                        }
                        

                        
                        command1 = (char*) (requestVector[i].get_cmd()).c_str();
                        execvp(command1, args1);

                    } else { // parent writes to pipe
                        
                        // if io1 exists
                        if (io2) {
                            // fork this process
                            int ioRfork2 = fork();
                            if (ioRfork2 == 0) {

                                // iterate though ioRequests1
                                for (int i = 0; i < ioRequests2.size(); i ++) {
                                    // if request input from file
                                    if (ioRequests2[i].op == "<") { // program input is a file

                                        // right is a file --> open it
                                        char* f = (char*) ioRequests2[i].filename.c_str();
                                        int fd = open (f, O_RDONLY);
                                        cout << "Opened " << ioRequests2[i].filename << " fd(" << fd << ")" << endl;
                                        
                                        // redirect file descriptor
                                        dup2(fd, 0);
                                    }                   

                                    // if request output to file
                                    if (ioRequests2[i].op == ">") { //output of pgm goes to file

                                        // right is a file --> open it
                                        char* f = (char*) ioRequests2[i].filename.c_str();
                                        int fd = open (f, O_RDWR|O_CREAT|O_TRUNC, 0700);
                                        cout << "Created " << ioRequests2[i].filename << " fd(" << fd << ")" << endl;

                                        // redirect file descriptor
                                        dup2(fd, 1);
                                    }
                                }

                                // set command
                                command2 = (char*) (requestVector[i+1].get_cmd()).c_str();

                                // after looping exec to the file
                                execvp(command2, args2);

                            } else  {
                                waitpid(ioRfork2, 0, 0);
                            }
                        }

                        dup2(fds[0],0);
                        close(fds[1]);
                        
                        command2 = (char*) (requestVector[i+1].get_cmd()).c_str();
                        execvp(command2, args2);
                        
                        if (i == requestVector.size() - 1) {
                            waitpid(cpid, 0, 0);
                        }
                    }
                }


            } else { // if there are no pipes

                // get arg vector
                vector<string> argVector = requestVector[0].get_args();

                char* command = (char*) (requestVector[0].get_cmd()).c_str();
                char* args [] = {command};
                int argsSize = 1;

                // set args
                for (int i = 0; i < argVector.size(); i++) {
                    // add arguments to the args array
                    args[argsSize] = (char*) argVector[i].c_str();

                    // increment argsSize
                    argsSize++;
                }

                // add null to end
                args[argsSize] = NULL;

                // get input out requests
                vector<io> ioRequests = requestVector[0].get_ioRequests();
                bool io = false;
                if (ioRequests.size() > 0) {
                    io = true;
                }

                if (io) { // IO redirection requests
                    int first = fork();
                    if (first == 0) {

                        for (int i = 0; i < ioRequests.size(); i ++) {
                            if (ioRequests[i].op == "<") { // program input is a file

                                // right is a file --> open it
                                char* f = (char*) ioRequests[i].filename.c_str();
                                int fd = open (f, O_RDONLY);
                                cout << "Opened " << ioRequests[i].filename << " fd(" << fd << ")" << endl;

                                // set command (strange fix)
                                // command = (char*) (requestVector[0].get_cmd()).c_str();

                                dup2(fd, 0);
                            } 

                            if (ioRequests[i].op == ">") { //output of pgm goes to file

                                // right is a file --> open it
                                char* f = (char*) ioRequests[i].filename.c_str();
                                int fd = open (f, O_RDWR|O_CREAT|O_TRUNC, 0700);
                                cout << "Created " << ioRequests[i].filename << " fd(" << fd << ")" << endl;
                                dup2(fd, 1);
                            }
                        }
                    
                        // after for loop 
                        // execute with updated fds
                        execvp(command, args);
                        
                    } else {
                        waitpid (first, 0, 0);
                    }

                } else { // if no IO request
                    cout << "";
                    execvp(command, args);
                }

            }

        } else {
            waitpid (pid, 0, 0);
        }

        // re-establish stdout/stdin
        dup2(3,0);
        dup2(4,1);
    }
}