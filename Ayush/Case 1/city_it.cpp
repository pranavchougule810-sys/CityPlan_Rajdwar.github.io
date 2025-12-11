// main.cpp
// Single-file IT Office Management System (CSV backed, menu-driven)
// Final version: Option 2 menu + Option 9 = Load Sample Data (overwrites when confirmed)
// Sample data embedded (20 rows per CSV). Choose to load from menu.
// Compile: g++ -std=gnu++17 main.cpp -o it_office

#include <bits/stdc++.h>
using namespace std;

// ---------- CSV helpers ----------

vector<vector<string>> readCSV(const string &fname) {
    vector<vector<string>> rows;
    ifstream in(fname);
    if (!in.is_open()) return rows;
    string line;
    while (getline(in, line)) {
        vector<string> cols;
        string cur;
        bool inquote = false;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == '"') inquote = !inquote;
            else if (c == ',' && !inquote) {
                cols.push_back(cur);
                cur.clear();
            } else cur.push_back(c);
        }
        cols.push_back(cur);
        rows.push_back(cols);
    }
    in.close();
    return rows;
}

void appendCSV(const string &fname, const vector<string> &row) {
    ofstream out(fname, ios::app);
    if (!out.is_open()) out.open(fname);
    for (size_t i = 0; i < row.size(); ++i) {
        string cell = row[i];
        if (cell.find(',') != string::npos) out << '"' << cell << '"';
        else out << cell;
        if (i + 1 < row.size()) out << ',';
    }
    out << '\n';
    out.close();
}

void overwriteCSV(const string &fname, const vector<vector<string>> &rows) {
    ofstream out(fname, ios::trunc);
    for (auto &r : rows) {
        for (size_t i = 0; i < r.size(); ++i) {
            if (r[i].find(',') != string::npos) out << '"' << r[i] << '"';
            else out << r[i];
            if (i + 1 < r.size()) out << ',';
        }
        out << '\n';
    }
    out.close();
}

// ---------- Time helpers ----------
string nowDate() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local = *localtime(&tt);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &local);
    return string(buf);
}
string nowTimeHHMM() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm local = *localtime(&tt);
    char buf[10];
    strftime(buf, sizeof(buf), "%H:%M", &local);
    return string(buf);
}
int timeToMinutes(const string &hhmm) {
    int h = 0, m = 0;
    sscanf(hhmm.c_str(), "%d:%d", &h, &m);
    return h * 60 + m;
}
string minutesToHHMM(int mins) {
    mins = (mins % (24*60) + 24*60) % (24*60);
    int h = mins / 60;
    int m = mins % 60;
    char buf[6];
    sprintf(buf, "%02d:%02d", h, m);
    return string(buf);
}

// ---------- simple merge sort for CSV rows by column ----------
void mergeByCol(vector<vector<string>> &a, int l, int m, int r, int col) {
    int n1 = m - l + 1, n2 = r - m;
    vector<vector<string>> L(n1), R(n2);
    for (int i = 0; i < n1; ++i) L[i] = a[l + i];
    for (int j = 0; j < n2; ++j) R[j] = a[m + 1 + j];
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        string lv = (col < (int)L[i].size() ? L[i][col] : "");
        string rv = (col < (int)R[j].size() ? R[j][col] : "");
        bool ln = !lv.empty() && all_of(lv.begin(), lv.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; });
        bool rn = !rv.empty() && all_of(rv.begin(), rv.end(), [](char c){ return isdigit(c) || c=='-' || c=='.'; });
        bool takeLeft;
        if (ln && rn) takeLeft = stod(lv) <= stod(rv);
        else takeLeft = lv <= rv;
        if (takeLeft) a[k++] = L[i++]; else a[k++] = R[j++];
    }
    while (i < n1) a[k++] = L[i++];
    while (j < n2) a[k++] = R[j++];
}
void mergeSortByCol(vector<vector<string>> &a, int l, int r, int col) {
    if (l >= r) return;
    int m = (l + r) / 2;
    mergeSortByCol(a, l, m, col);
    mergeSortByCol(a, m+1, r, col);
    mergeByCol(a, l, m, r, col);
}

// ---------- File names ----------
const string EMP_FILE = "employees.csv";
const string ATT_FILE = "attendance.csv";
const string SAL_FILE = "salary.csv";
const string TASK_FILE = "tasks.csv";
const string ISSUE_FILE = "issues.csv";
const string INV_FILE = "inventory.csv";
const string MEET_FILE = "meetings.csv";

// ---------- init functions (do nothing, CSVs managed externally or by sample loader) ----------
void initEmployee() { }
void initAttendance() { }
void initSalary() { }
void initTasks() { }
void initIssues() { }
void initInv() { }
void initMeet() { }

// ---------- Employee module ----------
void addEmployee() {
    string id,name,age,dept,desig;
    cout << "Enter ID: "; getline(cin, id);
    cout << "Enter Name: "; getline(cin, name);
    cout << "Enter Age: "; getline(cin, age);
    cout << "Enter Department: "; getline(cin, dept);
    cout << "Enter Designation: "; getline(cin, desig);
    appendCSV(EMP_FILE, {id,name,age,dept,desig});
    cout << "Employee added.\n";
}
void viewEmployees() {
    auto rows = readCSV(EMP_FILE);
    if (rows.size() <= 1) { cout << "No employees.\n"; return; }
    cout << "Employees:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        auto &r = rows[i];
        cout << r[0] << " | " << r[1] << " | " << (r.size()>2?r[2]:"") << " | " << (r.size()>3?r[3]:"") << " | " << (r.size()>4?r[4]:"") << "\n";
    }
}
void searchEmployee() {
    auto rows = readCSV(EMP_FILE);
    cout << "Enter search term (id or name): ";
    string term; getline(cin, term);
    string low = term; transform(low.begin(), low.end(), low.begin(), ::tolower);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        for (auto &cell : rows[i]) {
            string tmp = cell; transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
            if (tmp.find(low) != string::npos) {
                cout << "Found: ";
                for (auto &c : rows[i]) cout << c << " ";
                cout << "\n";
                found = true; break;
            }
        }
    }
    if (!found) cout << "No match found.\n";
}
void sortEmployees() {
    auto rows = readCSV(EMP_FILE);
    if (rows.size() <= 1) { cout << "Nothing to sort.\n"; return; }
    cout << "Sort by column: 0:id 1:name 2:age 3:dept 4:desig\nChoice: ";
    string s; getline(cin, s);
    int col = 0; try { col = stoi(s); } catch(...) { col = 0; }
    vector<vector<string>> body(rows.begin()+1, rows.end());
    mergeSortByCol(body, 0, (int)body.size()-1, col);
    vector<vector<string>> out; out.push_back(rows[0]); out.insert(out.end(), body.begin(), body.end());
    overwriteCSV(EMP_FILE, out);
    cout << "Sorted saved.\n";
}
void deleteEmployee() {
    auto rows = readCSV(EMP_FILE);
    cout << "Enter ID to delete: ";
    string id; getline(cin, id);
    vector<vector<string>> out; if (!rows.empty()) out.push_back(rows[0]);
    bool removed = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i][0] == id) removed = true; else out.push_back(rows[i]);
    }
    if (removed) { overwriteCSV(EMP_FILE, out); cout << "Deleted.\n"; }
    else cout << "ID not found.\n";
}

// ---------- Attendance ----------
void markAttendance() {
    cout << "Enter Employee ID: "; string id; getline(cin, id);
    cout << "1 - Mark IN\n2 - Mark OUT\nChoice: "; string c; getline(cin, c);
    string date = nowDate(); string time = nowTimeHHMM();
    auto rows = readCSV(ATT_FILE);
    if (c == "1") {
        appendCSV(ATT_FILE, {id,date,time,""});
        cout << "IN marked at " << time << " on " << date << "\n";
    } else {
        bool done = false;
        for (int i = (int)rows.size()-1; i >= 1; --i) {
            if (rows[i].size() >= 4 && rows[i][0] == id && rows[i][1] == date && rows[i][3].empty()) {
                rows[i][3] = time; done = true; break;
            }
        }
        if (done) { overwriteCSV(ATT_FILE, rows); cout << "OUT marked at " << time << "\n"; }
        else cout << "No matching IN for today.\n";
    }
}
void viewAttendance() {
    auto rows = readCSV(ATT_FILE);
    cout << "Attendance:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | " << rows[i][1] << " | IN:" << rows[i][2] << " | OUT:" << (rows[i].size()>3?rows[i][3]:"") << "\n";
    }
}
void calcHours() {
    auto rows = readCSV(ATT_FILE);
    cout << "Enter Employee ID: "; string id; getline(cin, id);
    cout << "Enter Month (YYYY-MM): "; string month; getline(cin, month);
    int totalMin = 0;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 4 && rows[i][0] == id && rows[i][1].rfind(month, 0) == 0) {
            string inT = rows[i][2], outT = rows[i][3];
            if (!inT.empty() && !outT.empty()) {
                int mins = timeToMinutes(outT) - timeToMinutes(inT);
                if (mins < 0) mins += 24*60;
                totalMin += max(0, mins);
            }
        }
    }
    double hours = totalMin / 60.0;
    cout << "Total hours in " << month << " = " << fixed << setprecision(2) << hours << "\n";
}

// ---------- Salary ----------
void computeSalary() {
    cout << "Enter Employee ID: "; string id; getline(cin, id);
    cout << "Enter Month (YYYY-MM): "; string month; getline(cin, month);
    cout << "Enter Rate per hour: "; string rstr; getline(cin, rstr);
    double rate = 0.0; try { rate = stod(rstr); } catch(...) { rate = 0.0; }
    auto rows = readCSV(ATT_FILE);
    int totalMin = 0;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 4 && rows[i][0] == id && rows[i][1].rfind(month, 0) == 0) {
            string inT = rows[i][2], outT = rows[i][3];
            if (!inT.empty() && !outT.empty()) {
                int mins = timeToMinutes(outT) - timeToMinutes(inT);
                if (mins < 0) mins += 24*60;
                totalMin += max(0, mins);
            }
        }
    }
    double hours = totalMin / 60.0;
    double total = hours * rate;
    appendCSV(SAL_FILE, {id, month, to_string(hours), to_string(rate), to_string(total)});
    cout << "Salary computed. Total = " << total << "\n";
}
void viewSalaries() {
    auto rows = readCSV(SAL_FILE);
    cout << "Salary Records:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | " << rows[i][1] << " | Hours:" << rows[i][2] << " | Rate:" << rows[i][3] << " | Total:" << rows[i][4] << "\n";
    }
}

// ---------- Tasks ----------
void addTask() {
    string tid,eid,title,pri;
    cout << "Task ID: "; getline(cin, tid);
    cout << "Assign to Employee ID: "; getline(cin, eid);
    cout << "Title: "; getline(cin, title);
    cout << "Priority (1-high,2-med,3-low): "; getline(cin, pri);
    appendCSV(TASK_FILE, {tid,eid,title,pri,"open"});
    cout << "Task added.\n";
}
void viewTasks() {
    auto rows = readCSV(TASK_FILE);
    cout << "Tasks:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | Emp:" << rows[i][1] << " | " << rows[i][2] << " | Pri:" << rows[i][3] << " | " << rows[i][4] << "\n";
    }
}
void assignTopTask() {
    auto rows = readCSV(TASK_FILE);
    int best = -1; int bestPri = INT_MAX;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 5 && rows[i][4] != "closed") {
            int p = 999; try { p = stoi(rows[i][3]); } catch(...) { p = 999; }
            if (p < bestPri) { bestPri = p; best = i; }
        }
    }
    if (best == -1) { cout << "No open tasks.\n"; return; }
    cout << "Top task: " << rows[best][0] << " priority " << rows[best][3] << ". Enter Employee ID to assign: ";
    string eid; getline(cin, eid);
    rows[best][1] = eid;
    overwriteCSV(TASK_FILE, rows);
    cout << "Assigned.\n";
}

// ---------- Issues / Tickets ----------
void raiseTicket() {
    string tid,cat,sev;
    cout << "Ticket ID: "; getline(cin, tid);
    cout << "Category: "; getline(cin, cat);
    cout << "Severity (1-high): "; getline(cin, sev);
    appendCSV(ISSUE_FILE, {tid,cat,sev,"open",""});
    cout << "Ticket raised.\n";
}
void viewTickets() {
    auto rows = readCSV(ISSUE_FILE);
    cout << "Tickets:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | " << rows[i][1] << " | Sev:" << rows[i][2] << " | " << rows[i][3] << " | Assigned:" << (rows[i].size()>4?rows[i][4]:"") << "\n";
    }
}
void prioritizeTicket() {
    auto rows = readCSV(ISSUE_FILE);
    int best = -1, bestSev = INT_MAX;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 4 && rows[i][3] != "closed") {
            int s = 999; try { s = stoi(rows[i][2]); } catch(...) { s = 999; }
            if (s < bestSev) { bestSev = s; best = i; }
        }
    }
    if (best == -1) { cout << "No open tickets.\n"; return; }
    cout << "Assign ticket " << rows[best][0] << " to Employee ID: ";
    string eid; getline(cin, eid);
    if (rows[best].size() < 5) rows[best].resize(5);
    rows[best][4] = eid;
    overwriteCSV(ISSUE_FILE, rows);
    cout << "Assigned.\n";
}
void closeTicket() {
    auto rows = readCSV(ISSUE_FILE);
    cout << "Enter Ticket ID to close: "; string tid; getline(cin, tid);
    bool ok = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i][0] == tid) { if (rows[i].size() < 4) rows[i].resize(4); rows[i][3] = "closed"; ok = true; }
    }
    if (ok) { overwriteCSV(ISSUE_FILE, rows); cout << "Closed.\n"; } else cout << "Not found.\n";
}

// ---------- Inventory ----------
void addItem() {
    string id,type,notes;
    cout << "Item ID: "; getline(cin, id);
    cout << "Type: "; getline(cin, type);
    cout << "Notes: "; getline(cin, notes);
    appendCSV(INV_FILE, {id,type,"","available",notes});
    cout << "Item added.\n";
}
void assignItem() {
    auto rows = readCSV(INV_FILE);
    cout << "Enter Item ID to assign: "; string iid; getline(cin, iid);
    cout << "Enter Employee ID: "; string eid; getline(cin, eid);
    bool found = false;
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i][0] == iid) { if (rows[i].size() < 4) rows[i].resize(4); rows[i][2] = eid; rows[i][3] = "assigned"; found = true; }
    }
    if (found) { overwriteCSV(INV_FILE, rows); cout << "Assigned.\n"; } else cout << "Not found.\n";
}
void viewInventory() {
    auto rows = readCSV(INV_FILE);
    cout << "Inventory:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | " << rows[i][1] << " | Assigned:" << (rows[i].size()>2?rows[i][2]:"") << " | " << (rows[i].size()>3?rows[i][3]:"") << "\n";
    }
}

// ---------- Meetings ----------
bool conflictExists(const string &date, const string &st, const string &et, const string &room) {
    auto rows = readCSV(MEET_FILE);
    int s = timeToMinutes(st), e = timeToMinutes(et);
    for (size_t i = 1; i < rows.size(); ++i) {
        if (rows[i].size() >= 5 && rows[i][1] == date && rows[i][4] == room) {
            int ss = timeToMinutes(rows[i][2]), ee = timeToMinutes(rows[i][3]);
            if (max(ss, s) < min(ee, e)) return true;
        }
    }
    return false;
}
void scheduleMeeting() {
    string mid,date,st,et,room,title;
    cout << "Meeting ID: "; getline(cin, mid);
    cout << "Date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Start (HH:MM): "; getline(cin, st);
    cout << "End (HH:MM): "; getline(cin, et);
    cout << "Room: "; getline(cin, room);
    cout << "Title: "; getline(cin, title);
    int s = timeToMinutes(st), e = timeToMinutes(et);
    int dur = e - s;
    if (dur <= 0) { cout << "Invalid duration.\n"; return; }
    string curS = st, curE = et;
    int attempts = 0;
    while (attempts < 48) {
        if (!conflictExists(date, curS, curE, room)) {
            appendCSV(MEET_FILE, {mid,date,curS,curE,room,title});
            cout << "Scheduled at " << curS << " - " << curE << "\n";
            return;
        }
        int ns = timeToMinutes(curS) + 30;
        int ne = ns + dur;
        if (ne > 24*60) break;
        curS = minutesToHHMM(ns); curE = minutesToHHMM(ne);
        attempts++;
    }
    cout << "No free slot found on that date in that room.\n";
}
void viewMeetings() {
    auto rows = readCSV(MEET_FILE);
    cout << "Meetings:\n";
    for (size_t i = 1; i < rows.size(); ++i) {
        cout << rows[i][0] << " | " << rows[i][1] << " | " << rows[i][2] << "-" << rows[i][3] << " | Room:" << rows[i][4] << " | " << (rows[i].size()>5?rows[i][5]:"") << "\n";
    }
}

// ---------- Sample data loader (20 rows each) ----------
void loadSampleData() {
    cout << "This will OVERWRITE all CSV files with sample data.\nAre you sure? (yes/no): ";
    string ans; getline(cin, ans);
    string low = ans; transform(low.begin(), low.end(), low.begin(), ::tolower);
    if (low != "yes") { cout << "Aborted. No changes made.\n"; return; }

    // employees (20)
    vector<vector<string>> emp = {
        {"id","name","age","department","designation"},
        {"E01","John","25","IT","Developer"},
        {"E02","Aarav","28","HR","HR Executive"},
        {"E03","Riya","24","Finance","Accountant"},
        {"E04","Karan","30","IT","System Admin"},
        {"E05","Neha","27","Marketing","Content Writer"},
        {"E06","Manoj","32","Operations","Manager"},
        {"E07","Pooja","26","IT","Tester"},
        {"E08","Arjun","29","Design","UI Designer"},
        {"E09","Simran","23","Support","Helpdesk"},
        {"E10","Rahul","31","IT","Team Lead"},
        {"E11","Tara","26","HR","Recruiter"},
        {"E12","Vikram","34","Finance","Senior Accountant"},
        {"E13","Maya","22","Marketing","Intern"},
        {"E14","Sahil","29","IT","DevOps"},
        {"E15","Isha","28","Design","UX Researcher"},
        {"E16","Rohit","33","Operations","Logistics"},
        {"E17","Anita","27","Support","Customer Rep"},
        {"E18","Dev","30","IT","Backend Dev"},
        {"E19","Lina","25","Finance","Analyst"},
        {"E20","Omar","35","Admin","Office Manager"}
    };
    overwriteCSV(EMP_FILE, emp);

    // attendance (20) - dates spread in Jan 2025
    vector<vector<string>> att = {
        {"id","date","in_time","out_time"},
        {"E01","2025-01-02","09:05","17:05"},
        {"E02","2025-01-02","09:10","17:30"},
        {"E03","2025-01-03","09:30","17:20"},
        {"E04","2025-01-03","08:50","17:10"},
        {"E05","2025-01-04","09:05","16:55"},
        {"E06","2025-01-05","09:00","18:00"},
        {"E07","2025-01-06","09:15","17:15"},
        {"E08","2025-01-07","09:00","17:00"},
        {"E09","2025-01-08","09:25","16:55"},
        {"E10","2025-01-09","08:55","17:05"},
        {"E11","2025-01-10","09:00","17:30"},
        {"E12","2025-01-11","09:20","17:10"},
        {"E13","2025-01-12","09:40","16:40"},
        {"E14","2025-01-13","09:00","17:00"},
        {"E15","2025-01-14","09:10","17:20"},
        {"E16","2025-01-15","08:50","17:00"},
        {"E17","2025-01-16","09:05","17:05"},
        {"E18","2025-01-17","09:00","17:00"},
        {"E19","2025-01-18","09:30","17:00"},
        {"E20","2025-01-19","08:45","16:45"}
    };
    overwriteCSV(ATT_FILE, att);

    // salary (20) - sample months/hours/rates
    vector<vector<string>> sal = {
        {"id","month","worked_hours","rate_per_hour","total_salary"},
        {"E01","2025-01","160","100","16000"},
        {"E02","2025-01","168","110","18480"},
        {"E03","2025-01","150","95","14250"},
        {"E04","2025-01","170","105","17850"},
        {"E05","2025-01","140","90","12600"},
        {"E06","2025-01","180","120","21600"},
        {"E07","2025-01","155","85","13175"},
        {"E08","2025-01","160","98","15680"},
        {"E09","2025-01","130","75","9750"},
        {"E10","2025-01","175","115","20125"},
        {"E11","2025-01","150","80","12000"},
        {"E12","2025-01","165","100","16500"},
        {"E13","2025-01","120","60","7200"},
        {"E14","2025-01","170","110","18700"},
        {"E15","2025-01","158","95","15010"},
        {"E16","2025-01","172","105","18060"},
        {"E17","2025-01","145","70","10150"},
        {"E18","2025-01","160","100","16000"},
        {"E19","2025-01","150","88","13200"},
        {"E20","2025-01","168","90","15120"}
    };
    overwriteCSV(SAL_FILE, sal);

    // tasks (20)
    vector<vector<string>> tasks = {
        {"task_id","emp_id","title","priority","status"},
        {"T01","E01","Fix login bug","1","open"},
        {"T02","E04","Server maintenance","1","open"},
        {"T03","E05","Write blog article","3","open"},
        {"T04","E07","Test checkout flow","2","open"},
        {"T05","E10","Optimize DB","1","open"},
        {"T06","E08","Design landing page","2","open"},
        {"T07","E18","API endpoint","1","open"},
        {"T08","E14","CI pipeline","1","open"},
        {"T09","E02","Employee onboarding doc","3","open"},
        {"T10","E12","Monthly report","2","open"},
        {"T11","E15","UX research","2","open"},
        {"T12","E06","Vendor coordination","3","open"},
        {"T13","E09","Support ticket backlog","2","open"},
        {"T14","E11","Recruitment ads","3","open"},
        {"T15","E13","Intern task: tests","3","open"},
        {"T16","E16","Logistics planning","2","open"},
        {"T17","E17","Customer followup","2","open"},
        {"T18","E19","Budget sheet","2","open"},
        {"T19","E20","Office supplies order","3","open"},
        {"T20","E03","Invoice reconciliation","2","open"}
    };
    overwriteCSV(TASK_FILE, tasks);

    // issues (20)
    vector<vector<string>> issues = {
        {"ticket_id","category","severity","status","assigned_to"},
        {"TK01","Network","1","open","E04"},
        {"TK02","Software","2","open","E01"},
        {"TK03","Hardware","1","open","E07"},
        {"TK04","Printer","3","open","E09"},
        {"TK05","Database","1","open","E10"},
        {"TK06","Email","2","open","E02"},
        {"TK07","Payment","1","open","E18"},
        {"TK08","Security","1","open","E14"},
        {"TK09","UI","3","open","E08"},
        {"TK10","Onboarding","2","open","E11"},
        {"TK11","Payroll","1","open","E12"},
        {"TK12","Licenses","3","open","E06"},
        {"TK13","Network","2","open","E04"},
        {"TK14","Hardware","1","open","E20"},
        {"TK15","Software","2","open","E01"},
        {"TK16","Access","1","open","E16"},
        {"TK17","Monitoring","2","open","E14"},
        {"TK18","Backup","1","open","E10"},
        {"TK19","Mobile","3","open","E08"},
        {"TK20","Storage","2","open","E05"}
    };
    overwriteCSV(ISSUE_FILE, issues);

    // inventory (20)
    vector<vector<string>> inv = {
        {"item_id","type","assigned_to","status","notes"},
        {"I01","Laptop","E01","assigned","Dell Latitude 5400"},
        {"I02","Keyboard","","available","Mechanical KB"},
        {"I03","Mouse","E05","assigned","Logitech MX"},
        {"I04","Monitor","E07","assigned","24in IPS"},
        {"I05","Chair","","available","Ergo chair"},
        {"I06","Phone","E09","assigned","Desk phone"},
        {"I07","Dock","E10","assigned","USB-C dock"},
        {"I08","Headset","E03","assigned","Noise-cancel"},
        {"I09","Router","","available","Spare router"},
        {"I10","SSD","E14","assigned","512GB NVMe"},
        {"I11","Webcam","E08","assigned","1080p cam"},
        {"I12","Tablet","E15","assigned","iPad mini"},
        {"I13","Projector","","available","Office projector"},
        {"I14","Scanner","E12","assigned","Flatbed"},
        {"I15","Whiteboard","","available","6x4 ft"},
        {"I16","UPS","E06","assigned","1kVA UPS"},
        {"I17","Switch","","available","8-port switch"},
        {"I18","Microphone","E11","assigned","Condenser mic"},
        {"I19","Cable Kit","","available","Assorted cables"},
        {"I20","Locker","E20","assigned","Personal locker"}
    };
    overwriteCSV(INV_FILE, inv);

    // meetings (20) - dates across Jan 2025
    vector<vector<string>> meet = {
        {"meeting_id","date","start_time","end_time","room","title"},
        {"M01","2025-01-02","10:00","11:00","Room1","Project kickoff"},
        {"M02","2025-01-03","11:30","12:30","Room2","Design review"},
        {"M03","2025-01-04","14:00","15:00","Room1","Client call"},
        {"M04","2025-01-05","09:00","10:00","Room3","Sprint planning"},
        {"M05","2025-01-06","16:00","17:00","Room2","Retrospective"},
        {"M06","2025-01-07","13:00","14:00","Room1","Sales sync"},
        {"M07","2025-01-08","15:00","16:00","Room4","Ops meeting"},
        {"M08","2025-01-09","10:00","11:00","Room2","Marketing plan"},
        {"M09","2025-01-10","11:00","12:00","Room1","Tech grooming"},
        {"M10","2025-01-11","09:30","10:30","Room3","HR policies"},
        {"M11","2025-01-12","14:00","15:00","Room4","Vendor call"},
        {"M12","2025-01-13","16:00","17:00","Room1","Budget review"},
        {"M13","2025-01-14","10:00","11:00","Room2","UX review"},
        {"M14","2025-01-15","11:30","12:30","Room3","Security audit"},
        {"M15","2025-01-16","13:30","14:30","Room1","Product roadmap"},
        {"M16","2025-01-17","15:00","16:00","Room4","Customer feedback"},
        {"M17","2025-01-18","09:00","10:00","Room2","Hiring panel"},
        {"M18","2025-01-19","10:30","11:30","Room1","All-hands prep"},
        {"M19","2025-01-20","14:00","15:00","Room3","Legal review"},
        {"M20","2025-01-21","16:00","17:00","Room2","Deployment plan"}
    };
    overwriteCSV(MEET_FILE, meet);

    cout << "Sample data loaded successfully!\n";
}

// ---------- Menus ----------
void employeeMenu() {
    while (true) {
        cout << "\nEmployee Menu\n1.Add\n2.View\n3.Search\n4.Sort\n5.Delete\n6.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") addEmployee();
        else if (c == "2") viewEmployees();
        else if (c == "3") searchEmployee();
        else if (c == "4") sortEmployees();
        else if (c == "5") deleteEmployee();
        else return;
    }
}
void attendanceMenu() {
    while (true) {
        cout << "\nAttendance Menu\n1.Mark IN/OUT\n2.View\n3.Calculate Hours\n4.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") markAttendance();
        else if (c == "2") viewAttendance();
        else if (c == "3") calcHours();
        else return;
    }
}
void salaryMenu() {
    while (true) {
        cout << "\nSalary Menu\n1.Compute Salary\n2.View Records\n3.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") computeSalary();
        else if (c == "2") viewSalaries();
        else return;
    }
}
void tasksMenu() {
    while (true) {
        cout << "\nTasks Menu\n1.Add Task\n2.View Tasks\n3.Assign Top Priority\n4.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") addTask();
        else if (c == "2") viewTasks();
        else if (c == "3") assignTopTask();
        else return;
    }
}
void ticketMenu() {
    while (true) {
        cout << "\nTickets Menu\n1.Raise Ticket\n2.View Tickets\n3.Prioritize & Assign\n4.Close Ticket\n5.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") raiseTicket();
        else if (c == "2") viewTickets();
        else if (c == "3") prioritizeTicket();
        else if (c == "4") closeTicket();
        else return;
    }
}
void inventoryMenu() {
    while (true) {
        cout << "\nInventory Menu\n1.Add Item\n2.Assign Item\n3.View Inventory\n4.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") addItem();
        else if (c == "2") assignItem();
        else if (c == "3") viewInventory();
        else return;
    }
}
void meetingMenu() {
    while (true) {
        cout << "\nMeeting Menu\n1.Schedule Meeting\n2.View Meetings\n3.Back\nChoice: ";
        string c; getline(cin, c);
        if (c == "1") scheduleMeeting();
        else if (c == "2") viewMeetings();
        else return;
    }
}

// ---------- main ----------
int main() {
    // ensure output visible immediately (helps some terminals)
    setvbuf(stdout, nullptr, _IONBF, 0);

    while (true) {
        cout << "\n=== IT OFFICE MANAGEMENT ===\n";
        cout << "1.Employee\n";
        cout << "2.Attendance\n";
        cout << "3.Salary\n";
        cout << "4.Tasks\n";
        cout << "5.Tickets\n";
        cout << "6.Inventory\n";
        cout << "7.Meetings\n";
        cout << "8.Exit\n";
        cout << "9.Load Sample Data\n";
        cout << "Choice: ";
        cout.flush();

        string ch; getline(cin, ch);

        if (ch == "1") employeeMenu();
        else if (ch == "2") attendanceMenu();
        else if (ch == "3") salaryMenu();
        else if (ch == "4") tasksMenu();
        else if (ch == "5") ticketMenu();
        else if (ch == "6") inventoryMenu();
        else if (ch == "7") meetingMenu();
        else if (ch == "9") loadSampleData();
        else if (ch == "8") { cout << "Exiting...\n"; break; }
        else cout << "Invalid choice.\n";
    }

    return 0;
}