#ifndef REQUEST_H
#define REQUEST_H
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

struct io {
    string op;
    string filename;
};

// class for requests 
class Request {

    private:
        string cmd;
        vector<string> args;
        string quote;
        vector<io> ioRequests;

    public:
        // defualt constructor
        Request();

        // add arg
        void add_arg(string a);

        // arg_count
        int arg_count();

        // add ioRequest
        void add_ioRequest(string o, string f);

        // setters
        void set_cmd(string c);
        void set_quote(string q) {quote = q;}
        void set_args(vector<string> v) {args = v;}

        // getters
        string get_cmd() {return cmd;}
        string get_quote() {return quote;}
        vector<string> get_args() {return args;}
        vector<io> get_ioRequests() {return ioRequests;}
};

#endif