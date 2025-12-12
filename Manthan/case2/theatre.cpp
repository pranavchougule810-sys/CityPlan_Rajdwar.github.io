
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>
#include <cstring> // for strncpy
#include <limits>

using namespace std;

// -------------------- CONFIG (tweak if needed) --------------------
#define THEATRE_MAX_MOVIES 2000
#define THEATRE_MAX_AUDITORIUMS 50
#define THEATRE_MAX_SEAT_ROWS 50
#define THEATRE_MAX_SEAT_COLS 50
#define THEATRE_MAX_SHOWS 5000
#define THEATRE_MAX_BOOKINGS 20000
#define THEATRE_MAX_SNACKS 1000
#define THEATRE_MAX_SNACK_ORDERS 20000
#define THEATRE_MAX_STAFF 2000
#define THEATRE_MAX_MAINT_LOGS 5000
#define THEATRE_HASH_SIZE 32749
#define THEATRE_INF 999999

// -------------------- SIMPLE CSV & UTIL HELPERS --------------------
int toInt(const string &s) {
    try { return stoi(s); } catch(...) { return 0; }
}

int splitCSV(const string &line, string out[], int maxCols) {
    int col = 0;
    string cur = "";
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') { inQuotes = !inQuotes; continue; }
        if (c == ',' && !inQuotes && col < maxCols - 1) {
            out[col++] = cur;
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out[col++] = cur;
    return col;
}

// -------------------- DATA STRUCTS --------------------
// Movie BST (manual pointers)
struct TheatreMovie
{
    int movie_id;
    string title;
    string genre;
    int duration_minutes;
    double rating;
    string language;
    string release_date;
    TheatreMovie *left;
    TheatreMovie *right;
};

// Auditorium (array-based seats)
struct TheatreAuditorium
{
    int aud_id;
    string name;
    int rows;
    int cols;
    int total_seats;
    char seats[THEATRE_MAX_SEAT_ROWS][THEATRE_MAX_SEAT_COLS]; // 'E','B','X'
    string seat_type[THEATRE_MAX_SEAT_ROWS][THEATRE_MAX_SEAT_COLS];
};

// Show (array)
struct TheatreShow
{
    int show_id;
    int movie_id;
    int aud_id;
    string start_datetime; // "YYYY-MM-DD HH:MM"
    string end_datetime;
    int base_price;
    int tickets_sold;
    int revenue;
};

// Booking record (small POD)
struct TheatreBooking
{
    int booking_id;
    int show_id;
    char seat_label[8];
    char customer_name[100];
    char customer_phone[20];
    int price_paid;
    int status; // 0 cancelled, 1 active
    char booking_datetime[32];
};

// booking hash entry
struct TheatreBookingEntry
{
    int key;
    TheatreBooking val;
    bool used;
    bool deleted;
};

// Snack
struct TheatreSnack
{
    int snack_id;
    string name;
    string category;
    int price;
    int prep_time;
};

// Snack order queue item
struct TheatreSnackOrder
{
    int order_id;
    int booking_id;
    char seat_label[8];
    int item_ids[10];
    int item_count;
    int total_price;
    int status; // 0 pending,1 inprogress,2 done
    char order_datetime[32];
};

// staff & maintenance
struct TheatreStaff
{
    int id;
    string name;
    string role;
    int salary;
};
struct TheatreMaint
{
    int id;
    int aud_id;
    string date;
    string task;
    int staff_id;
    int status;
};

// -------------------- MODULE GLOBALS --------------------
static TheatreMovie *theatreMovieRoot = NULL;
static TheatreAuditorium auditoriums[THEATRE_MAX_AUDITORIUMS];
static int theatreAudCount = 0;

static TheatreShow shows[THEATRE_MAX_SHOWS];
static int theatreShowCount = 0;

static TheatreBookingEntry bookingHash[THEATRE_HASH_SIZE];
static int theatreBookingCount = 0;
static int theatreNextBookingId = 50000;

static TheatreSnack snacks[THEATRE_MAX_SNACKS];
static int theatreSnackCount = 0;

static TheatreSnackOrder snackQueueArr[THEATRE_MAX_SNACK_ORDERS];
static int snackQueueFront = 0, snackQueueRear = -1, snackQueueCount = 0;

static TheatreStaff theatreStaff[THEATRE_MAX_STAFF];
static int theatreStaffCount = 0;
static TheatreMaint theatreMaint[THEATRE_MAX_MAINT_LOGS];
static int theatreMaintCount = 0;

// auditorium graph for Dijkstra (matrix)
static int theatreGraphN = 0;
static int theatreCost[THEATRE_MAX_AUDITORIUMS][THEATRE_MAX_AUDITORIUMS];
static int theatre_dijkstra_dist[THEATRE_MAX_AUDITORIUMS];
static int theatre_dijkstra_path[THEATRE_MAX_AUDITORIUMS];
static int theatre_dijkstra_visited[THEATRE_MAX_AUDITORIUMS];

// -------------------- SMALL HELPERS --------------------
static string theatre_now_datetime()
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char buf[64];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
    return string(buf);
}

// -------------------- MOVIE BST FUNCTIONS --------------------
TheatreMovie *theatreCreateMovieNode(int id, const string &title, const string &genre,
                                     int duration, double rating, const string &lang, const string &rdate)
{
    TheatreMovie *m = new TheatreMovie();
    m->movie_id = id;
    m->title = title;
    m->genre = genre;
    m->duration_minutes = duration;
    m->rating = rating;
    m->language = lang;
    m->release_date = rdate;
    m->left = m->right = NULL;
    return m;
}

TheatreMovie *theatreInsertMovieNode(TheatreMovie *root, TheatreMovie *node)
{
    if (!root)
        return node;
    if (node->title < root->title)
        root->left = theatreInsertMovieNode(root->left, node);
    else
        root->right = theatreInsertMovieNode(root->right, node);
    return root;
}

TheatreMovie *theatreFindMovieByTitle(TheatreMovie *root, const string &title)
{
    if (!root)
        return NULL;
    if (title == root->title)
        return root;
    if (title < root->title)
        return theatreFindMovieByTitle(root->left, title);
    return theatreFindMovieByTitle(root->right, title);
}

int theatre_count_movies_recursive(TheatreMovie *r)
{
    if (!r)
        return 0;
    return 1 + theatre_count_movies_recursive(r->left) + theatre_count_movies_recursive(r->right);
}

bool theatreMoviesAtCapacity()
{
    int c = theatre_count_movies_recursive(theatreMovieRoot);
    return c >= THEATRE_MAX_MOVIES;
}

void theatreInorderList(TheatreMovie *root)
{
    if (!root)
        return;
    theatreInorderList(root->left);
    cout << "ID:" << root->movie_id << " | " << root->title << " | " << root->genre
         << " | " << root->duration_minutes << "min | Rating:" << root->rating
         << " | " << root->language << " | " << root->release_date << "\n";
    theatreInorderList(root->right);
}

TheatreMovie *theatreDeleteMovieNode(TheatreMovie *root, const string &title, bool &deleted)
{
    if (!root)
        return NULL;
    if (title < root->title)
        root->left = theatreDeleteMovieNode(root->left, title, deleted);
    else if (title > root->title)
        root->right = theatreDeleteMovieNode(root->right, title, deleted);
    else
    {
        deleted = true;
        if (!root->left)
        {
            TheatreMovie *t = root->right;
            delete root;
            return t;
        }
        else if (!root->right)
        {
            TheatreMovie *t = root->left;
            delete root;
            return t;
        }
        else
        {
            TheatreMovie *succ = root->right;
            while (succ->left)
                succ = succ->left;
            root->title = succ->title;
            root->movie_id = succ->movie_id;
            root->genre = succ->genre;
            root->duration_minutes = succ->duration_minutes;
            root->rating = succ->rating;
            root->language = succ->language;
            root->release_date = succ->release_date;
            root->right = theatreDeleteMovieNode(root->right, succ->title, deleted);
        }
    }
    return root;
}

// -------------------- QUICK SORT for indexes (exact style) --------------------
// We'll sort an index array referencing shows[] by comparator that returns negative/0/positive
typedef int (*IndexCmpFunc)(int Aidx, int Bidx);

int theatre_partition_index(int arr[], int l, int r, IndexCmpFunc cmp)
{
    // pivot = arr[l]
    int pivotIndex = arr[l];
    int i = l;
    int j = r + 1;
    while (true)
    {
        // i++
        do
        {
            i++;
        } while (i <= r && cmp(arr[i], pivotIndex) < 0);
        // j--
        do
        {
            j--;
        } while (j >= l && cmp(arr[j], pivotIndex) > 0);
        if (i >= j)
            break;
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
    // swap arr[l] and arr[j]
    int tmp = arr[l];
    arr[l] = arr[j];
    arr[j] = tmp;
    return j;
}

void theatre_quickSort_indices(int arr[], int l, int r, IndexCmpFunc cmp)
{
    if (l < r)
    {
        int s = theatre_partition_index(arr, l, r, cmp);
        theatre_quickSort_indices(arr, l, s - 1, cmp);
        theatre_quickSort_indices(arr, s + 1, r, cmp);
    }
}

// comparator for shows by start_datetime (lexicographic compares works for YYYY-MM-DD HH:MM)
int theatre_cmp_show_start(int Aidx, int Bidx)
{
    string A = shows[Aidx].start_datetime;
    string B = shows[Bidx].start_datetime;
    if (A < B)
        return -1;
    if (A > B)
        return 1;
    return 0;
}

// comparator for shows by revenue (desc)
int theatre_cmp_show_revenue(int Aidx, int Bidx)
{
    if (shows[Aidx].revenue > shows[Bidx].revenue)
        return -1;
    if (shows[Aidx].revenue < shows[Bidx].revenue)
        return 1;
    return 0;
}

// -------------------- BOYER–MOORE (bad char) --------------------
void theatre_bm_build_badchar(const string &pat, int badchar[256])
{
    int m = (int)pat.size();
    for (int i = 0; i < 256; ++i)
        badchar[i] = -1;
    for (int i = 0; i < m; ++i)
        badchar[(unsigned char)pat[i]] = i;
}
bool theatre_boyer_moore_search(const string &txt, const string &pat)
{
    if (pat.empty())
        return true;
    int n = (int)txt.size(), m = (int)pat.size();
    if (n < m)
        return false;
    int badchar[256];
    theatre_bm_build_badchar(pat, badchar);
    int s = 0;
    while (s <= n - m)
    {
        int j = m - 1;
        while (j >= 0 && pat[j] == txt[s + j])
            j--;
        if (j < 0)
            return true;
        else
        {
            int bc = badchar[(unsigned char)txt[s + j]];
            int shift = j - bc;
            if (shift < 1)
                shift = 1;
            s += shift;
        }
    }
    return false;
}

// -------------------- KMP (reuse style) --------------------
int *theatre_kmp_prefix_alloc(const string &pat)
{
    int m = (int)pat.size();
    int *lps = new int[m];
    if (m == 0)
        return lps;
    lps[0] = 0;
    int j = 0;
    for (int i = 1; i < m; ++i)
    {
        while (j > 0 && pat[i] != pat[j])
            j = lps[j - 1];
        if (pat[i] == pat[j])
            j++;
        lps[i] = j;
    }
    return lps;
}
bool theatre_kmp_search(const string &text, const string &pat)
{
    if (pat.empty())
        return true;
    if (text.size() < pat.size())
        return false;
    int *lps = theatre_kmp_prefix_alloc(pat);
    int i = 0, j = 0;
    int n = (int)text.size(), m = (int)pat.size();
    while (i < n)
    {
        if (text[i] == pat[j])
        {
            i++;
            j++;
        }
        else
        {
            if (j > 0)
                j = lps[j - 1];
            else
                i++;
        }
        if (j == m)
        {
            delete[] lps;
            return true;
        }
    }
    delete[] lps;
    return false;
}

// -------------------- BOOKING HASH (open addressing linear probing) --------------------
unsigned int theatre_hash_key(int key)
{
    return (unsigned int)(key * 2654435761u) % THEATRE_HASH_SIZE;
}
void theatre_init_booking_hash()
{
    for (int i = 0; i < THEATRE_HASH_SIZE; ++i)
    {
        bookingHash[i].used = false;
        bookingHash[i].deleted = false;
        bookingHash[i].key = -1;
    }
}
bool theatre_booking_insert(const TheatreBooking &b)
{
    if (theatreBookingCount >= THEATRE_MAX_BOOKINGS)
    {
        cout << "Overflow: booking capacity reached!\n";
        return false;
    }
    unsigned int idx = theatre_hash_key(b.booking_id);
    unsigned int start = idx;
    while (bookingHash[idx].used && !bookingHash[idx].deleted && bookingHash[idx].key != b.booking_id)
    {
        idx = (idx + 1) % THEATRE_HASH_SIZE;
        if (idx == start)
        {
            cout << "Hash table full!\n";
            return false;
        }
    }
    bookingHash[idx].used = true;
    bookingHash[idx].deleted = false;
    bookingHash[idx].key = b.booking_id;
    bookingHash[idx].val = b;
    theatreBookingCount++;
    return true;
}
bool theatre_booking_get(int key, TheatreBooking &out)
{
    unsigned int idx = theatre_hash_key(key);
    unsigned int start = idx;
    while (bookingHash[idx].used)
    {
        if (!bookingHash[idx].deleted && bookingHash[idx].key == key)
        {
            out = bookingHash[idx].val;
            return true;
        }
        idx = (idx + 1) % THEATRE_HASH_SIZE;
        if (idx == start)
            break;
    }
    return false;
}
bool theatre_booking_remove(int key)
{
    unsigned int idx = theatre_hash_key(key);
    unsigned int start = idx;
    while (bookingHash[idx].used)
    {
        if (!bookingHash[idx].deleted && bookingHash[idx].key == key)
        {
            bookingHash[idx].deleted = true;
            theatreBookingCount--;
            return true;
        }
        idx = (idx + 1) % THEATRE_HASH_SIZE;
        if (idx == start)
            break;
    }
    return false;
}

// -------------------- SNACK QUEUE --------------------
void theatre_init_snack_queue()
{
    snackQueueFront = 0;
    snackQueueRear = -1;
    snackQueueCount = 0;
}
bool theatre_enqueue_snack(const TheatreSnackOrder &ord)
{
    if (snackQueueCount >= THEATRE_MAX_SNACK_ORDERS)
    {
        cout << "Overflow: snack queue full!\n";
        return false;
    }
    snackQueueRear = (snackQueueRear + 1) % THEATRE_MAX_SNACK_ORDERS;
    snackQueueArr[snackQueueRear] = ord;
    snackQueueCount++;
    return true;
}
TheatreSnackOrder theatre_dequeue_snack()
{
    TheatreSnackOrder empty;
    empty.order_id = -1;
    if (snackQueueCount == 0)
        return empty;
    TheatreSnackOrder res = snackQueueArr[snackQueueFront];
    snackQueueFront = (snackQueueFront + 1) % THEATRE_MAX_SNACK_ORDERS;
    snackQueueCount--;
    return res;
}

// -------------------- DIJKSTRA (matrix style exactly like yours) --------------------
void theatre_build_graph()
{
    theatreGraphN = theatreAudCount;
    for (int i = 0; i < theatreGraphN; ++i)
        for (int j = 0; j < theatreGraphN; ++j)
            theatreCost[i][j] = THEATRE_INF;
    for (int i = 0; i < theatreGraphN; ++i)
        theatreCost[i][i] = 0;
}
void theatre_add_edge_undirected(int a, int b, int w)
{
    if (a < 0 || b < 0 || a >= theatreGraphN || b >= theatreGraphN)
        return;
    theatreCost[a][b] = w;
    theatreCost[b][a] = w;
}
void theatre_dijkstra(int n, int src)
{
    for (int i = 0; i < n; ++i)
    {
        theatre_dijkstra_dist[i] = theatreCost[src][i];
        theatre_dijkstra_path[i] = src;
        theatre_dijkstra_visited[i] = 0;
    }
    theatre_dijkstra_visited[src] = 1;
    for (int iter = 0; iter < n - 1; ++iter)
    {
        int u = -1;
        int minv = THEATRE_INF + 1;
        for (int i = 0; i < n; ++i)
        {
            if (!theatre_dijkstra_visited[i] && theatre_dijkstra_dist[i] < minv)
            {
                minv = theatre_dijkstra_dist[i];
                u = i;
            }
        }
        if (u == -1)
            return;
        theatre_dijkstra_visited[u] = 1;
        for (int v = 0; v < n; ++v)
        {
            if (!theatre_dijkstra_visited[v] && theatreCost[u][v] < THEATRE_INF)
            {
                if (theatre_dijkstra_dist[u] + theatreCost[u][v] < theatre_dijkstra_dist[v])
                {
                    theatre_dijkstra_dist[v] = theatre_dijkstra_dist[u] + theatreCost[u][v];
                    theatre_dijkstra_path[v] = u;
                }
            }
        }
    }
}
void theatre_print_dijkstra_path(int dest)
{
    int pathArr[THEATRE_MAX_AUDITORIUMS];
    int cnt = 0;
    int cur = dest;
    // guard: if path array uninitialized, avoid infinite loop
    if (theatre_dijkstra_path[cur] < 0 || theatre_dijkstra_path[cur] >= theatreGraphN) {
        cout << "(no path)\n";
        return;
    }
    while (cur != theatre_dijkstra_path[cur])
    {
        pathArr[cnt++] = cur;
        cur = theatre_dijkstra_path[cur];
        if (cnt >= THEATRE_MAX_AUDITORIUMS - 1) break;
    }
    pathArr[cnt++] = cur;
    cout << "Path: ";
    for (int i = cnt - 1; i >= 0; --i)
    {
        cout << auditoriums[pathArr[i]].name;
        if (i)
            cout << " -> ";
    }
    cout << "\n";
}

// -------------------- CSV LOADERS & GENERATORS --------------------
// movies.csv: movie_id,title,genre,duration_minutes,rating,language,release_date
void theatreLoadMoviesCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line); // header
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2)
            continue;
        string cols[8];
        int n = splitCSV(line, cols, 8);
        if (n < 6)
            continue;
        if (theatreMoviesAtCapacity())
        {
            cout << "Overflow: movies capacity reached!\n";
            break;
        }
        int id = toInt(cols[0]);
        string title = cols[1];
        string genre = cols[2];
        int dur = toInt(cols[3]);
        double rating = atof(cols[4].c_str());
        string lang = cols[5];
        string date = (n >= 7 ? cols[6] : "1970-01-01");
        TheatreMovie *m = theatreCreateMovieNode(id, title, genre, dur, rating, lang, date);
        theatreMovieRoot = theatreInsertMovieNode(theatreMovieRoot, m);
        loaded++;
    }
    cout << "Loaded " << loaded << " movies from " << fn << "\n";
}

// auditoriums.csv: aud_id,name,rows,cols,type,total_seats
void theatreLoadAuditoriumsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2)
            continue;
        string cols[8];
        int n = splitCSV(line, cols, 8);
        if (n < 4)
            continue;
        if (theatreAudCount >= THEATRE_MAX_AUDITORIUMS)
        {
            cout << "Overflow: auditoriums max reached!\n";
            break;
        }
        int id = toInt(cols[0]);
        auditoriums[theatreAudCount].aud_id = id;
        auditoriums[theatreAudCount].name = cols[1];
        int r = min(toInt(cols[2]), THEATRE_MAX_SEAT_ROWS);
        int c = min(toInt(cols[3]), THEATRE_MAX_SEAT_COLS);
        auditoriums[theatreAudCount].rows = r;
        auditoriums[theatreAudCount].cols = c;
        auditoriums[theatreAudCount].total_seats = r * c;
        for (int i = 0; i < r; ++i)
            for (int j = 0; j < c; ++j)
            {
                auditoriums[theatreAudCount].seats[i][j] = 'E';
                auditoriums[theatreAudCount].seat_type[i][j] = "standard";
            }
        theatreAudCount++;
        loaded++;
    }
    theatre_build_graph(); // adjust graph
    cout << "Loaded " << loaded << " auditoriums from " << fn << "\n";
}

// shows.csv: show_id,movie_id,aud_id,start_datetime,end_datetime,base_price
void theatreLoadShowsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2)
            continue;
        string cols[8];
        int n = splitCSV(line, cols, 8);
        if (n < 6)
            continue;
        if (theatreShowCount >= THEATRE_MAX_SHOWS)
        {
            cout << "Overflow: shows max reached!\n";
            break;
        }
        shows[theatreShowCount].show_id = toInt(cols[0]);
        shows[theatreShowCount].movie_id = toInt(cols[1]);
        shows[theatreShowCount].aud_id = toInt(cols[2]);
        shows[theatreShowCount].start_datetime = cols[3];
        shows[theatreShowCount].end_datetime = cols[4];
        shows[theatreShowCount].base_price = toInt(cols[5]);
        shows[theatreShowCount].tickets_sold = 0;
        shows[theatreShowCount].revenue = 0;
        theatreShowCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " shows from " << fn << "\n";
}

// bookings.csv: booking_id,show_id,seat_label,customer_name,customer_phone,price_paid,status,booking_datetime
void theatreLoadBookingsCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2)
            continue;
        string cols[12];
        int n = splitCSV(line, cols, 12);
        if (n < 7)
            continue;
        TheatreBooking b;
        b.booking_id = toInt(cols[0]);
        b.show_id = toInt(cols[1]);
        strncpy(b.seat_label, cols[2].c_str(), sizeof(b.seat_label) - 1);
        b.seat_label[sizeof(b.seat_label) - 1] = 0;
        strncpy(b.customer_name, cols[3].c_str(), sizeof(b.customer_name) - 1);
        b.customer_name[sizeof(b.customer_name) - 1] = 0;
        strncpy(b.customer_phone, cols[4].c_str(), sizeof(b.customer_phone) - 1);
        b.customer_phone[sizeof(b.customer_phone) - 1] = 0;
        b.price_paid = toInt(cols[5]);
        b.status = toInt(cols[6]);
        if (n >= 8)
            strncpy(b.booking_datetime, cols[7].c_str(), sizeof(b.booking_datetime) - 1);
        else
            strncpy(b.booking_datetime, theatre_now_datetime().c_str(), sizeof(b.booking_datetime) - 1);
        theatre_booking_insert(b);
        loaded++;
        // mark seat as booked in auditorium if possible
        int sidx = -1;
        for (int i = 0; i < theatreShowCount; ++i) if (shows[i].show_id == b.show_id) { sidx = i; break; }
        if (sidx != -1) {
            int aud_idx = -1;
            for (int i = 0; i < theatreAudCount; ++i) if (auditoriums[i].aud_id == shows[sidx].aud_id) { aud_idx = i; break; }
            if (aud_idx != -1) {
                int rr, cc;
                string seatl = string(b.seat_label);
                auto theatre_parse_seat_label_local = [&](TheatreAuditorium &a, const string &label, int &r, int &c)->bool {
                    if (label.size() < 2) return false;
                    char rc = label[0];
                    int rowIdx = rc - 'A';
                    int colIdx = atoi(label.substr(1).c_str()) - 1;
                    if (rowIdx < 0 || rowIdx >= a.rows) return false;
                    if (colIdx < 0 || colIdx >= a.cols) return false;
                    r = rowIdx; c = colIdx; return true;
                };
                if (theatre_parse_seat_label_local(auditoriums[aud_idx], seatl, rr, cc)) {
                    auditoriums[aud_idx].seats[rr][cc] = (b.status==1 ? 'B' : 'E');
                }
            }
        }
    }
    cout << "Loaded " << loaded << " bookings from " << fn << "\n";
}

// staff CSV loader: id,name,role,salary
void theatreLoadStaffCSV(const string &fn)
{
    ifstream in(fn.c_str());
    if (!in.is_open())
    {
        cout << "Cannot open " << fn << "\n";
        return;
    }
    string line;
    getline(in, line);
    int loaded = 0;
    while (getline(in, line))
    {
        if (line.size() < 2)
            continue;
        string cols[6];
        int n = splitCSV(line, cols, 6);
        if (n < 4)
            continue;
        if (theatreStaffCount >= THEATRE_MAX_STAFF)
        {
            cout << "Overflow: theatre staff max reached!\n";
            break;
        }
        theatreStaff[theatreStaffCount].id = toInt(cols[0]);
        theatreStaff[theatreStaffCount].name = cols[1];
        theatreStaff[theatreStaffCount].role = cols[2];
        theatreStaff[theatreStaffCount].salary = toInt(cols[3]);
        theatreStaffCount++;
        loaded++;
    }
    cout << "Loaded " << loaded << " staff from " << fn << "\n";
}

// -------------------- INTERACTIVE / MENU FUNCTIONS --------------------
int theatre_createMovieID()
{
    static int mid = 3000;
    return ++mid;
}
int theatre_createShowID()
{
    static int sid = 11000;
    return ++sid;
}

void theatreAddMovie()
{
    if (theatreMoviesAtCapacity())
    {
        cout << "Overflow: movies capacity reached!\n";
        return;
    }
    int id = theatre_createMovieID();
    string title, genre, durS, ratingS, lang, date;
    cout << "Enter title: ";
    getline(cin, title);
    cout << "Enter genre: ";
    getline(cin, genre);
    cout << "Enter duration (minutes): ";
    getline(cin, durS);
    cout << "Enter rating (numeric): ";
    getline(cin, ratingS);
    cout << "Enter language: ";
    getline(cin, lang);
    cout << "Enter release date (YYYY-MM-DD): ";
    getline(cin, date);
    TheatreMovie *m = theatreCreateMovieNode(id, title, genre, toInt(durS), atof(ratingS.c_str()), lang, date);
    theatreMovieRoot = theatreInsertMovieNode(theatreMovieRoot, m);
    cout << "Added movie id " << id << "\n";
}

void theatreDeleteMovie()
{
    cout << "Enter exact title to delete: ";
    string t;
    getline(cin, t);
    bool deleted = false;
    theatreMovieRoot = theatreDeleteMovieNode(theatreMovieRoot, t, deleted);
    if (deleted)
        cout << "Deleted.\n";
    else
        cout << "Not found.\n";
}

void theatreListMovies()
{
    if (!theatreMovieRoot)
    {
        cout << "No movies.\n";
        return;
    }
    theatreInorderList(theatreMovieRoot);
}

void theatreSearchMovie()
{
    cout << "Enter search pattern: ";
    string pat;
    getline(cin, pat);
    if (pat.empty())
    {
        cout << "Empty pattern.\n";
        return;
    }
    // traverse inorder and use boyer-moore on lowercase forms
    function<void(TheatreMovie *)> inorder = [&](TheatreMovie *node)
    {
        if (!node)
            return;
        inorder(node->left);
        string s = node->title;
        string ss = s, pp = pat;
        for (size_t i = 0; i < ss.size(); ++i)
            ss[i] = tolower(ss[i]);
        for (size_t i = 0; i < pp.size(); ++i)
            pp[i] = tolower(pp[i]);
        if (theatre_boyer_moore_search(ss, pp))
        {
            cout << "Found: " << node->title << " (ID " << node->movie_id << ")\n";
        }
        inorder(node->right);
    };
    inorder(theatreMovieRoot);
}

// Add auditorium interactively
void theatreAddAuditorium()
{
    if (theatreAudCount >= THEATRE_MAX_AUDITORIUMS)
    {
        cout << "Overflow: auditoriums limit\n";
        return;
    }
    int id = 600 + theatreAudCount;
    string name, rS, cS;
    cout << "Enter auditorium name: ";
    getline(cin, name);
    cout << "Enter rows: ";
    getline(cin, rS);
    cout << "Enter cols: ";
    getline(cin, cS);
    int r = min(toInt(rS), THEATRE_MAX_SEAT_ROWS);
    int c = min(toInt(cS), THEATRE_MAX_SEAT_COLS);
    auditoriums[theatreAudCount].aud_id = id;
    auditoriums[theatreAudCount].name = name;
    auditoriums[theatreAudCount].rows = r;
    auditoriums[theatreAudCount].cols = c;
    auditoriums[theatreAudCount].total_seats = r * c;
    for (int i = 0; i < r; ++i)
        for (int j = 0; j < c; ++j)
        {
            auditoriums[theatreAudCount].seats[i][j] = 'E';
            auditoriums[theatreAudCount].seat_type[i][j] = "standard";
        }
    theatreAudCount++;
    theatre_build_graph();
    cout << "Added auditorium id " << id << "\n";
}

void theatreViewSeatMap()
{
    cout << "Enter auditorium id: ";
    int id;
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int idx = -1;
    for (int i = 0; i < theatreAudCount; ++i)
        if (auditoriums[i].aud_id == id)
        {
            idx = i;
            break;
        }
    if (idx == -1)
    {
        cout << "Not found.\n";
        return;
    }
    TheatreAuditorium &a = auditoriums[idx];
    cout << "Seat map for " << a.name << " (" << a.rows << "x" << a.cols << ")\n";
    for (int r = 0; r < a.rows; ++r)
    {
        for (int c = 0; c < a.cols; ++c)
            cout << a.seats[r][c];
        cout << "\n";
    }
}

bool theatre_parse_seat_label(TheatreAuditorium &a, const string &label, int &r, int &c)
{
    if (label.size() < 2)
        return false;
    char rc = label[0];
    int rowIdx = rc - 'A';
    int colIdx = atoi(label.substr(1).c_str()) - 1;
    if (rowIdx < 0 || rowIdx >= a.rows)
        return false;
    if (colIdx < 0 || colIdx >= a.cols)
        return false;
    r = rowIdx;
    c = colIdx;
    return true;
}

void theatreBookSeat()
{
    if (theatreBookingCount >= THEATRE_MAX_BOOKINGS)
    {
        cout << "Overflow: booking limit\n";
        return;
    }
    cout << "Enter show id: ";
    int sid;
    cin >> sid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int sidx = -1;
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].show_id == sid)
        {
            sidx = i;
            break;
        }
    if (sidx == -1)
    {
        cout << "Show not found.\n";
        return;
    }
    int aud_idx = -1;
    for (int i = 0; i < theatreAudCount; ++i)
        if (auditoriums[i].aud_id == shows[sidx].aud_id)
        {
            aud_idx = i;
            break;
        }
    if (aud_idx == -1)
    {
        cout << "Auditorium not found for this show.\n";
        return;
    }
    cout << "Enter seat label (e.g., A1): ";
    string seat;
    getline(cin, seat);
    int r, c;
    if (!theatre_parse_seat_label(auditoriums[aud_idx], seat, r, c))
    {
        cout << "Invalid seat label\n";
        return;
    }
    if (auditoriums[aud_idx].seats[r][c] != 'E')
    {
        cout << "Seat unavailable\n";
        return;
    }
    cout << "Enter customer name: ";
    string cname;
    getline(cin, cname);
    cout << "Enter customer phone: ";
    string phone;
    getline(cin, phone);
    TheatreBooking b;
    b.booking_id = theatreNextBookingId++;
    b.show_id = sid;
    strncpy(b.seat_label, seat.c_str(), sizeof(b.seat_label) - 1);
    b.seat_label[sizeof(b.seat_label) - 1] = 0;
    strncpy(b.customer_name, cname.c_str(), sizeof(b.customer_name) - 1);
    b.customer_name[sizeof(b.customer_name) - 1] = 0;
    strncpy(b.customer_phone, phone.c_str(), sizeof(b.customer_phone) - 1);
    b.customer_phone[sizeof(b.customer_phone) - 1] = 0;
    b.price_paid = shows[sidx].base_price;
    b.status = 1;
    strncpy(b.booking_datetime, theatre_now_datetime().c_str(), sizeof(b.booking_datetime) - 1);
    if (!theatre_booking_insert(b))
    {
        cout << "Failed to insert booking.\n";
        return;
    }
    auditoriums[aud_idx].seats[r][c] = 'B';
    shows[sidx].tickets_sold++;
    shows[sidx].revenue += b.price_paid;
    cout << "Booking done. ID: " << b.booking_id << "\n";
}

void theatreCancelBooking()
{
    cout << "Enter booking id: ";
    int bid;
    cin >> bid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    TheatreBooking b;
    if (!theatre_booking_get(bid, b))
    {
        cout << "Not found.\n";
        return;
    }
    if (b.status == 0)
    {
        cout << "Already cancelled.\n";
        return;
    }
    // find show and free seat
    int sidx = -1;
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].show_id == b.show_id)
        {
            sidx = i;
            break;
        }
    if (sidx != -1)
    {
        int aud_idx = -1;
        for (int i = 0; i < theatreAudCount; ++i)
            if (auditoriums[i].aud_id == shows[sidx].aud_id)
            {
                aud_idx = i;
                break;
            }
        if (aud_idx != -1)
        {
            int rr, cc;
            if (theatre_parse_seat_label(auditoriums[aud_idx], string(b.seat_label), rr, cc))
            {
                auditoriums[aud_idx].seats[rr][cc] = 'E';
                shows[sidx].tickets_sold = max(0, shows[sidx].tickets_sold - 1);
                shows[sidx].revenue = max(0, shows[sidx].revenue - b.price_paid);
            }
        }
    }
    theatre_booking_remove(bid);
    cout << "Cancelled booking " << bid << "\n";
}

void theatreListShows()
{
    if (theatreShowCount == 0)
    {
        cout << "No shows.\n";
        return;
    }
    int idxArr[THEATRE_MAX_SHOWS];
    for (int i = 0; i < theatreShowCount; ++i)
        idxArr[i] = i;
    theatre_quickSort_indices(idxArr, 0, theatreShowCount - 1, theatre_cmp_show_start);
    cout << "Shows sorted by start time:\n";
    for (int i = 0; i < theatreShowCount; ++i)
    {
        TheatreShow &s = shows[idxArr[i]];
        cout << s.show_id << " | Movie:" << s.movie_id << " | Aud:" << s.aud_id << " | " << s.start_datetime << " - " << s.end_datetime << " | Price:" << s.base_price << " | Tickets:" << s.tickets_sold << "\n";
    }
}

bool theatre_check_show_conflict(const string &s1, const string &e1, const string &s2, const string &e2)
{
    // assuming format YYYY-MM-DD HH:MM lexicographical comparisons work
    if (s1 >= e2)
        return false;
    if (s2 >= e1)
        return false;
    return true;
}

void theatreAddShow()
{
    if (theatreShowCount >= THEATRE_MAX_SHOWS)
    {
        cout << "Overflow: shows limit\n";
        return;
    }
    TheatreShow sh;
    sh.show_id = theatre_createShowID();
    string mS, aS;
    cout << "Enter movie id: ";
    getline(cin, mS);
    cout << "Enter auditorium id: ";
    getline(cin, aS);
    cout << "Enter start datetime (YYYY-MM-DD HH:MM): ";
    getline(cin, sh.start_datetime);
    cout << "Enter end datetime (YYYY-MM-DD HH:MM): ";
    getline(cin, sh.end_datetime);
    cout << "Enter base price: ";
    string pS;
    getline(cin, pS);
    sh.movie_id = toInt(mS);
    sh.aud_id = toInt(aS);
    sh.base_price = toInt(pS);
    sh.tickets_sold = 0;
    sh.revenue = 0;
    // conflict check
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].aud_id == sh.aud_id)
        {
            if (theatre_check_show_conflict(sh.start_datetime, sh.end_datetime, shows[i].start_datetime, shows[i].end_datetime))
            {
                cout << "Conflict with show " << shows[i].show_id << " : " << shows[i].start_datetime << " - " << shows[i].end_datetime << "\n";
                cout << "Add anyway? (y/n): ";
                char c;
                cin >> c;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (c != 'y' && c != 'Y')
                    return;
                break;
            }
        }
    shows[theatreShowCount++] = sh;
    cout << "Show added id " << sh.show_id << "\n";
}

// Snack functions
void theatreAddSnack()
{
    if (theatreSnackCount >= THEATRE_MAX_SNACKS)
    {
        cout << "Overflow: snacks limit\n";
        return;
    }
    TheatreSnack &s = snacks[theatreSnackCount];
    s.snack_id = 8000 + theatreSnackCount + 1;
    cout << "Enter snack name: ";
    getline(cin, s.name);
    cout << "Enter category: ";
    getline(cin, s.category);
    cout << "Enter price: ";
    string p;
    getline(cin, p);
    s.price = toInt(p);
    cout << "Enter prep time (minutes): ";
    string t;
    getline(cin, t);
    s.prep_time = toInt(t);
    theatreSnackCount++;
    cout << "Snack added id " << s.snack_id << "\n";
}
void theatreListSnacks()
{
    if (theatreSnackCount == 0)
    {
        cout << "No snacks.\n";
        return;
    }
    for (int i = 0; i < theatreSnackCount; ++i)
        cout << snacks[i].snack_id << " | " << snacks[i].name << " | " << snacks[i].category << " | Rs " << snacks[i].price << "\n";
}
void theatreOrderSnack()
{
    if (snackQueueCount >= THEATRE_MAX_SNACK_ORDERS)
    {
        cout << "Overflow: snack orders full\n";
        return;
    }
    TheatreSnackOrder ord;
    ord.order_id = rand() % 100000 + 10000;
    cout << "Enter booking id (0 if none): ";
    int bid;
    cin >> bid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    ord.booking_id = bid;
    cout << "Enter seat label (e.g., A1): ";
    string seat;
    getline(cin, seat);
    strncpy(ord.seat_label, seat.c_str(), sizeof(ord.seat_label) - 1);
    ord.item_count = 0;
    ord.total_price = 0;
    cout << "Enter snack ids separated by space, terminated by 0:\n";
    while (true)
    {
        int sid;
        cin >> sid;
        if (sid == 0)
            break;
        if (ord.item_count < 10)
            ord.item_ids[ord.item_count++] = sid;
        for (int i = 0; i < theatreSnackCount; ++i)
            if (snacks[i].snack_id == sid)
                ord.total_price += snacks[i].price;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    ord.status = 0;
    strncpy(ord.order_datetime, theatre_now_datetime().c_str(), sizeof(ord.order_datetime) - 1);
    if (theatre_enqueue_snack(ord))
        cout << "Order queued id " << ord.order_id << " total Rs " << ord.total_price << "\n";
}

void theatreProcessSnack()
{
    TheatreSnackOrder ord = theatre_dequeue_snack();
    if (ord.order_id == -1)
    {
        cout << "No snack orders.\n";
        return;
    }
    cout << "Processing order " << ord.order_id << " total Rs " << ord.total_price << "\n";
    cout << "Mark done? (y/n): ";
    char c;
    cin >> c;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (c == 'y' || c == 'Y')
        cout << "Done.\n";
    else
    {
        ord.status = 0;
        theatre_enqueue_snack(ord);
        cout << "Requeued.\n";
    }
}

// Staff & maintenance
void theatreAddStaff()
{
    if (theatreStaffCount >= THEATRE_MAX_STAFF)
    {
        cout << "Overflow: staff limit\n";
        return;
    }
    TheatreStaff &s = theatreStaff[theatreStaffCount];
    s.id = theatreStaffCount + 1;
    cout << "Enter name: ";
    getline(cin, s.name);
    cout << "Enter role: ";
    getline(cin, s.role);
    cout << "Enter salary: ";
    string tmp;
    getline(cin, tmp);
    s.salary = toInt(tmp);
    theatreStaffCount++;
    cout << "Staff added id " << s.id << "\n";
}
void theatreListStaff()
{
    if (theatreStaffCount == 0)
    {
        cout << "No staff.\n";
        return;
    }
    for (int i = 0; i < theatreStaffCount; ++i)
        cout << theatreStaff[i].id << " | " << theatreStaff[i].name << " | " << theatreStaff[i].role << " | Rs " << theatreStaff[i].salary << "\n";
}
void theatreAddMaint()
{
    if (theatreMaintCount >= THEATRE_MAX_MAINT_LOGS)
    {
        cout << "Overflow: maint logs full\n";
        return;
    }
    TheatreMaint &m = theatreMaint[theatreMaintCount];
    m.id = theatreMaintCount + 1;
    cout << "Enter auditorium id: ";
    string t;
    getline(cin, t);
    m.aud_id = toInt(t);
    cout << "Enter date (YYYY-MM-DD): ";
    getline(cin, m.date);
    cout << "Enter task: ";
    getline(cin, m.task);
    cout << "Enter staff id: ";
    getline(cin, t);
    m.staff_id = toInt(t);
    m.status = 0;
    theatreMaintCount++;
    cout << "Maintenance logged id " << m.id << "\n";
}
void theatreListMaint()
{
    if (theatreMaintCount == 0)
    {
        cout << "No maintenance logs.\n";
        return;
    }
    for (int i = 0; i < theatreMaintCount; ++i)
        cout << theatreMaint[i].id << " | Aud:" << theatreMaint[i].aud_id << " | " << theatreMaint[i].date << " | " << theatreMaint[i].task << " | Staff:" << theatreMaint[i].staff_id << "\n";
}

// Revenue report per show
void theatreShowRevenue()
{
    cout << "Enter show id: ";
    int sid;
    cin >> sid;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    for (int i = 0; i < theatreShowCount; ++i)
        if (shows[i].show_id == sid)
        {
            cout << "Tickets sold: " << shows[i].tickets_sold << " Revenue: Rs " << shows[i].revenue << "\n";
            return;
        }
    cout << "Show not found.\n";
}

// Evacuation route (Dijkstra)
void theatreEvacuation()
{
    if (theatreAudCount == 0)
    {
        cout << "No auditoriums.\n";
        return;
    }
    cout << "Enter source auditorium id: ";
    int id;
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    int src = -1;
    for (int i = 0; i < theatreAudCount; ++i)
        if (auditoriums[i].aud_id == id)
        {
            src = i;
            break;
        }
    if (src == -1)
    {
        cout << "Not found.\n";
        return;
    }
    theatre_dijkstra(theatreGraphN, src);
    cout << "Distances from " << auditoriums[src].name << ":\n";
    for (int i = 0; i < theatreGraphN; ++i)
    {
        cout << auditoriums[i].name << ": " << theatre_dijkstra_dist[i] << "\n";
        theatre_print_dijkstra_path(i);
    }
}

// -------------------- MODULE MENU --------------------
void theatreShowMenu()
{
    cout << "\n====================================\n";
    cout << "         THEATRE MANAGEMENT\n";
    cout << "====================================\n";
    cout << " 1. Show all movies\n";
    cout << " 2. Add new movie\n";
    cout << " 3. Delete movie\n";
    cout << " 4. Search movie (BM)\n";
    cout << " 5. Sort shows by start time (QuickSort)\n";
    cout << " 6. Add auditorium\n";
    cout << " 7. View seat map\n";
    cout << " 8. Load auditoriums from CSV (auditoriums.csv)\n";
    cout << " 9. Add show (schedule)\n";
    cout << "10. List shows\n";
    cout << "11. Load shows from CSV (shows.csv)\n";
    cout << "12. Book seat\n";
    cout << "13. Cancel booking\n";
    cout << "14. Load bookings CSV (bookings.csv)\n";
    cout << "15. Find booking by phone (BM search)\n";
    cout << "16. Add snack\n";
    cout << "17. List snacks\n";
    cout << "18. Order snack to seat\n";
    cout << "19. Process next snack order\n";
    cout << "20. Add staff\n";
    cout << "21. Show staff\n";
    cout << "22. Load staff CSV (theatre_staff.csv)\n";
    cout << "23. Add maintenance log\n";
    cout << "24. Show maintenance logs\n";
    cout << "25. Show revenue for show\n";
    cout << "26. Evacuation route (Dijkstra)\n";
    cout << "27. Load movies from CSV (movies.csv)\n";
    cout << "28. Load bookings from CSV (bookings.csv)\n";
    cout << "29. List auditorium\n";
    cout << "30.Load All Data\n";
    cout << " 0. Return to MAIN MENU\n";
    cout << "====================================\n";
    cout << "Enter choice: ";
}

void theatreInitModule()
{
    srand((unsigned int)time(NULL));
    theatre_init_booking_hash();
    theatre_init_snack_queue();
    theatre_build_graph();
}

// find bookings by phone substring using BM
void theatreFindBookingByPhone()
{
    cout << "Enter phone substring: ";
    string pat;
    getline(cin, pat);
    if (pat.empty())
    {
        cout << "Empty.\n";
        return;
    }
    bool found = false;
    for (int i = 0; i < THEATRE_HASH_SIZE; ++i)
    {
        if (bookingHash[i].used && !bookingHash[i].deleted)
        {
            TheatreBooking &b = bookingHash[i].val;
            string phone = string(b.customer_phone);
            string phoneL = phone;
            string patL = pat;
            for (size_t k = 0; k < phoneL.size(); ++k)
                phoneL[k] = tolower(phoneL[k]);
            for (size_t k = 0; k < patL.size(); ++k)
                patL[k] = tolower(patL[k]);
            if (theatre_boyer_moore_search(phoneL, patL))
            {
                cout << "Booking ID: " << b.booking_id << " Show: " << b.show_id << " Seat: " << b.seat_label << " Cust: " << b.customer_name << " Phone: " << b.customer_phone << "\n";
                found = true;
            }
        }
    }
    if (!found)
        cout << "No matching bookings.\n";
}
void theatreListAuditoriums()
{
    if (theatreAudCount == 0)
    {
        cout << "No auditoriums available.\n";
        return;
    }

    cout << "\n--- AUDITORIUM LIST ---\n";
    for (int i = 0; i < theatreAudCount; i++)
    {
        TheatreAuditorium &a = auditoriums[i];
        cout << "ID: " << a.aud_id
             << " | Name: " << a.name
             << " | Rows: " << a.rows
             << " | Cols: " << a.cols
             << " | Total Seats: " << a.total_seats
             << "\n";
    }
}
void theatreLoadAllData()
{
    cout << "\n=== Loading ALL Theatre Data ===\n";

    theatreLoadMoviesCSV("movies.csv");
    theatreLoadAuditoriumsCSV("auditoriums.csv");
    theatreLoadShowsCSV("shows.csv");
    theatreLoadBookingsCSV("bookings.csv");
    theatreLoadStaffCSV("theatre_staff.csv");

    cout << "=== Finished loading all CSV files ===\n";
}



// -------------------- THEATRE SYSTEM MAIN (entry) --------------------
void theatreSystem()
{
    theatreInitModule();
    int choice;
    while (true)
    {
        theatreShowMenu();
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        switch (choice)
        {
        case 1:
            theatreListMovies();
            break;
        case 2:
            theatreAddMovie();
            break;
        case 3:
            theatreDeleteMovie();
            break;
        case 4:
            theatreSearchMovie();
            break;
        case 5:
            theatreListShows();
            break;
        case 6:
            theatreAddAuditorium();
            break;
        case 7:
            theatreViewSeatMap();
            break;
        case 8:
            theatreLoadAuditoriumsCSV("auditoriums.csv");
            break;
        case 9:
            theatreAddShow();
            break;
        case 10:
            theatreListShows();
            break;
        case 11:
            theatreLoadShowsCSV("shows.csv");
            break;
        case 12:
            theatreBookSeat();
            break;
        case 13:
            theatreCancelBooking();
            break;
        case 14:
            theatreLoadBookingsCSV("bookings.csv");
            break;
        case 15:
            theatreFindBookingByPhone();
            break;
        case 16:
            theatreAddSnack();
            break;
        case 17:
            theatreListSnacks();
            break;
        case 18:
            theatreOrderSnack();
            break;
        case 19:
            theatreProcessSnack();
            break;
        case 20:
            theatreAddStaff();
            break;
        case 21:
            theatreListStaff();
            break;
        case 22:
            theatreLoadStaffCSV("theatre_staff.csv");
            break;
        case 23:
            theatreAddMaint();
            break;
        case 24:
            theatreListMaint();
            break;
        case 25:
            theatreShowRevenue();
            break;
        case 26:
            theatreEvacuation();
            break;
        case 27:
            theatreLoadMoviesCSV("movies.csv");
            break;
        case 28:
            theatreLoadBookingsCSV("bookings.csv");
            break;
        case 29:
            theatreListAuditoriums();
            break;
        case 30:
             theatreLoadAllData();
             break;
        case 0:
            cout << "Returning to main menu...\n";
            return;
        default:
            cout << "Invalid choice.\n";
        }
    }
}

// -------------------- TEST MAIN (for standalone testing) --------------------
int main()
{
    // Quick note: when integrating into your mega project remove this main()
    theatreSystem();
    return 0;
}
