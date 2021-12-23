#include "Request.h"
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

// constructor
Request::Request() {
    cmd = "";
    args = {};
}

// set CMD
void Request::set_cmd(string c) {
    cmd = c;
}

// add arg
void Request::add_arg(string a) {
    args.push_back(a);
}

// arg count 
int Request::arg_count() {
    return args.size();
}

// add ioRequest
void Request::add_ioRequest(string o, string f) {
    io request;
    request.op = o;
    request.filename = f;

    ioRequests.push_back(request);
}