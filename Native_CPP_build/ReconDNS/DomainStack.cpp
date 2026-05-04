#include<iostream>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>

using namespace std;

struct DomainModel{};

class DomainStack{
    private:
        char domain[256];
};