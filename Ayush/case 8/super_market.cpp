// supermarket_management.cpp
// Standalone Supermarket Management module (CSV-backed)

#include <bits/stdc++.h>
using namespace std;

/* ------------------------------------------------------------------
   SHARED HELPERS (CSV + TIME)
------------------------------------------------------------------ */

vector<vector<string>> readCSV(const string &fname) {
    vector<vector<string>> rows;
    ifstream in(fname);
    if (!in.is_open()) return rows;
    string line;
    while (getline(in, line)) {
        vector<string> cols;
        string cur; bool inquote = false;
        for (char c : line) {
            if (c=='"') inquote = !inquote;
            else if (c==',' && !inquote) {
                cols.push_back(cur); cur.clear();
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
    for (size_t i=0;i<row.size();i++) {
        string c = row[i];
        if (c.find(',')!=string::npos) out<<"\""<<c<<"\"";
        else out<<c;
        if (i+1<row.size()) out<<",";
    }
    out<<"\n";
    out.close();
}

void overwriteCSV(const string &fname,const vector<vector<string>>&rows){
    ofstream out(fname,ios::trunc);
    for (auto &r:rows){
        for(size_t i=0;i<r.size();i++){
            if (r[i].find(',')!=string::npos) out<<"\""<<r[i]<<"\"";
            else out<<r[i];
            if(i+1<r.size()) out<<",";
        }
        out<<"\n";
    }
    out.close();
}

string nowDate(){
    time_t t=time(nullptr);
    tm *x=localtime(&t);
    char b[20]; strftime(b,20,"%Y-%m-%d",x);
    return b;
}

/* ------------------------------------------------------------------
   FILE NAMES
------------------------------------------------------------------ */

const string SM_PRODUCT_FILE = "supermarket_products.csv";
const string SM_SALES_FILE   = "supermarket_sales.csv";

/* ------------------------------------------------------------------
   INVENTORY MANAGEMENT
------------------------------------------------------------------ */

void sm_addProduct() {
    string id,name,price,stock,category;
    cout<<"Product ID: "; getline(cin,id);
    cout<<"Name: "; getline(cin,name);
    cout<<"Category: "; getline(cin,category);
    cout<<"Price: "; getline(cin,price);
    cout<<"Stock: "; getline(cin,stock);

    appendCSV(SM_PRODUCT_FILE,{id,name,category,price,stock});
    cout<<"Product added.\n";
}

void sm_viewProducts() {
    auto rows = readCSV(SM_PRODUCT_FILE);
    cout<<"\n=== PRODUCTS ===\n";
    for(size_t i=1;i<rows.size();i++){
        cout<<rows[i][0]<<" | "<<rows[i][1]<<" | "<<rows[i][2]
            <<" | Rs."<<rows[i][3]<<" | Stock:"<<rows[i][4]<<"\n";
    }
}

void sm_updateStock() {
    auto rows = readCSV(SM_PRODUCT_FILE);
    cout<<"Enter Product ID: ";
    string pid; getline(cin,pid);

    cout<<"Add/Subtract stock (e.g. 10 or -5): ";
    string s; getline(cin,s);
    int delta = stoi(s);

    bool ok=false;
    for(size_t i=1;i<rows.size();i++){
        if(rows[i][0]==pid){
            int st = stoi(rows[i][4]);
            rows[i][4] = to_string(st + delta);
            ok=true; break;
        }
    }

    if(ok){
        overwriteCSV(SM_PRODUCT_FILE, rows);
        cout<<"Stock updated.\n";
    } else cout<<"Product not found.\n";
}

void sm_searchProduct() {
    cout<<"Enter name to search: ";
    string q; getline(cin,q);
    string low=q; transform(low.begin(),low.end(),low.begin(),::tolower);

    auto rows = readCSV(SM_PRODUCT_FILE);
    cout<<"\nResults:\n";
    bool found=false;
    for(size_t i=1;i<rows.size();i++){
        string nm=rows[i][1];
        string tmp=nm; transform(tmp.begin(),tmp.end(),tmp.begin(),::tolower);
        if(tmp.find(low)!=string::npos){
            cout<<rows[i][0]<<" | "<<nm<<" | Rs."<<rows[i][3]<<"\n";
            found=true;
        }
    }
    if(!found) cout<<"No match.\n";
}

/* ------------------------------------------------------------------
   BILLING SYSTEM
------------------------------------------------------------------ */

void sm_newSale() {
    auto prod = readCSV(SM_PRODUCT_FILE);
    if(prod.size()<=1){
        cout<<"No products available.\n"; return;
    }

    vector<pair<string,int>> cart;   // product_id, qty
    while(true){
        cout<<"\n1.Add Item  2.Checkout  3.Cancel\nChoice: ";
        string c; getline(cin,c);

        if(c=="1"){
            cout<<"Product ID: ";
            string pid; getline(cin,pid);
            int price=0, stock=0; bool ok=false;
            for(size_t i=1;i<prod.size();i++){
                if(prod[i][0]==pid){
                    stock = stoi(prod[i][4]);
                    price = stoi(prod[i][3]);
                    ok=true;
                    break;
                }
            }
            if(!ok){ cout<<"Invalid ID.\n"; continue; }

            cout<<"Quantity: ";
            string qq; getline(cin,qq);
            int q = stoi(qq);
            if(q>stock){
                cout<<"Not enough stock.\n"; continue;
            }
            cart.push_back({pid,q});
            cout<<"Added.\n";
        }
        else if(c=="2"){
            if(cart.empty()){
                cout<<"Cart empty.\n"; continue;
            }
            int total=0;
            for(auto &x:cart){
                string pid=x.first; int q=x.second;
                for(size_t i=1;i<prod.size();i++){
                    if(prod[i][0]==pid){
                        int price=stoi(prod[i][3]);
                        total += price*q;
                        int st=stoi(prod[i][4]);
                        prod[i][4] = to_string(st-q);
                    }
                }
            }

            string sid="S"+to_string(rand()%9999+1000);
            appendCSV(SM_SALES_FILE,{sid,to_string(total),nowDate()});
            overwriteCSV(SM_PRODUCT_FILE, prod);

            cout<<"Checkout complete. Bill = Rs."<<total<<"\n";
            return;
        }
        else if(c=="3"){
            cout<<"Sale cancelled.\n"; return;
        }
        else cout<<"Invalid.\n";
    }
}

void sm_viewSales() {
    auto rows = readCSV(SM_SALES_FILE);
    cout<<"\n=== SALES HISTORY ===\n";
    for(size_t i=1;i<rows.size();i++){
        cout<<rows[i][0]<<" | Rs."<<rows[i][1]<<" | "<<rows[i][2]<<"\n";
    }
}

void sm_totalSalesOfDay() {
    cout<<"Enter date (YYYY-MM-DD): ";
    string d; getline(cin,d);
    auto rows = readCSV(SM_SALES_FILE);
    int sum=0;
    for(size_t i=1;i<rows.size();i++){
        if(rows[i][2]==d) sum += stoi(rows[i][1]);
    }
    cout<<"Total = Rs."<<sum<<"\n";
}

/* ------------------------------------------------------------------
   SAMPLE DATA LOADER
------------------------------------------------------------------ */

void sm_loadSampleData() {
    vector<vector<string>> products = {
        {"id","name","category","price","stock"},
        {"P01","Rice 1kg","Grocery","55","120"},
        {"P02","Wheat Flour 1kg","Grocery","48","150"},
        {"P03","Milk 1L","Dairy","52","80"},
        {"P04","Bread","Bakery","35","60"},
        {"P05","Eggs 12pc","Poultry","72","200"},
        {"P06","Sugar 1kg","Grocery","44","100"},
        {"P07","Cola 1L","Beverage","40","90"},
        {"P08","Soap","Personal Care","32","140"},
        {"P09","Shampoo","Personal Care","120","70"},
        {"P10","Tea 500g","Grocery","140","50"}
    };
    overwriteCSV(SM_PRODUCT_FILE, products);

    vector<vector<string>> sales = {
        {"sale_id","amount","date"},
        {"S101","540","2025-01-01"},
        {"S102","320","2025-01-01"},
        {"S103","410","2025-01-02"}
    };
    overwriteCSV(SM_SALES_FILE, sales);

    cout<<"Sample supermarket data loaded.\n";
}

/* ------------------------------------------------------------------
   MAIN MENU
------------------------------------------------------------------ */

void sm_mainMenu() {
    while(true){
        cout<<"\n=== SUPERMARKET MANAGEMENT ===\n";
        cout<<"1.Products\n2.Stock Update\n3.Search Product\n4.New Sale\n";
        cout<<"5.View Sales\n6.Total Sales of Day\n7.Load Sample Data\n8.Exit\n";
        cout<<"Choice: ";
        string c; getline(cin,c);

        if(c=="1") sm_viewProducts();
        else if(c=="2") sm_updateStock();
        else if(c=="3") sm_searchProduct();
        else if(c=="4") sm_newSale();
        else if(c=="5") sm_viewSales();
        else if(c=="6") sm_totalSalesOfDay();
        else if(c=="7") sm_loadSampleData();
        else if(c=="8") break;
        else cout<<"Invalid.\n";
    }
}

int main() {
    srand(time(NULL));
    sm_mainMenu();
    return 0;
}
