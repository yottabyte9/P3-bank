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
#include <set>
#include <unordered_set>

using namespace std;

uint64_t convertTimestamp(string line);
uint64_t currentTime;
int counterid=0;
void querylist();
void place();

struct User{
    public:
        uint64_t timestamp;
        string id;
        int pin;
        int balance;
        unordered_set<string> ip;
        
        User() : timestamp(0), id(""), pin(0), balance(0), ip() {
        }

        // Constructor with integer timestamp
        User(uint64_t ts, string idi, int pini, int balancei)
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
        uint64_t timestamp;
        string ip;
        User& sender;
        User& recipient;
        int amount;
        uint64_t exec;
        bool o; //true -> o, false -> s
        int fee;
        bool done = false;
        int id;

        Transaction(uint64_t ts, string ipAddress, User& sndr, User& rcpt, int amnt, uint64_t e, bool isO, int id)
        : timestamp(ts), ip(ipAddress), sender(sndr), recipient(rcpt), amount(amnt), exec(e), o(isO), fee(), done(), id(id){
        }
        Transaction(uint64_t ts, string ipAddress, string sndrid, string rcptid, int amnt, uint64_t e, bool isO, int id)
        : timestamp(ts), ip(ipAddress), sender(usermap[sndrid]), recipient(usermap[rcptid]), amount(amnt), exec(e), o(isO), fee(), done(), id(id) {
        }
};
struct Comparator{
    bool operator()(Transaction* t1, Transaction* t2) const{
        if(t1->exec > t2->exec){
            return true; //if t2 comes first
        }
        if(t1->exec == t2->exec){
            return t1->id > t2->id; //true if t2 comes first
        }
        return false;
    }
};
priority_queue <Transaction*, vector<Transaction*>, Comparator> transpq; //track transactions



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

bool validTransaction(uint64_t ts, string ipAddress, string sndrid, string rcptid, string amt, uint64_t e){
    //cout << e << " checking validity \n";
    if(ts + 3000000 < e){
        if(v) cout << "Select a time less than three days in the future.\n";
        return false;
    }
    if(usermap.find(sndrid) == usermap.end()){ //sender does not exist
        if(v) cout << "Sender " << sndrid << " does not exist.\n";
        return false;
    }
    if(usermap.find(rcptid) == usermap.end()){ //recipient does not exist
        if(v) cout << "Recipient " << rcptid << " does not exist.\n";
        return false;
    }
    User& sndr = usermap[sndrid];
    User& rcpt = usermap[rcptid];
    if(usermap[sndr.id].timestamp > e || usermap[rcpt.id].timestamp > e){
        if(v) cout << "At the time of execution, sender and/or recipient have not registered.\n";
        return false;
    }
    if(sndr.ip.size() == 0){
        if(v) cout << "Sender " << sndrid << " is not logged in.\n";
        return false;
    }
    if(sndr.ip.find(ipAddress) == sndr.ip.end()){
        if(v) cout << "Fraudulent transaction detected, aborting request.\n";
        return false;
    }
    if(v) cout << "Transaction placed at " << ts << ": $" << amt <<" from " << sndrid << " to " << rcptid << " at " << e << ".\n";
    return true;
}

bool notvalidTransaction(uint64_t ts, string ipAddress, string sndrid, string rcptid, uint64_t e){
    //cout << e << " checking validity \n";
    if(ts + 3000000 < e){
        if(v) cout << "Select a time less than three days in the future.\n";
        return false;
    }
    if(usermap.find(sndrid) == usermap.end()){ //sender does not exist
        if(v) cout << "Sender " << sndrid << " does not exist.\n";
        return false;
    }
    if(usermap.find(rcptid) == usermap.end()){ //recipient does not exist
        if(v) cout << "Recipient " << rcptid << " does not exist.\n";
        return false;
    }
    User& sndr = usermap[sndrid];
    User& rcpt = usermap[rcptid];
    if(usermap[sndr.id].timestamp > e || usermap[rcpt.id].timestamp > e){
        if(v) cout << "At the time of execution, sender and/or recipient have not registered.\n";
        return false;
    }
    if(sndr.ip.size() == 0){
        if(v) cout << "Sender " << sndrid << " is not logged in.\n";
        return false;
    }
    if(sndr.ip.find(ipAddress) == sndr.ip.end()){
        if(v) cout << "Fraudulent transaction detected, aborting request.\n";
        return false;
    }
    return true;
}

bool checkamt(Transaction* place){
    int fee = place->amount/100;
    if(fee < 10){
        fee = 10;
    }
    if(fee > 450){
        fee = 450;
    }
    if(place->timestamp > place->sender.timestamp + 50000000000){
        fee = (fee*3)/4;
    }
    //place.amount += (int)fee;
    if(place->o){ //sender cover
        if(place->sender.balance < place->amount + fee){
            if(v){
                cout << "Insufficient funds to process transaction " << place->id << ".\n"; //is this IP or ID?
            }
            return false;
        } 
    }
    else if(!place->o){//split cover
        if(place->sender.balance < place->amount + fee - fee/2){
            if(v) cout << "Insufficient funds to process transaction " << place->id << ".\n";
            return false;
        }
        if(place->recipient.balance < fee/2){
            if(v) cout << "Insufficient funds to process transaction " << place->id << ".\n";
            return false;
        }
    }
    place->fee = fee;
    return true;
} 

vector<Transaction*> transdone;

void transactionfill(){
    string line;
    // User IP must be updated
    while(getline(cin, line)) {
        // Skip comment lines
        if (!line.empty() && line[0] == '#') {
            continue;
        }

        stringstream ss(line); // Use a stringstream to process the current line
        ss >> line;

        if (line == "login") {
            string id, pin, ip;
            ss >> id >> pin >> ip;
            if(validLogin(id, stoi(pin))){
                usermap[id].ip.insert(ip);
                if(v){
                    cout << "User " << id << " logged in.\n";
                }
            }
            else{
                if(v){
                    cout << "Failed to log in " << id << ".\n";
                }
            }
        }
        else if (line == "out") {
            string id, ip;
            ss >> id >> ip;
            auto userIt = usermap.find(id);
            if( userIt == usermap.end()){
                cout << "Failed to log out " << id << ".\n"; 
            }
            else if (userIt != usermap.end()) {
                bool flag = false;
                if(usermap[id].ip.find(ip) != usermap[id].ip.end()){
                    flag = true;
                    usermap[id].ip.erase(ip);
                }
                else if(usermap[id].ip.find(ip) == usermap[id].ip.end()){
                    flag = false;
                }
                if(flag){
                    if(v){
                        cout << "User " << id << " logged out.\n";
                    }
                }
                else{
                    if(v){
                        cout << "Failed to log out " << id << ".\n";
                    }
                }
            }
        }
        else if (line == "place") {
            string timestamp, ip, sender, recipient, amount, exec, os;
            ss >> timestamp >> ip >> sender >> recipient >> amount >> exec >> os;
            //cout << "Transaction: " << ip << sender << recipient << "  ";
            bool isO = (os=="o");
            if(convertTimestamp(exec) < convertTimestamp(timestamp)){
                cerr << "execution date earlier than place date\n";
                exit(1);
            }
            if(convertTimestamp(timestamp) < currentTime){
                cerr << "place order earlier than current time\n";
                exit(1);
            }
            if(notvalidTransaction(convertTimestamp(timestamp), ip, sender, recipient, convertTimestamp(exec))){
                while(transpq.size() > 0 && transpq.top()->exec <= convertTimestamp(timestamp)){
                    Transaction* temp = transpq.top();
                    if(checkamt(temp)){
                        if(temp->o){ //seller covers fee
                            temp->sender.balance -= temp->fee;
                            temp->sender.balance -= temp->amount;
                            temp->recipient.balance += temp->amount; 
                        }
                        else{
                            temp->sender.balance -= temp->fee - temp->fee/2;
                            temp->sender.balance -= temp->amount;
                            temp->recipient.balance += temp->amount - temp->fee/2;
                        }
                        //transdone.push_back(temp);
                        if(v){
                            cout << "Transaction executed at " << temp->exec << ": $" << temp->amount << " from " << temp->sender.id << " to " << temp->recipient.id << ".\n";
                        }
                        temp->done = true;
                    }
                    transdone.push_back(temp);
                    transpq.pop();
                }
                if(validTransaction(convertTimestamp(timestamp), ip, sender, recipient, amount, convertTimestamp(exec))){
                    //cout << counterid << endl;
                    Transaction *temp = new Transaction(convertTimestamp(timestamp), ip, sender, recipient, stoi(amount), convertTimestamp(exec), isO, counterid);
                    counterid++;
                    transpq.push(temp);
                }
                currentTime = convertTimestamp(timestamp);
            }
            //cout << transdone.size() << "\n";
        }
        else if(line == "$$$"){
            place();
            querylist();
            break;
        }
    }
}

void place(){
    while(!transpq.empty()){
        Transaction* temp = transpq.top();
        transdone.push_back(temp);
        if(checkamt(temp)){
            if(temp->o){ //seller covers fee
                temp->sender.balance -= temp->fee;
                temp->sender.balance -= temp->amount;
                temp->recipient.balance += temp->amount;
            }
            else{
                temp->sender.balance += (int)(temp->fee/2);//fix 
                temp->sender.balance -= (temp->amount + temp->fee);
                temp->recipient.balance += temp->amount - (int)(temp->fee/2);
            }
            //transdone.push_back(temp);
            temp->done = true;
            if(v){
                cout << "Transaction executed at " << temp->exec << ": $" << temp->amount << " from " << temp->sender.id << " to " << temp->recipient.id << ".\n";
            }
        }
        /* else{
            delete temp;
        } */
        transpq.pop();
    }
}

string formatTimeDifference(uint64_t startTimestamp, uint64_t endTimestamp) {
    uint64_t seconds = (endTimestamp - startTimestamp);
    
    uint64_t years = seconds / 10000000000;
    seconds %= 10000000000;

    uint64_t months = seconds / 100000000 ;
    seconds %= 100000000;

    uint64_t days = seconds / 1000000;
    seconds %= 1000000;

    uint64_t hours = seconds / 10000;
    seconds %= 10000;

    uint64_t minutes = seconds / 100;
    seconds %= 100;

    string result;
    if (years > 0) {
        result += " " + to_string(years) + " year" + (years > 1 ? "s" : "");
    }
    if (months > 0) {
        result += " " + to_string(months) + " month" + (months > 1 ? "s" : "");
    }
    if (days > 0) {
        result += " " + to_string(days) + " day" + (days > 1 ? "s" : "");
    }
    if (hours > 0) {
        result += " " + to_string(hours) + " hour" + (hours > 1 ? "s" : "");
    }
    if (minutes > 0) {
        result += " " + to_string(minutes) + " minute" + (minutes > 1 ? "s" : "");
    }
    if (seconds > 0) {
        result += " " + to_string(seconds) + " second" + (seconds > 1 ? "s" : "");
    }
    //return to_string((endTimestamp - startTimestamp-1));
    return result;
}

void querylist(){
    string indicator;
    string in1;
    string in2;
    while(getline(cin, indicator)){
        stringstream ss(indicator);
        ss >> indicator;
        if(indicator == "l"){
            ss >> in1 >> in2;
            uint64_t ts1 = convertTimestamp(in1);
            uint64_t ts2 = convertTimestamp(in2);
            int counter = 0;
            for(int i=0; i<(int)(transdone.size()); i++){
                if(transdone[i]->exec < ts2 && transdone[i]->exec >= ts1 && transdone[i]->done){
                    if(transdone[i]->amount == 1){
                        cout << to_string(transdone[i]->id) << ": " << transdone[i]->sender.id << " sent " << transdone[i]->amount << " dollar to " << transdone[i]->recipient.id << " at " << transdone[i]->exec << ".\n";
                    }
                    else{
                        cout << to_string(transdone[i]->id) << ": " << transdone[i]->sender.id << " sent " << transdone[i]->amount << " dollars to " << transdone[i]->recipient.id << " at " << transdone[i]->exec << ".\n";
                    }
                    counter++;
                }
            }
            if(counter == 1){
                cout << "There was " << counter << " transaction that was placed between time " << ts1 << " to " << ts2 << ".\n";
            }
            else{
                cout << "There were " << counter << " transactions that were placed between time " << ts1 << " to " << ts2 << ".\n";

            }
        }
        else if(indicator == "r"){
            ss >> in1 >> in2;
            uint64_t ts1 = convertTimestamp(in1);
            uint64_t ts2 = convertTimestamp(in2);
            int revenue = 0;
            for(int i=0; i<(int)(transdone.size()); i++){
                if(transdone[i]->exec < ts2 && transdone[i]->exec >= ts1 && transdone[i]->done){
                    revenue += transdone[i]->fee;
                }
            }
            
            cout << "281Bank has collected " << revenue << " dollars in fees over" << formatTimeDifference(ts1, ts2) << ".\n";
        }
        else if(indicator == "h"){
            ss >> in1;
            string id = in1;
            if(usermap.find(id) == usermap.end()){
                cout << "User " << id << " does not exist.\n";
            }
            else{
                int incounter = 0;
                int outcounter = 0;
                vector<string> transout;
                vector<string> transin;
                for(int i=(int)(transdone.size())-1; i>=0; i--){
                    if(transdone[i]->sender.id.compare(id) == 0 && transdone[i]->done){
                        if(outcounter < 10){
                            string p;
                            if(transdone[i]->amount == 1){
                                p = "" + to_string(transdone[i]->id) + ": " + transdone[i]->sender.id+" sent "+to_string(transdone[i]->amount)+" dollar to "+transdone[i]->recipient.id+" at "+to_string(transdone[i]->exec)+".";
                            }
                            else{
                                p = "" + to_string(transdone[i]->id) + ": " + transdone[i]->sender.id+" sent "+to_string(transdone[i]->amount)+" dollars to "+transdone[i]->recipient.id+" at "+to_string(transdone[i]->exec)+".";
                            }
                            transout.push_back(p);
                        }
                        outcounter++;
                    }
                    if(transdone[i]->recipient.id.compare(id) == 0 && transdone[i]->done){
                        if(incounter < 10){
                            string p;
                            if(transdone[i]->amount == 1){
                                p = "" + to_string(transdone[i]->id) + ": " + transdone[i]->sender.id+" sent "+to_string(transdone[i]->amount)+" dollar to "+transdone[i]->recipient.id+" at "+to_string(transdone[i]->exec)+".";
                            }
                            else{
                                p = "" + to_string(transdone[i]->id) + ": " + transdone[i]->sender.id+" sent "+to_string(transdone[i]->amount)+" dollars to "+transdone[i]->recipient.id+" at "+to_string(transdone[i]->exec)+".";
                            }
                            transin.push_back(p);
                        }
                        incounter++;
                    }
                }
                cout << "Customer " << id << " account summary:\n";
                cout << "Balance: $" << usermap[id].balance << "\n";
                cout << "Total # of transactions: " << incounter + outcounter << "\n";
                cout << "Incoming " << incounter << ":\n";
                for(int i=(int)(transin.size()-1); i>=0; i--){
                    cout << transin[i] << "\n";
                }
                cout << "Outgoing " << outcounter << ":\n";
                for(int i=(int)(transout.size()-1); i>=0; i--){
                    cout << transout[i] << "\n";
                }
            }
        }
        else if(indicator == "s"){
            ss >> in1;
            uint64_t tsDay = convertTimestamp(in1);
            tsDay = (tsDay/1000000 )*1000000 ;
            uint64_t tsEnd = tsDay + 1000000;
            cout << "Summary of [" << tsDay << ", " << tsEnd << "):\n";
            int counter = 0;
            int revenue = 0;
            for(int i=0; i<(int)(transdone.size()); i++){
                if(transdone[i]->exec < tsEnd && transdone[i]->exec >= tsDay && transdone[i]->done){
                    if(transdone[i]->amount == 1){
                        cout << transdone[i]->id << ": " << transdone[i]->sender.id << " sent " << transdone[i]->amount << " dollar to " << transdone[i]->recipient.id << " at " << transdone[i]->exec << ".\n";
                    }
                    else{
                        cout << transdone[i]->id << ": " << transdone[i]->sender.id << " sent " << transdone[i]->amount << " dollars to " << transdone[i]->recipient.id << " at " << transdone[i]->exec << ".\n";
                    }
                    counter++;
                    revenue += transdone[i]->fee;
                }
                else if(transdone[i]->exec > tsEnd){
                    break;
                }
            }
            if(counter == 1){
                cout << "There was a total of " << counter << " transaction, 281Bank has collected " << revenue << " dollars in fees.\n";
            }
            else{
                cout << "There were a total of " << counter << " transactions, 281Bank has collected " << revenue << " dollars in fees.\n";
            }
        }
    }
}

void removeall(){
    for(int i=0; i<(int)(transdone.size()); i++){
        if(transdone[i] != nullptr){
            delete transdone[i];
        }
    }
    if(transpq.size() != 0){
        cout << "FAILED TO DELETE TRANSACTION PQ OBJECTS";
    }
}

int main(int argc, char **argv){
    ios_base::sync_with_stdio(false);
    cl(argc, argv);
    regfill();
    transactionfill();
    //place();
    //querylist();
    removeall();
};
