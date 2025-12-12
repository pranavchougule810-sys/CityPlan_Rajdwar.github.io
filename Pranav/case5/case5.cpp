#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <unordered_map>
#include <string>
#include <iomanip>
using namespace std;

struct RepairJob {
    string jobID;
    string customerName;
    string deviceType;
    string issue;
    string submissionDate;
    double estimatedCost;
    int estimatedDays;
    string status; // pending, in-progress, completed
};

class TechRepairShop {
private:
    queue<RepairJob> repairQueue;
    unordered_map<string, RepairJob> jobDatabase; // jobID -> RepairJob
    unordered_map<string, int> deviceCount; // Device type counter
    double totalRevenue;
    double eWasteCollected;
    
public:
    TechRepairShop() : totalRevenue(0.0), eWasteCollected(0.0) {}
    
    double calculateCost(string deviceType) {
        if(deviceType == "Laptop") return 1500.0;
        else if(deviceType == "Mobile") return 800.0;
        else if(deviceType == "Tablet") return 1000.0;
        else if(deviceType == "Smartwatch") return 600.0;
        return 500.0;
    }
    
    int estimateRepairDays(string deviceType) {
        if(deviceType == "Laptop") return 5;
        else if(deviceType == "Mobile") return 3;
        else if(deviceType == "Tablet") return 4;
        return 2;
    }
    
    void addRepairJob(string jobID, string customer, string device, 
                      string issue, string date) {
        RepairJob job;
        job.jobID = jobID;
        job.customerName = customer;
        job.deviceType = device;
        job.issue = issue;
        job.submissionDate = date;
        job.estimatedCost = calculateCost(device);
        job.estimatedDays = estimateRepairDays(device);
        job.status = "pending";
        
        repairQueue.push(job);
        jobDatabase[jobID] = job;
        deviceCount[device]++;
    }
    
    void processRepairs(int count) {
        cout << "\n========== Processing Repair Jobs ==========\n";
        int processed = 0;
        
        while(!repairQueue.empty() && processed < count) {
            RepairJob job = repairQueue.front();
            repairQueue.pop();
            
            job.status = "completed";
            jobDatabase[job.jobID] = job;
            totalRevenue += job.estimatedCost;
            
            cout << "Job: " << job.jobID << " | Customer: " << job.customerName
                 << " | Device: " << job.deviceType
                 << " | Cost: Rs." << fixed << setprecision(2) << job.estimatedCost << endl;
            processed++;
        }
    }
    
    void searchJob(string jobID) {
        if(jobDatabase.find(jobID) != jobDatabase.end()) {
            RepairJob job = jobDatabase[jobID];
            cout << "\n========== Job Details ==========\n";
            cout << "Job ID: " << job.jobID << endl;
            cout << "Customer: " << job.customerName << endl;
            cout << "Device: " << job.deviceType << endl;
            cout << "Issue: " << job.issue << endl;
            cout << "Status: " << job.status << endl;
            cout << "Cost: Rs." << fixed << setprecision(2) << job.estimatedCost << endl;
            cout << "Est. Days: " << job.estimatedDays << endl;
        } else {
            cout << "Job not found!" << endl;
        }
    }
    
    void displayDeviceStatistics() {
        cout << "\n========== Device Type Statistics ==========\n";
        for(auto& device : deviceCount) {
            cout << device.first << ": " << device.second << " repairs" << endl;
        }
    }
    
    void trackEWaste(double kg) {
        eWasteCollected += kg;
    }
    
    void displayStatistics() {
        cout << "\n========== Shop Statistics ==========\n";
        cout << "Jobs in Queue: " << repairQueue.size() << endl;
        cout << "Total Jobs: " << jobDatabase.size() << endl;
        cout << "Total Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
        cout << "E-Waste Collected: " << eWasteCollected << " kg" << endl;
    }
    
    void loadFromCSV(string filename) {
        ifstream file(filename);
        if(!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }
        
        string line;
        getline(file, line); // Skip header
        
        while(getline(file, line)) {
            stringstream ss(line);
            string jobID, customer, device, issue, date;
            
            getline(ss, jobID, ',');
            getline(ss, customer, ',');
            getline(ss, device, ',');
            getline(ss, issue, ',');
            getline(ss, date, ',');
            
            addRepairJob(jobID, customer, device, issue, date);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    TechRepairShop shop;
    
    cout << "========== Tech Repair Shop System ==========\n";
    cout << "Loading data from case5.csv...\n";
    
    shop.loadFromCSV("case5.csv");
    shop.displayStatistics();
    shop.processRepairs(15);
    shop.displayDeviceStatistics();
    shop.trackEWaste(12.5);
    
    // Search example
    cout << "\nSearching for job J001:" << endl;
    shop.searchJob("J001");
    
    return 0;
}
