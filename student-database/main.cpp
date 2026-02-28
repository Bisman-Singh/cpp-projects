#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <cstring>

struct Student {
    int id = 0;
    char name[64] = {};
    int age = 0;
    char grade = 'F';
    double gpa = 0.0;

    void set_name(const std::string& n) {
        std::strncpy(name, n.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    }

    std::string get_name() const { return std::string(name); }
};

class StudentDB {
public:
    explicit StudentDB(const std::string& filename) : filename_(filename) {
        load();
    }

    bool add(const Student& s) {
        if (find_index(s.id) >= 0) {
            std::cout << "Error: Student with ID " << s.id << " already exists.\n";
            return false;
        }
        students_.push_back(s);
        save();
        return true;
    }

    void view_all() const {
        if (students_.empty()) {
            std::cout << "No students in database.\n";
            return;
        }
        print_header();
        for (auto& s : students_) {
            print_student(s);
        }
        std::cout << "\nTotal: " << students_.size() << " student(s)\n";
    }

    Student* search_by_id(int id) {
        int idx = find_index(id);
        return idx >= 0 ? &students_[idx] : nullptr;
    }

    std::vector<Student*> search_by_name(const std::string& query) {
        std::vector<Student*> results;
        std::string lower_query = to_lower(query);
        for (auto& s : students_) {
            if (to_lower(s.get_name()).find(lower_query) != std::string::npos) {
                results.push_back(&s);
            }
        }
        return results;
    }

    bool update(int id, const Student& updated) {
        int idx = find_index(id);
        if (idx < 0) return false;
        students_[idx] = updated;
        save();
        return true;
    }

    bool remove(int id) {
        int idx = find_index(id);
        if (idx < 0) return false;
        students_.erase(students_.begin() + idx);
        save();
        return true;
    }

    void sort_by_name() {
        std::sort(students_.begin(), students_.end(),
                  [](const Student& a, const Student& b) {
                      return std::strcmp(a.name, b.name) < 0;
                  });
        save();
    }

    void sort_by_gpa() {
        std::sort(students_.begin(), students_.end(),
                  [](const Student& a, const Student& b) {
                      return a.gpa > b.gpa;
                  });
        save();
    }

    void sort_by_id() {
        std::sort(students_.begin(), students_.end(),
                  [](const Student& a, const Student& b) {
                      return a.id < b.id;
                  });
        save();
    }

    void statistics() const {
        if (students_.empty()) {
            std::cout << "No students in database.\n";
            return;
        }

        double total_gpa = 0;
        std::map<char, int> grade_dist;
        for (auto& s : students_) {
            total_gpa += s.gpa;
            grade_dist[s.grade]++;
        }

        std::cout << "\n--- Statistics ---\n";
        std::cout << "Total students: " << students_.size() << "\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Average GPA:    " << (total_gpa / students_.size()) << "\n";

        double max_gpa = std::max_element(students_.begin(), students_.end(),
            [](const Student& a, const Student& b) { return a.gpa < b.gpa; })->gpa;
        double min_gpa = std::min_element(students_.begin(), students_.end(),
            [](const Student& a, const Student& b) { return a.gpa < b.gpa; })->gpa;

        std::cout << "Highest GPA:    " << max_gpa << "\n";
        std::cout << "Lowest GPA:     " << min_gpa << "\n";

        std::cout << "\nGrade Distribution:\n";
        for (char g : {'A', 'B', 'C', 'D', 'F'}) {
            int count = grade_dist.count(g) ? grade_dist[g] : 0;
            std::cout << "  " << g << ": " << count << " student(s)\n";
        }
    }

    static void print_header() {
        std::cout << std::left
                  << std::setw(8) << "ID"
                  << std::setw(25) << "Name"
                  << std::setw(6) << "Age"
                  << std::setw(8) << "Grade"
                  << std::setw(8) << "GPA" << "\n";
        std::cout << std::string(55, '-') << "\n";
    }

    static void print_student(const Student& s) {
        std::cout << std::left
                  << std::setw(8) << s.id
                  << std::setw(25) << s.get_name()
                  << std::setw(6) << s.age
                  << std::setw(8) << s.grade
                  << std::fixed << std::setprecision(2)
                  << std::setw(8) << s.gpa << "\n";
    }

private:
    std::string filename_;
    std::vector<Student> students_;

    int find_index(int id) const {
        for (size_t i = 0; i < students_.size(); ++i) {
            if (students_[i].id == id) return static_cast<int>(i);
        }
        return -1;
    }

    static std::string to_lower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    void load() {
        std::ifstream file(filename_, std::ios::binary);
        if (!file.is_open()) return;
        Student s;
        while (file.read(reinterpret_cast<char*>(&s), sizeof(Student))) {
            students_.push_back(s);
        }
    }

    void save() const {
        std::ofstream file(filename_, std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot write to " << filename_ << "\n";
            return;
        }
        for (auto& s : students_) {
            file.write(reinterpret_cast<const char*>(&s), sizeof(Student));
        }
    }
};

void clear_input() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int read_int(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) {
            clear_input();
            return val;
        }
        std::cout << "Invalid input. Please enter a number.\n";
        clear_input();
    }
}

double read_double(const std::string& prompt) {
    double val;
    while (true) {
        std::cout << prompt;
        if (std::cin >> val) {
            clear_input();
            return val;
        }
        std::cout << "Invalid input. Please enter a number.\n";
        clear_input();
    }
}

char read_grade(const std::string& prompt) {
    char g;
    while (true) {
        std::cout << prompt;
        if (std::cin >> g) {
            clear_input();
            g = static_cast<char>(std::toupper(g));
            if (g == 'A' || g == 'B' || g == 'C' || g == 'D' || g == 'F') return g;
        }
        std::cout << "Invalid grade. Enter A, B, C, D, or F.\n";
        clear_input();
    }
}

Student read_student_info(bool ask_id = true) {
    Student s;
    if (ask_id) {
        s.id = read_int("  ID: ");
    }
    std::cout << "  Name: ";
    std::string name;
    std::getline(std::cin, name);
    s.set_name(name);
    s.age = read_int("  Age: ");
    s.grade = read_grade("  Grade (A-F): ");
    s.gpa = read_double("  GPA (0.0-4.0): ");
    return s;
}

void interactive_menu() {
    StudentDB db("students.dat");

    while (true) {
        std::cout << "\n========== Student Database ==========\n";
        std::cout << "  1. Add Student\n";
        std::cout << "  2. View All Students\n";
        std::cout << "  3. Search by ID\n";
        std::cout << "  4. Search by Name\n";
        std::cout << "  5. Update Student\n";
        std::cout << "  6. Delete Student\n";
        std::cout << "  7. Sort Students\n";
        std::cout << "  8. Statistics\n";
        std::cout << "  0. Exit\n";
        std::cout << "=======================================\n";

        int choice = read_int("Choose: ");

        switch (choice) {
            case 1: {
                std::cout << "\n--- Add Student ---\n";
                Student s = read_student_info();
                if (db.add(s)) {
                    std::cout << "Student added successfully.\n";
                }
                break;
            }
            case 2: {
                std::cout << "\n--- All Students ---\n";
                db.view_all();
                break;
            }
            case 3: {
                int id = read_int("Enter ID: ");
                Student* s = db.search_by_id(id);
                if (s) {
                    StudentDB::print_header();
                    StudentDB::print_student(*s);
                } else {
                    std::cout << "Student not found.\n";
                }
                break;
            }
            case 4: {
                std::cout << "Enter name to search: ";
                std::string query;
                std::getline(std::cin, query);
                auto results = db.search_by_name(query);
                if (results.empty()) {
                    std::cout << "No matching students found.\n";
                } else {
                    StudentDB::print_header();
                    for (auto* s : results) {
                        StudentDB::print_student(*s);
                    }
                    std::cout << results.size() << " result(s) found.\n";
                }
                break;
            }
            case 5: {
                int id = read_int("Enter student ID to update: ");
                Student* existing = db.search_by_id(id);
                if (!existing) {
                    std::cout << "Student not found.\n";
                    break;
                }
                std::cout << "Current record:\n";
                StudentDB::print_header();
                StudentDB::print_student(*existing);
                std::cout << "\nEnter new details:\n";
                Student updated = read_student_info(false);
                updated.id = id;
                if (db.update(id, updated)) {
                    std::cout << "Student updated successfully.\n";
                }
                break;
            }
            case 6: {
                int id = read_int("Enter student ID to delete: ");
                if (db.remove(id)) {
                    std::cout << "Student deleted.\n";
                } else {
                    std::cout << "Student not found.\n";
                }
                break;
            }
            case 7: {
                std::cout << "Sort by: 1) Name  2) GPA  3) ID\n";
                int sort_choice = read_int("Choose: ");
                switch (sort_choice) {
                    case 1: db.sort_by_name(); break;
                    case 2: db.sort_by_gpa(); break;
                    case 3: db.sort_by_id(); break;
                    default: std::cout << "Invalid choice.\n"; continue;
                }
                std::cout << "Sorted.\n";
                db.view_all();
                break;
            }
            case 8: {
                db.statistics();
                break;
            }
            case 0: {
                std::cout << "Goodbye!\n";
                return;
            }
            default:
                std::cout << "Invalid choice.\n";
        }
    }
}

int main() {
    interactive_menu();
    return 0;
}
