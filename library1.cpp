// ===============================
// Enhanced Library Management System (C++98 Compatible)
// ===============================

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <map>
#include <set>
#include <algorithm>
using namespace std;

// ===== Levenshtein Distance for Fuzzy Search =====
int min3(int a, int b, int c) {
    return min(a, min(b, c));
}

int levenshtein(const string& a, const string& b) {
    vector< vector<int> > dp(a.size() + 1, vector<int>(b.size() + 1));
    for (size_t i = 0; i <= a.size(); ++i) dp[i][0] = i;
    for (size_t j = 0; j <= b.size(); ++j) dp[0][j] = j;
    for (size_t i = 1; i <= a.size(); ++i) {
        for (size_t j = 1; j <= b.size(); ++j) {
            if (a[i - 1] == b[j - 1]) dp[i][j] = dp[i - 1][j - 1];
            else dp[i][j] = 1 + min3(dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]);
        }
    }
    return dp[a.size()][b.size()];
}

string toLower(const string &s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

struct IssuedInfo {
    string user;
    time_t issueDate;
};

struct Review {
    string user;
    int rating; // 1 to 5
    string comment;
};

class Book {
public:
    int id;
    string title, author, genre;
    bool issued;
    IssuedInfo info;
    vector<Review> reviews;

    Book() {
        id = 0;
        issued = false;
        info.user = "";
        info.issueDate = 0;
    }

    Book(int _id, string _title, string _author, string _genre) {
        id = _id;
        title = _title;
        author = _author;
        genre = _genre;
        issued = false;
        info.user = "";
        info.issueDate = 0;
    }

    void display() const {
        cout << "ID: " << id << ", Title: " << title << ", Author: " << author << ", Genre: " << genre;
        if (issued) cout << ", Issued to: " << info.user;
        cout << ", Issued: " << (issued ? "Yes" : "No") << endl;
        if (!reviews.empty()) {
            double avg = 0;
            for (size_t i = 0; i < reviews.size(); ++i) avg += reviews[i].rating;
            avg /= reviews.size();
            cout << "Average Rating: " << avg << "\n";
        }
    }
};

class Library {
private:
    vector<Book> books;
    map<int, vector<string> > waitlist;
    map<string, vector<int> > borrowingHistory;
    string currentUser;
    bool isAdmin;

    void saveToFile() {
        ofstream file("library1.txt");
        for (size_t i = 0; i < books.size(); ++i) {
            Book b = books[i];
            file << b.id << "\n" << b.title << "\n" << b.author << "\n" << b.genre << "\n";
            file << b.issued << "\n" << b.info.user << "\n" << b.info.issueDate << "\n";
            file << b.reviews.size() << "\n";
            for (size_t j = 0; j < b.reviews.size(); ++j) {
                file << b.reviews[j].user << "\n" << b.reviews[j].rating << "\n" << b.reviews[j].comment << "\n";
            }
            file << "---\n";
        }
        file.close();
    }

    void loadFromFile() {
        ifstream file("library1.txt");
        books.clear();
        string line;
        while (getline(file, line)) {
            Book b;
            b.id = atoi(line.c_str());
            getline(file, b.title);
            getline(file, b.author);
            getline(file, b.genre);
            getline(file, line); b.issued = (line == "1");
            getline(file, b.info.user);
            getline(file, line); b.info.issueDate = atol(line.c_str());
            getline(file, line);
            int reviewCount = atoi(line.c_str());
            for (int i = 0; i < reviewCount; ++i) {
                Review r;
                getline(file, r.user);
                getline(file, line); r.rating = atoi(line.c_str());
                getline(file, r.comment);
                b.reviews.push_back(r);
            }
            getline(file, line); // ---
            books.push_back(b);
        }
        file.close();
    }

    int calculateFine(time_t issueDate) {
        time_t now = time(0);
        double days = difftime(now, issueDate) / (60 * 60 * 24);
        if (days > 7) return (int)((days - 7) * 5);
        return 0;
    }

public:
    Library(string user, bool admin) {
        currentUser = user;
        isAdmin = admin;
        loadFromFile();
    }

    void addBook() {
        if (!isAdmin) return;
        int id; string title, author, genre;
        cout << "Enter Book ID: "; cin >> id;
        cin.ignore();
        cout << "Enter Title: "; getline(cin, title);
        cout << "Enter Author: "; getline(cin, author);
        cout << "Enter Genre: "; getline(cin, genre);
        books.push_back(Book(id, title, author, genre));
        saveToFile();
        cout << "Book added successfully!\n";
    }

    void viewBooks() {
        cout << "\n--- Book List ---\n";
        for (size_t i = 0; i < books.size(); ++i) books[i].display();
    }

    void searchFuzzyTitle() {
        string query;
        cin.ignore();
        cout << "Enter book title (approx): ";
        getline(cin, query);

        query = toLower(query);
        vector< pair<int, int> > matches;

        for (size_t i = 0; i < books.size(); ++i) {
            string fullTitle = toLower(books[i].title);
            istringstream iss(fullTitle);
            string word;
            bool matched = false;
            int bestDist = 1000;

            while (iss >> word) {
                int dist = levenshtein(word, query);
                int threshold = max(2, (int)(word.length() * 0.4));
                if (dist <= threshold) {
                    matched = true;
                    if (dist < bestDist) bestDist = dist;
                }
            }

            if (matched)
                matches.push_back(make_pair(bestDist, (int)i));
        }

        sort(matches.begin(), matches.end());

        if (matches.empty()) {
            cout << "No similar titles found.\n";
            return;
        }

        cout << "\n--- Matching Books ---\n";
        for (size_t i = 0; i < matches.size(); ++i)
            books[matches[i].second].display();
    }

    void issueBook() {
        int id;
        cout << "Enter Book ID to issue: "; cin >> id;
        for (size_t i = 0; i < books.size(); ++i) {
            if (books[i].id == id) {
                if (books[i].issued) {
                    cout << "Book is already issued. Added to waitlist.\n";
                    waitlist[id].push_back(currentUser);
                    return;
                }
                books[i].issued = true;
                books[i].info.user = currentUser;
                books[i].info.issueDate = time(0);
                borrowingHistory[currentUser].push_back(id);
                saveToFile();
                cout << "Book issued successfully.\n";
                return;
            }
        }
        cout << "Book not found.\n";
    }

    void returnBook() {
        int id;
        cout << "Enter Book ID to return: "; cin >> id;
        for (size_t i = 0; i < books.size(); ++i) {
            if (books[i].id == id && books[i].issued && books[i].info.user == currentUser) {
                int fine = calculateFine(books[i].info.issueDate);
                cout << "Fine (if any): ₹" << fine << endl;
                books[i].issued = false;
                books[i].info.user = "";
                books[i].info.issueDate = 0;
                if (!waitlist[id].empty()) {
                    cout << "Notifying next user in waitlist: " << waitlist[id][0] << endl;
                    waitlist[id].erase(waitlist[id].begin());
                }
                saveToFile();
                cout << "Book returned successfully.\n";
                return;
            }
        }
        cout << "Book not found or not issued by you.\n";
    }

    void rateBook() {
        string name;
        int rating;
        string comment;
        cin.ignore();
        cout << "Enter Book Title to rate: ";
        getline(cin, name);
        name = toLower(name);

        int foundIndex = -1;
        for (size_t i = 0; i < books.size(); ++i) {
            if (toLower(books[i].title) == name) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex == -1) {
            cout << "Book not found.\n";
            return;
        }

        cout << "Enter rating (1-5): "; cin >> rating;
        cin.ignore();
        cout << "Enter comment: "; getline(cin, comment);

        Review r;
        r.user = currentUser; r.rating = rating; r.comment = comment;
        books[foundIndex].reviews.push_back(r);
        saveToFile();
        cout << "Review added.\n";
    }
};

bool login(string &username, bool &isAdmin) {
    cout << "Login\nEnter username: ";
    cin >> username;
    string pwd;
    cout << "Enter password: ";
    cin >> pwd;
    if (username == "admin" && pwd == "admin123") { isAdmin = true; return true; }
    if (pwd == "student123") { isAdmin = false; return true; }
    cout << "Invalid credentials.\n";
    return false;
}

int main() {
    string user; bool isAdmin;
    if (!login(user, isAdmin)) return 0;
    Library lib(user, isAdmin);
    int choice;
    do {
        cout << "\n===== Library Menu =====\n";
        if (isAdmin) cout << "1. Add Book\n";
        cout << "2. View Books\n3. Search Book by Title (Fuzzy)\n";
        cout << "4. Issue Book\n5. Return Book\n6. Rate a Book\n0. Exit\nChoose an option: ";
        cin >> choice;
        switch (choice) {
            case 1: if (isAdmin) lib.addBook(); break;
            case 2: lib.viewBooks(); break;
            case 3: lib.searchFuzzyTitle(); break;
            case 4: lib.issueBook(); break;
            case 5: lib.returnBook(); break;
            case 6: lib.rateBook(); break;
            case 0: cout << "Exiting...\n"; break;
            default: cout << "Invalid option.\n";
        }
    } while (choice != 0);
    return 0;
}
