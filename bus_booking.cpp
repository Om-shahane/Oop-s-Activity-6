// Bus Booking Management System


#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <fstream>
#include <limits>
using namespace std;

// Base class: Person
class Person {
protected:
    string name, phone;
public:
    Person(string n, string p) : name(move(n)), phone(move(p)) {}
    virtual ~Person() = default;
    virtual void showRole() const = 0;           // polymorphism
    const string& getName() const  { return name; }
    const string& getPhone() const { return phone; }
};


class Passenger : public Person {  // Derived: Passenger 
    int age;
public:
    Passenger(string n, string p, int a) : Person(move(n), move(p)), age(a) {}
    void showRole() const override { cout << "Passenger: " << name << " (" << age << " yrs)\n"; }
    int getAge() const { return age; }
    // Senior citizen / child discount
    double discount() const { return (age >= 60 || age <= 12) ? 0.10 : 0.0; }
};

// ---------------- Abstract class: Bus ----------------
class Bus {
protected:
    int id;
    string source, destination;
    int departureHour;              
    vector<bool> booked;             
public:
    Bus(int i, string s, string d, int hour, int seats)
        : id(i), source(move(s)), destination(move(d)), departureHour(hour), booked(seats, false) {}
    virtual ~Bus() = default;

    virtual string type() const = 0;
    virtual double baseFare() const = 0;
    virtual double seatFare(int /*seatNo*/) const { return baseFare(); }   

    // INNOVATION: dynamic pricing based on demand + peak hours
    double currentFare(int seatNo) const {
        double fare = seatFare(seatNo);
        double occ = occupancy();
        if (occ >= 0.90)      fare *= 1.30;
        else if (occ >= 0.70) fare *= 1.15;
        if (departureHour >= 17 && departureHour <= 21) fare *= 1.10;      
        return fare;
    }

    double occupancy() const {
        int c = 0;
        for (bool b : booked) if (b) c++;
        return (double)c / booked.size();
    }
    int totalSeats() const { return (int)booked.size(); }
    int availableSeats() const { return totalSeats() - (int)(occupancy() * totalSeats() + 0.5); }
    bool isBooked(int s) const { return booked[s - 1]; }
    bool validSeat(int s) const { return s >= 1 && s <= totalSeats(); }
    void setBooked(int s, bool v) { booked[s - 1] = v; }

    int getId() const { return id; }
    const string& getSource() const { return source; }
    const string& getDestination() const { return destination; }
    int getHour() const { return departureHour; }

    void display() const {
        cout << left << setw(5) << id << setw(10) << source << setw(12) << destination
             << setw(10) << type() << setw(4) << departureHour << ":00  "
             << setw(8) << availableSeats() << "Rs." << fixed << setprecision(0) << baseFare() << "\n";
    }

    // Seat map: [01] free, [XX] booked
    virtual void showSeatMap() const {
        cout << "\nSeat map for Bus " << id << " (" << type() << ")  [XX] = booked\n\n";
        for (int i = 1; i <= totalSeats(); i++) {
            if (booked[i - 1]) cout << "[XX] ";
            else               cout << "[" << setw(2) << setfill('0') << i << setfill(' ') << "] ";
            if (i % 4 == 2) cout << "   ";           // aisle
            if (i % 4 == 0) cout << "\n";
        }
        cout << "\n";
    }
};

// ---------------- Derived buses ----------------
class NonACBus : public Bus {
public:
    NonACBus(int i, string s, string d, int h) : Bus(i, move(s), move(d), h, 40) {}
    string type() const override { return "Non-AC"; }
    double baseFare() const override { return 400; }
};

class ACBus : public Bus {
public:
    ACBus(int i, string s, string d, int h) : Bus(i, move(s), move(d), h, 36) {}
    string type() const override { return "AC"; }
    double baseFare() const override { return 700; }
};

class SleeperBus : public Bus {
public:
    SleeperBus(int i, string s, string d, int h) : Bus(i, move(s), move(d), h, 24) {}
    string type() const override { return "Sleeper"; }
    double baseFare() const override { return 1000; }
    double seatFare(int seatNo) const override { return seatNo % 2 ? baseFare() + 150 : baseFare(); }
};

// ---------------- Ticket ----------------
class Ticket {
    static int counter;
public:
    int ticketId;
    string passengerName, phone;
    int busId, seatNo;
    double fare;
    bool active;

    Ticket(const Passenger& p, int bus, int seat, double f)
        : ticketId(++counter), passengerName(p.getName()), phone(p.getPhone()),
          busId(bus), seatNo(seat), fare(f), active(true) {}

    void print(const Bus& b) const {
        cout << "\n=========== BUS TICKET ===========\n"
             << "Ticket No : " << ticketId << "\n"
             << "Passenger : " << passengerName << "  (" << phone << ")\n"
             << "Route     : " << b.getSource() << " -> " << b.getDestination() << "\n"
             << "Bus       : " << b.getId() << " (" << b.type() << ")  Departs " << b.getHour() << ":00\n"
             << "Seat No   : " << seatNo << "\n"
             << "Fare Paid : Rs." << fixed << setprecision(2) << fare << "\n"
             << "Status    : " << (active ? "CONFIRMED" : "CANCELLED") << "\n"
             << "==================================\n";
    }
    void saveToFile(const Bus& b) const {
        ofstream f("ticket_" + to_string(ticketId) + ".txt");
        f << "Ticket " << ticketId << " | " << passengerName << " | "
          << b.getSource() << "->" << b.getDestination()
          << " | Bus " << busId << " | Seat " << seatNo << " | Rs." << fare << "\n";
    }
};
int Ticket::counter = 1000;

// ---------------- Booking System ----------------
class BookingSystem {
    vector<unique_ptr<Bus>> buses;
    vector<Ticket> tickets;

    Bus* findBus(int id) {
        for (auto& b : buses) if (b->getId() == id) return b.get();
        return nullptr;
    }
public:
    BookingSystem() {
        buses.push_back(make_unique<NonACBus>(101, "Pune", "Mumbai", 7));
        buses.push_back(make_unique<ACBus>(102, "Pune", "Mumbai", 18));
        buses.push_back(make_unique<SleeperBus>(103, "Pune", "Goa", 21));
        buses.push_back(make_unique<ACBus>(104, "Mumbai", "Nashik", 9));
        buses.push_back(make_unique<NonACBus>(105, "Pune", "Nashik", 14));
    }

    void showAllBuses() const {
        cout << "\n" << left << setw(5) << "ID" << setw(10) << "From" << setw(12) << "To"
             << setw(10) << "Type" << setw(10) << "Time" << setw(8) << "Seats" << "Base Fare\n"
             << string(65, '-') << "\n";
        for (auto& b : buses) b->display();
    }

    void search() {
        string s, d;
        cout << "From: "; cin >> s;
        cout << "To  : "; cin >> d;
        bool found = false;
        cout << "\n";
        for (auto& b : buses)
            if (b->getSource() == s && b->getDestination() == d) { b->display(); found = true; }
        if (!found) cout << "No buses found on this route.\n";
    }

    void book() {
        showAllBuses();
        int id;
        cout << "\nEnter Bus ID: "; cin >> id;
        Bus* bus = findBus(id);
        if (!bus) { cout << "Invalid bus.\n"; return; }
        if (bus->availableSeats() == 0) { cout << "Bus is full.\n"; return; }

        bus->showSeatMap();
        string name, phone; int age, seat;
        cout << "Passenger name : "; cin >> name;
        cout << "Phone          : "; cin >> phone;
        cout << "Age            : "; cin >> age;
        cout << "Seat number    : "; cin >> seat;

        if (!bus->validSeat(seat))  { cout << "Invalid seat number.\n"; return; }
        if (bus->isBooked(seat))    { cout << "Seat already booked.\n"; return; }

        Passenger p(name, phone, age);
        double fare = bus->currentFare(seat);
        fare -= fare * p.discount();

        cout << "\nFare (incl. dynamic pricing" << (p.discount() > 0 ? " & 10% discount" : "")
             << "): Rs." << fixed << setprecision(2) << fare << "\nConfirm? (y/n): ";
        char c; cin >> c;
        if (c != 'y' && c != 'Y') { cout << "Booking aborted.\n"; return; }

        bus->setBooked(seat, true);
        tickets.emplace_back(p, id, seat, fare);
        tickets.back().print(*bus);
        tickets.back().saveToFile(*bus);
        cout << "(Ticket saved to ticket_" << tickets.back().ticketId << ".txt)\n";
    }

    void cancel() {
        int tid, hrs;
        cout << "Ticket number: "; cin >> tid;
        for (auto& t : tickets) {
            if (t.ticketId == tid && t.active) {
                cout << "Hours before departure: "; cin >> hrs;
                double pct = hrs > 24 ? 0.90 : (hrs > 6 ? 0.50 : 0.0);   // refund policy
                t.active = false;
                findBus(t.busId)->setBooked(t.seatNo, false);
                cout << "Ticket cancelled. Refund: Rs." << fixed << setprecision(2)
                     << t.fare * pct << " (" << (int)(pct * 100) << "%)\n";
                return;
            }
        }
        cout << "Active ticket not found.\n";
    }

    void viewTicket() {
        int tid;
        cout << "Ticket number: "; cin >> tid;
        for (auto& t : tickets)
            if (t.ticketId == tid) { t.print(*findBus(t.busId)); return; }
        cout << "Ticket not found.\n";
    }

    void viewSeats() {
        int id; cout << "Bus ID: "; cin >> id;
        Bus* b = findBus(id);
        b ? b->showSeatMap() : void(cout << "Invalid bus.\n");
    }
};

// ---------------- main ----------------
int main() {
    BookingSystem sys;
    int choice;
    do {
        cout << "\n===== BUS TICKET BOOKING SYSTEM =====\n"
             << "1. View all buses\n2. Search by route\n3. View seat map\n"
             << "4. Book ticket\n5. Cancel ticket\n6. View ticket\n0. Exit\nChoice: ";
        if (!(cin >> choice)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); continue; }
        switch (choice) {
            case 1: sys.showAllBuses(); break;
            case 2: sys.search();       break;
            case 3: sys.viewSeats();    break;
            case 4: sys.book();         break;
            case 5: sys.cancel();       break;
            case 6: sys.viewTicket();   break;
            case 0: cout << "Thank you! Happy journey.\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
    return 0;
}
