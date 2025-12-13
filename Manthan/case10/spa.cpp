// ==================================
// SPA MANAGEMENT SYSTEM (REALISTIC)
// With FULL STAFF MANAGEMENT
// ==================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

// ---------------- CONFIG ----------------
#define SPA_MAX_CUSTOMERS     2000
#define SPA_MAX_STAFF         500
#define SPA_MAX_SERVICES      500
#define SPA_MAX_APPOINTMENTS  3000
#define SPA_MAX_BILLS         3000

// ---------------- STRUCTS ----------------
struct SpaCustomer {
    int id;
    string name;
    string phone;
};

struct SpaStaff {
    int id;
    string name;
    string role;       // Therapist / Receptionist / Manager
    string specialty;  // For therapists
    int available;     // 1 = free, 0 = busy
};

struct SpaService {
    int id;
    string name;
    int duration;
    double price;
};

struct SpaAppointment {
    int id;
    int customerId;
    int serviceId;
    int staffId;
    string timeSlot;
    string status; // Booked / InProgress / Completed
};

struct SpaBill {
    int id;
    int appointmentId;
    double amount;
    string status; // Pending / Paid
};

// ---------------- STORAGE ----------------
SpaCustomer customers[SPA_MAX_CUSTOMERS];
SpaStaff staff[SPA_MAX_STAFF];
SpaService services[SPA_MAX_SERVICES];
SpaAppointment appointments[SPA_MAX_APPOINTMENTS];
SpaBill bills[SPA_MAX_BILLS];

int customerCount = 0;
int staffCount = 0;
int serviceCount = 0;
int appointmentCount = 0;
int billCount = 0;

// ---------------- CSV HELPER ----------------
int splitCSV(const string &line, string out[], int maxCols) {
    stringstream ss(line);
    string item;
    int i = 0;
    while (getline(ss, item, ',') && i < maxCols)
        out[i++] = item;
    return i;
}

// ---------------- LOAD CSVs ----------------
void spaLoadCustomers() {
    ifstream f("spa_customers.csv");
    if (!f) return;
    string line, c[4];
    getline(f, line);
    while (getline(f, line) && customerCount < SPA_MAX_CUSTOMERS) {
        splitCSV(line, c, 4);
        customers[customerCount++] = {stoi(c[0]), c[1], c[2]};
    }
}

void spaLoadStaff() {
    ifstream f("spa_staff.csv");
    if (!f) return;
    string line, c[6];
    getline(f, line);
    while (getline(f, line) && staffCount < SPA_MAX_STAFF) {
        splitCSV(line, c, 6);
        staff[staffCount++] = {
            stoi(c[0]), c[1], c[2], c[3], stoi(c[4])
        };
    }
}

void spaLoadServices() {
    ifstream f("spa_services.csv");
    if (!f) return;
    string line, c[5];
    getline(f, line);
    while (getline(f, line) && serviceCount < SPA_MAX_SERVICES) {
        splitCSV(line, c, 5);
        services[serviceCount++] = {
            stoi(c[0]), c[1], stoi(c[2]), stod(c[3])
        };
    }
}

void spaLoadAppointments() {
    ifstream f("spa_appointments.csv");
    if (!f) return;
    string line, c[6];
    getline(f, line);
    while (getline(f, line) && appointmentCount < SPA_MAX_APPOINTMENTS) {
        splitCSV(line, c, 6);
        appointments[appointmentCount++] = {
            stoi(c[0]), stoi(c[1]), stoi(c[2]),
            stoi(c[3]), c[4], c[5]
        };
    }
}
string spaGetCustomerName(int id) {
    for (int i = 0; i < customerCount; i++)
        if (customers[i].id == id)
            return customers[i].name;
    return "Unknown";
}

string spaGetStaffName(int id) {
    for (int i = 0; i < staffCount; i++)
        if (staff[i].id == id)
            return staff[i].name;
    return "Unassigned";
}

string spaGetServiceName(int id) {
    for (int i = 0; i < serviceCount; i++)
        if (services[i].id == id)
            return services[i].name;
    return "Unknown";
}


void spaLoadAll() {
    spaLoadCustomers();
    spaLoadStaff();
    spaLoadServices();
    spaLoadAppointments();

    cout << "\nLoaded:\n";
    cout << customerCount << " customers\n";
    cout << staffCount << " staff\n";
    cout << serviceCount << " services\n";
    cout << appointmentCount << " appointments\n";
}

// ---------------- STAFF MANAGEMENT ----------------
void spaListStaff() {
    cout << "\n--- STAFF LIST ---\n";
    for (int i = 0; i < staffCount; i++) {
        cout << "ID:" << staff[i].id
             << " | " << staff[i].name
             << " | Role:" << staff[i].role
             << " | Spec:" << staff[i].specialty
             << " | " << (staff[i].available ? "Available" : "Busy") << "\n";
    }
}

void spaAddStaff() {
    if (staffCount >= SPA_MAX_STAFF) return;

    SpaStaff &s = staff[staffCount];
    s.id = staffCount + 1;

    cout << "Name: ";
    cin.ignore();
    getline(cin, s.name);
    cout << "Role: ";
    getline(cin, s.role);
    cout << "Specialty: ";
    getline(cin, s.specialty);

    s.available = 1;
    staffCount++;

    cout << "Staff added\n";
}

void spaRemoveStaff() {
    int id;
    cout << "Staff ID to remove: ";
    cin >> id;

    for (int i = 0; i < staffCount; i++) {
        if (staff[i].id == id) {
            staff[i] = staff[staffCount - 1];
            staffCount--;
            cout << "Staff removed\n";
            return;
        }
    }
    cout << "Staff not found\n";
}

// ---------------- APPOINTMENTS ----------------
void spaBookAppointment() {
    SpaAppointment &a = appointments[appointmentCount];
    a.id = appointmentCount + 1;

    cout << "Customer ID: ";
    cin >> a.customerId;
    cout << "Service ID: ";
    cin >> a.serviceId;
    cout << "Time Slot: ";
    cin >> a.timeSlot;

    a.staffId = -1;
    a.status = "Booked";
    appointmentCount++;

    cout << "Appointment booked\n";
}

void spaAssignStaff() {
    int aid;
    cout << "Appointment ID: ";
    cin >> aid;

    for (int i = 0; i < appointmentCount; i++) {
        if (appointments[i].id == aid &&
            appointments[i].status == "Booked") {

            for (int s = 0; s < staffCount; s++) {
                if (staff[s].available &&
                    staff[s].role == "Therapist") {

                    appointments[i].staffId = staff[s].id;
                    appointments[i].status = "InProgress";
                    staff[s].available = 0;

                    cout << "Assigning "
                         << spaGetStaffName(staff[s].id)
                         << " to "
                         << spaGetCustomerName(appointments[i].customerId)
                         << "\n";

                    return;
                }
            }
        }
    }
    cout << "No therapist available\n";
}


void spaCompleteAppointment() {
    int aid;
    cout << "Appointment ID: ";
    cin >> aid;

    for (int i = 0; i < appointmentCount; i++) {
        if (appointments[i].id == aid &&
            appointments[i].status == "InProgress") {

            appointments[i].status = "Completed";

            for (int s = 0; s < staffCount; s++)
                if (staff[s].id == appointments[i].staffId)
                    staff[s].available = 1;

            SpaBill &b = bills[billCount];
            b.id = billCount + 1;
            b.appointmentId = aid;
            b.status = "Pending";

            for (int j = 0; j < serviceCount; j++)
                if (services[j].id == appointments[i].serviceId)
                    b.amount = services[j].price;

            billCount++;
            cout << "Completed & billed\n";
            return;
        }
    }
    cout << "Invalid appointment\n";
}

// ---------------- LIST APPOINTMENTS ----------------
void spaListAppointments() {
    cout << "\n--- APPOINTMENTS ---\n";
    for (int i = 0; i < appointmentCount; i++) {
        cout << "ApptID:" << appointments[i].id
             << " | Customer: " << spaGetCustomerName(appointments[i].customerId)
             << " | Service: " << spaGetServiceName(appointments[i].serviceId)
             << " | Staff: " << spaGetStaffName(appointments[i].staffId)
             << " | [" << appointments[i].status << "]\n";
    }
}


// ---------------- MENU ----------------
void spaMenu() {
    int ch;
    do {
        cout << "\n==== SPA MANAGEMENT ====\n";
        cout << "1. Load ALL CSVs\n";
        cout << "2. List Staff\n";
        cout << "3. Add Staff\n";
        cout << "4. Remove Staff\n";
        cout << "5. Book Appointment\n";
        cout << "6. Assign Therapist\n";
        cout << "7. Complete Appointment & Bill\n";
        cout << "8. List Appointments\n";
        cout << "0. Exit\n";
        cout << "Choice: ";
        cin >> ch;

        switch (ch) {
            case 1: spaLoadAll(); break;
            case 2: spaListStaff(); break;
            case 3: spaAddStaff(); break;
            case 4: spaRemoveStaff(); break;
            case 5: spaBookAppointment(); break;
            case 6: spaAssignStaff(); break;
            case 7: spaCompleteAppointment(); break;
            case 8: spaListAppointments(); break;
        }
    } while (ch != 0);
}

// ---------------- MAIN ----------------
int main() {
    spaMenu();
    return 0;
}

