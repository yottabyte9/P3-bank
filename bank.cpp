// Project Identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98
#include <iostream>
#include <string>
#include <math.h>
#include <algorithm>
#include <queue>
#include <getopt.h>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

uint64_t convertTimestamp(string line);


struct User{
    public:
        int64_t timestamp;
        string id;
        int pin;
        int balance;
        vector<uint64_t> ip;
        
        User() : timestamp(0), id(""), pin(0), balance(0), ip() {
        }

        // Constructor with integer timestamp
        User(int64_t ts, string idi, int pini, int balancei)
            : timestamp(ts), id(idi), pin(pini), balance(balancei), ip() {
        }

        // Constructor with string timestamp
        User(string ts, string idi, int pini, int balancei)
            : timestamp(convertTimestamp(ts)), id(idi), pin(pini), balance(balancei), ip() {
        }
};
unordered_map<string, User> usermap;

struct Transaction{
    public:
        int64_t timestamp;
        uint64_t ip;
        User& sender;
        User& recipient;
        int amount;
        int64_t exec;
        bool o; //true -> o, false -> s
        int fee;

        Transaction(int64_t ts, uint64_t ipAddress, User& sndr, User& rcpt, int amnt, int64_t e, bool isO)
        : timestamp(ts), ip(ipAddress), sender(sndr), recipient(rcpt), amount(amnt), exec(e), o(isO), fee() {
        }
        Transaction(int64_t ts, uint64_t ipAddress, string sndrid, string rcptid, int amnt, int64_t e, bool isO)
        : timestamp(ts), ip(ipAddress), sender(usermap[sndrid]), recipient(usermap[rcptid]), amount(amnt), exec(e), o(isO), fee() {
        }

        Transaction& operator=(Transaction& other) {
        if (this != &other) {
            timestamp = other.timestamp;
            ip = other.ip;
            sender = other.sender;
            recipient = other.recipient;
            amount = other.amount;
            exec = other.exec;
            o = other.o;
            fee = other.fee;
        }
        return *this;
    }
};
struct Comparator{
    bool operator()(Transaction t1, Transaction t2) const{
        if(t1.exec > t2.exec){
            return true;
        }
        return false;
    }
};
priority_queue <Transaction, vector<Transaction>, Comparator> transpq; //track transactions



bool f, v = false;
string filename;


void cl(int argc, char** argv){
    int gotopt;
    int option_index = 0;
    option long_opts[]={
        {"verbose", no_argument, nullptr, 'v'},
        {"file", required_argument, nullptr, 'f'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, '\0'}
    };

    while((gotopt = getopt_long(argc, argv, "vf:h", long_opts, &option_index)) != -1){
        switch(gotopt){
            case 'h':
                cout << "help";
                exit(0);
            case 'f':
                f = true;
                filename = optarg;
                break;
            case 'v':
                v = true;
                break;
            default:
                cout << "default error\n";
                exit(1);
        }
    }
}

uint64_t convertTimestamp(string line){ //converts ips as well
    string cleanedInput;
    for (char c : line) {
        if (c != ':' && c != '.') {
            cleanedInput += c;
        }
    }
    return static_cast<uint64_t>(std::stoul(cleanedInput));
}


void regfill(){
    ifstream infile(filename);
    string line;
    while(getline(infile, line)){
        istringstream iss(line); // Use istringstream to split the line
        string timestampPart, id, pinPart, balancePart;
        
        // Split the line based on '|'
        if (getline(iss, timestampPart, '|') &&
            getline(iss, id, '|') &&
            getline(iss, pinPart, '|') &&
            getline(iss, balancePart, '|')) {
            User temp(timestampPart, id, stoi(pinPart), stoi(balancePart));
            
            usermap[temp.id] = temp;
        }
    }
}

bool validLogin(string id, int pin){ //make IP into number
    auto it = usermap.find(id);
    if(it==usermap.end()){
        return false; //no user there
    }
    else if(usermap[id].pin != pin){
        return false; //wrong pin
    } //already logged in???
    return true;
}

bool validTransaction(int64_t ts, uint64_t ipAddress, string sndrid, string rcptid, int64_t e){
    //cout << e << " checking validity \n";
    if(ts + 3000000 < e){
        //cout << "wrong date \n";
        return false;
    }
    if(usermap.find(rcptid) == usermap.end()){ //recipient does not exist
        //cout << "no reciever\n";
        return false;
    }
    if(usermap.find(sndrid) == usermap.end()){ //sender does not exist
        //cout << "no sender\n";
        return false;
    }
    User& sndr = usermap[sndrid];
    User& rcpt = usermap[rcptid];
    auto it = find(sndr.ip.begin(), sndr.ip.end(), ipAddress);
    if(it == sndr.ip.end()){
        //cout << "no ip\n";
        return false;
    }
    if(usermap[sndr.id].timestamp > ts || usermap[rcpt.id].timestamp > ts){
        //cout << "someone not registered yet\n";
        return false;
    }
    //cout << "passed validity \n";
    return true;
}

bool checkamt(Transaction place){
    int fee = (int)(place.amount *0.01);
    if(fee < 10){
        fee = 10;
    }
    if(fee > 450){
        fee = 450;
    }
    //place.amount += (int)fee;
    if(place.o){ //sender cover
        if(place.sender.balance < place.amount + fee) return false;
    }
    else if(!place.o){
        if(place.sender.balance < place.amount) return false;
        if(place.recipient.balance + place.amount < fee) return false;
    }
    place.fee = fee;
    cout << place.sender.balance << "\n";
    return true;

}

void transactionfill(){
    //cout << "begin transaction fill \n";
    string line;
    //user ip must be updated
    while(getline(cin, line)){
        if(cin.peek() == '#'){
            while(cin.peek() != '\n'){
                cin >> line;
            }
            continue;
        }
        cin >> line;
        if(line == "login"){
            //cout << "started login\n";
            string id;
            string pin;
            string ip;
            cin >> id >> pin >> ip;
            if(validLogin(id, stoi(pin))){
                usermap[id].ip.push_back(convertTimestamp(ip));
            }
        }
        else if(line == "place"){
            string timestamp;
            string ip;
            string sender;
            string recipient;
            string amount;
            string exec;
            string os;
            cin >> timestamp >> ip >> sender >> recipient >> amount >> exec >> os;
            //cout << "Transaction: " << ip << sender << recipient << "  ";
            bool isO = (os=="o");
            uint64_t convertedIP = convertTimestamp(ip);
            if(validTransaction(convertTimestamp(timestamp), convertedIP, sender, recipient, convertTimestamp(exec))){
                Transaction temp(convertTimestamp(timestamp), convertedIP, sender, recipient, stoi(amount), convertTimestamp(exec), isO);
                transpq.push(temp);
            }
            //cout << "Transaction done \n";
        }
    }
}

vector<Transaction> transdone;

void place(){
    //cout << transpq.size();
    while(!transpq.empty()){
        Transaction temp = transpq.top();
        if(checkamt(transpq.top())){
            cout << temp.sender.id << ", " << temp.recipient.id << ", " << temp.amount << endl;
            /* if(temp.o){ //seller covers fee
                temp.sender.balance -= temp.fee;
            }
            else{
                temp.sender.balance -= (int)(temp.fee/2);
                temp.amount -= (int)(temp.fee/2);
            } */
            temp.sender.balance -= temp.amount;
            temp.recipient.balance += temp.amount;
        }
        transpq.pop();
    }
}

int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    cl(argc, argv);
    regfill();
    transactionfill();
    place();
};
