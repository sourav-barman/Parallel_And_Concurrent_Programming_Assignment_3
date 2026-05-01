#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <atomic>
#define BOUND 500
using namespace std;

vector <long long> thrTimes1, thrTimes2;
struct params {
    int N;
    int numReads;
    int numWrites;
    double lamdaR;
    double lamdaW;
};

params readInputFile(const string &fileName) {
    params p;
    ifstream fin(fileName);
    if (!fin) {
        throw runtime_error("Error in Opening File");
    }

    string key;
    while (fin >> key) {
        if (key == "N") fin >> p.N;
        else if (key == "numReads") fin >> p.numReads;
        else if (key == "numWrites") fin >> p.numWrites;
        else if (key == "lamdaR") fin >> p.lamdaR;
        else if (key == "lamdaW") fin >> p.lamdaW;
    }
    return p;
}

long long getCurrTime() {
    return chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

class RegularMRSWRegister {
    private: 
        bool *r_bit;
    public:
        RegularMRSWRegister() {
            r_bit = new bool[BOUND];
            for (int i = 1; i < BOUND; i++)
                r_bit[i] = false;
            r_bit[0] = true;
        }

        void write(int x) {
            r_bit[x] = true;
            for (int i = x-1; i >= 0; i--)
                r_bit[i] = false;
        }

        int read() {
            for (int i = 0; i < BOUND; i++) 
                if (r_bit[i])   return i;
            return -1;
        }

        ~RegularMRSWRegister() {
            delete []r_bit;
        }
};

class RegularMRSWRegisterUsingAtomicBool {
    private: 
        atomic <bool> *r_bit;
    public:
        RegularMRSWRegisterUsingAtomicBool() {
            r_bit = new atomic <bool> [BOUND];
            for (long i = 1; i < BOUND; i++)
                r_bit[i].store(false, memory_order_seq_cst);
            r_bit[0].store(true, memory_order_seq_cst);
        }

        void write(int x) {
            r_bit[x].store(true, memory_order_seq_cst);
            for (int i = x-1; i >= 0; i--)
                r_bit[i].store(false, memory_order_seq_cst);
        }

        int read() {
            for (int i = 0; i < BOUND; i++) 
                if (r_bit[i].load(memory_order_seq_cst))   return i;
            return -1;
        }

        ~RegularMRSWRegisterUsingAtomicBool() {
            delete []r_bit;
        }
};

void readerThread(int id, int numReads, double lamdaR, RegularMRSWRegister& shVar);
void writerThread(int id, int numWrites, double lamdaW, RegularMRSWRegister& shVar);

void readerThreadUsingAtomicBool(int id, int numReads, double lamdaR, RegularMRSWRegisterUsingAtomicBool& shVar);
void writerThreadUsingAtomicBool(int id, int numWrites, double lamdaW, RegularMRSWRegisterUsingAtomicBool& shVar);

int main() {
    try {
        params p = readInputFile("inp-params.txt");
        int totalThreads = p.N + 1;
        thrTimes1.assign(totalThreads, 0);
        thrTimes2.assign(totalThreads, 0);

        //---Without Atomic Bool---//
        RegularMRSWRegister reg1;
        vector <thread> threads1;
        threads1.reserve(totalThreads);
        for (int i = 0; i < p.N; i++) {
            threads1.emplace_back(readerThread, i, p.numReads, p.lamdaR, ref(reg1));
        }
        threads1.emplace_back(writerThread, p.N, p.numWrites, p.lamdaW, ref(reg1));

        for (int i = 0; i <= p.N; i++) {
            threads1[i].join();
        }

        cout << "All threads completed." << endl;
        cout << "Thread execution times: Without Atomic Bool " << endl;
        for (int i = 0; i < totalThreads; i++) {
            cout << "Thread " << i << " : " << thrTimes1[i] << " micro sec" << endl;
        }

        //---With Atomic Bool---//
        RegularMRSWRegisterUsingAtomicBool reg2;
        vector <thread> threads2;
        threads2.reserve(totalThreads);
        for (int i = 0; i < p.N; i++) {
            threads2.emplace_back(readerThreadUsingAtomicBool, i, p.numReads, p.lamdaR, ref(reg2));
        }
        threads2.emplace_back(writerThreadUsingAtomicBool, p.N, p.numWrites, p.lamdaW, ref(reg2));

        for (int i = 0; i <= p.N; i++) {
            threads2[i].join();
        }

        cout << "\nAll threads completed." << endl;
        cout << "Thread execution times: With Atomic Bool " << endl;
        for (int i = 0; i < totalThreads; i++) {
            cout << "Thread " << i << " : " << thrTimes2[i] << " micro sec" << endl;
        }

    } catch(const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}

void readerThread(int id, int numReads, double lamdaR, RegularMRSWRegister& shVar) {
    int val;
    double sleepTime;
    long long startTime, endTime;
  
    string logFileName = "Reader_Log_File-" + to_string(id) + "(Without_Atomic_Bool).log";
    ofstream fout(logFileName);

    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<double> expDist(1.0/lamdaR);

    for (int i = 1; i <= numReads; i++) {
        startTime = getCurrTime();
        fout << "Reader " << id << " started Read " << i << " at " << startTime << endl;

        val = shVar.read();

        endTime = getCurrTime();
        fout << "Reader " << id << " completed Read " << i << " at " << endTime << " returning value: " << val << endl;
        thrTimes1[id] += (endTime - startTime);
        sleepTime = expDist(gen);
        this_thread::sleep_for(chrono::microseconds((int)sleepTime));
    }
    fout.close();
}

void writerThread(int id, int numWrites, double lamdaW, RegularMRSWRegister& shVar) {
    double sleepTime;
    long long startTime, endTime;
    int val;
    string logFileName = "Writer_Log_File(Without_Atomic_Bool).log";
    ofstream fout(logFileName);

    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<double> expDist(1.0/lamdaW);
    uniform_int_distribution<int> valDist(0, BOUND - 1);

    for (int i = 1; i <= numWrites; i++) {
        val = valDist(gen);
        startTime = getCurrTime();

        fout << "Writer " << " started Write " << i << " at " << startTime << endl;

        shVar.write(val);

        endTime = getCurrTime();
        fout << "Writer " << " completed Write " << i << " at " << endTime << " with the value: " << val << endl;
        thrTimes1[id] += (endTime - startTime);
        sleepTime = expDist(gen);
        this_thread::sleep_for(chrono::microseconds((int)sleepTime));
    }
    fout.close();
}

void readerThreadUsingAtomicBool(int id, int numReads, double lamdaR, RegularMRSWRegisterUsingAtomicBool& shVar) {
    int val;
    double sleepTime;
    long long startTime, endTime;
  
    string logFileName = "Reader_Log_File-" + to_string(id) + "(With_Atomic_Bool).log";
    ofstream fout(logFileName);

    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<double> expDist(1.0/lamdaR);

    for (int i = 1; i <= numReads; i++) {
        startTime = getCurrTime();
        fout << "Reader " << id << " started Read " << i << " at " << startTime << endl;

        val = shVar.read();

        endTime = getCurrTime();
        fout << "Reader " << id << " completed Read " << i << " at " << endTime << " returning value: " << val << endl;
        thrTimes2[id] += (endTime - startTime);
        sleepTime = expDist(gen);
        this_thread::sleep_for(chrono::microseconds((int)sleepTime));
    }
    fout.close();
}

void writerThreadUsingAtomicBool(int id, int numWrites, double lamdaW, RegularMRSWRegisterUsingAtomicBool& shVar) {
    double sleepTime;
    long long startTime, endTime;
    int val;
    string logFileName = "Writer_Log_File(With_Atomic_Bool).log";
    ofstream fout(logFileName);

    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<double> expDist(1.0/lamdaW);
    uniform_int_distribution<int> valDist(0, BOUND - 1);

    for (int i = 1; i <= numWrites; i++) {
        val = valDist(gen);
        startTime = getCurrTime();

        fout << "Writer " << " started Write " << i << " at " << startTime << endl;

        shVar.write(val);

        endTime = getCurrTime();
        fout << "Writer " << " completed Write " << i << " at " << endTime << " with the value: " << val << endl;
        thrTimes2[id] += (endTime - startTime);
        sleepTime = expDist(gen);
        this_thread::sleep_for(chrono::microseconds((int)sleepTime));
    }
    fout.close();
}