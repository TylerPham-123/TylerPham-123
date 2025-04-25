// BankSim3000
//
// The purpose of this bank and teller simulation is to help a bank manager to make an informed
// decision on how many tellers to hire at a branch with longer than desired wait times.
// This version tracks customer wait times directly.

#include <iostream>
#include <variant>
#include <vector>
#include <queue>
#include <cassert>
#include <algorithm>
#include <optional>
#include <numeric> // For std::accumulate

using namespace std;

const size_t MIN_TELLERS = 1; // This makes sure there is always atleast 1 teller.
const size_t MAX_TELLERS = 5; // This makes sure there are only 5 tellers at max.

// Integer time units.
using Time = int; // Help improve code readability and make it clearer.

// We will be tracking teller state in a variable std::vector.
using TellerIndex = size_t;

// Arrival event containing only the arrival and transaction times.
struct ArrivalEvent {
    Time arrivalTime;
    Time transactionTime;
};

// Include arrival time for wait time calculation.
struct Customer {
    ArrivalEvent arrivalEvent;
    Time entryTime; // Time the customer entered the bank line*
};

// A departure event including the expected departure time and the
// teller being departed from.
struct DepartureEvent {
    Time departureTime;
    TellerIndex tellerIndex;
    Customer customer; // Include the customer in the departure event*
};

// Either an arrival or departure event. Variant can be thought of as a safer union.
using Event = std::variant<ArrivalEvent, DepartureEvent>; // Help hold and, at the same time, keep track of the two values.

// Helper function to get the time from either an arrival or departure event.
Time get_event_time(const Event& e) {
    if(holds_alternative<ArrivalEvent>(e)) {
        return get<ArrivalEvent>(e).arrivalTime; // If (e) is an ArrivalEvent, get the arrivalTime and return it.
    }
    return get<DepartureEvent>(e).departureTime; // If (e) is a DepartureEvent, get the departureTime and return it.
}

// A compare functor / function object for the priority queue. Creates a min-heap.
struct CompareEvent {
    bool operator()(const Event& e1, const Event& e2) {
        return get_event_time(e1) < get_event_time(e2); // This will consider the event with a lower value as a higher priority.
    }
};

// Holds availability and is simpler now as we directly track wait times.
class Teller {
private:
    bool isBusy = false;

public:
    Teller() : isBusy(false) {} // A public constructor helps set isBusy status to false by default

    bool isAvailable() const {
        return !isBusy;
    } // const means to make isAvailable only able to read the value of isBusy but does not modify it.

    void startService() {
        isBusy = true;
    }

    void stopService() {
        isBusy = false;
    }
};

struct SimulationResults {
    vector<Time> customerWaitTimes; // Declares a vector type Time (or int) named customerWaitTimes

    // Finds the average customer wait time.
    double averageWaitTime() const {
        if (customerWaitTimes.empty()) {
            return 0.0;
        }
        return static_cast<double>(accumulate(customerWaitTimes.begin(), customerWaitTimes.end(), 0)) / customerWaitTimes.size();
    }

    // Finds the maximum customer wait time.
    Time maxWaitTime() const {
        if (customerWaitTimes.empty()) {
            return 0;
        }
        return *max_element(customerWaitTimes.begin(), customerWaitTimes.end());
    }

    SimulationResults(vector<Time> customerWaitTimes) : customerWaitTimes(customerWaitTimes) { }
};

// A line of customers waiting to be served by a teller.
using BankLine = queue<Customer>;
// The event priority queue used by the simulation.
using EventQueue = priority_queue<Event, vector<Event>, CompareEvent>;
// A list of arrival events used to start the simulation.
using SimulationInput = vector<ArrivalEvent>;

class BankSim3000 {
private:
    // Input is stored locally to help restart the simulation for multiple tellers.
    SimulationInput simulationInput;
    // The event queue. Initially this is loaded with the simulation input.
    EventQueue eventQueue;
    // The bank line. Initially this is empty.
    BankLine bankLine;

    // One teller simulation state for each teller.
    vector<Teller> tellers;

    // Stores the wait times of completed customers.
    vector<Time> completedCustomerWaitTimes;

    // Resets the tellers vector to the requested size and initialized to the default constructor.
    void resetTellers(size_t tellerCount) {
        if(tellerCount != tellers.size()) {
            tellers.reserve(tellerCount); // reserve never decreases size.
        }

        tellers.clear();

        for(size_t i=0; i<tellerCount; ++i) {
            tellers.emplace_back();
        } // emplace_back() takes the arguments that would be used to construct a new object directly within the container.
    }

    // Clears the bank line.
    void clearBankLine() {
        assert(bankLine.empty()); // It should already be cleared after a complete simulation run.
        while(!bankLine.empty()) {
            bankLine.pop();
        } // pop pops out any customer that is still in the line.
    }

    // Clears the event queue and initializes it to our input data.
    void setupEventQueue() {
        assert(eventQueue.empty()); // Should also already be empty after a complete simulation.
        while(!eventQueue.empty()) {
            eventQueue.pop();
        }

        // Load all the input data from simulationInput into the event priority queue.
        for (const ArrivalEvent& arrival : simulationInput) {
            eventQueue.push(arrival);
        }
    }

    // Clears the completed customer wait times.
    void clearCompletedWaitTimes() {
        completedCustomerWaitTimes.clear();
    }

    // Sets up the simulation for the given number of tellers.
    void setupSimulation(size_t tellerCount) {
        if(tellerCount < MIN_TELLERS) {
            throw invalid_argument("Teller count must >= " + to_string(MIN_TELLERS));
        }
        if(tellerCount > MAX_TELLERS) {
            throw invalid_argument("Teller count must be <= " + to_string(MAX_TELLERS));
        }

        setupEventQueue();

        resetTellers(tellerCount);

        clearBankLine();

        clearCompletedWaitTimes();
    }

    // Processes either an arrival or a departure event.
    void processEvent(Time currentTime, const Event & e) {
        if(holds_alternative<ArrivalEvent>(e)) {
            ArrivalEvent arrivalEvent = get<ArrivalEvent>(e);
            processArrival(currentTime, arrivalEvent);
        } else {
            assert(holds_alternative<DepartureEvent>(e));
            DepartureEvent departureEvent = get<DepartureEvent>(e);
            processDeparture(currentTime, departureEvent);
        }
    }

    // Helper used by processArrival.
    // Returns the index of an available teller or nullopt if all are busy.
    optional<size_t> searchAvailableTellers() const {
        for(size_t i=0; i<tellers.size(); ++i) {
            if(tellers[i].isAvailable()) {
                return i;
            }
        }
        return nullopt;
    }

    // Process arrival events.
    //
    // If teller is not available, place customer at the end of the bank line.
    // Otherwise, assign customer to teller and add a new departure event.
    void processArrival(Time currentTime, const ArrivalEvent& arrivalEvent) {
        auto availableTellerIndex = searchAvailableTellers();

        if (availableTellerIndex.has_value()) {
            // Found an available teller, so assign the customer to them.
            TellerIndex tellerIndex = availableTellerIndex.value();
            tellers[tellerIndex].startService();

            // Create a new customer object.
            Customer customer{arrivalEvent, currentTime};

            // Calculate the departure time.
            Time departureTime = currentTime + arrivalEvent.transactionTime;

            // Add a departure event to the event queue.
            eventQueue.push(DepartureEvent{departureTime, tellerIndex, customer});
        } else {
            // No teller available, add the customer to the bank line with their entry time.
            bankLine.push(Customer{arrivalEvent, currentTime});
        }
    }

    // Process departure events.
    //
    // If bank line is empty then the teller becomes available.
    // Otherwise, take the next customer off the bank line and enqueue a new departure
    // event into the event priority queue.
    void processDeparture(Time currentTime, const DepartureEvent& departureEvent) {
        size_t tellerIndex = departureEvent.tellerIndex;
        Customer departingCustomer = departureEvent.customer;

        // Calculate and store the wait time of the departing customer.
        Time waitTime = currentTime - departingCustomer.entryTime - departingCustomer.arrivalEvent.transactionTime;
        if (waitTime > 0) { // Ensure wait time is not negative (can happen in edge cases)
            completedCustomerWaitTimes.push_back(waitTime);
        }

        if (!bankLine.empty()) {
            // Get the next customer from the bank line.
            Customer nextCustomer = bankLine.front();
            bankLine.pop();

            // Assign the customer to the now available teller.
            tellers[tellerIndex].startService();

            // Calculate the departure time for this new customer.
            Time nextDepartureTime = currentTime + nextCustomer.arrivalEvent.transactionTime;

            // Add a new departure event to the event queue.
            eventQueue.push(DepartureEvent{nextDepartureTime, tellerIndex, nextCustomer});
        } else {
            // If the bank line is empty, the teller becomes available.
            tellers[tellerIndex].stopService();
        }
    }

    // Runs the simulation.
    void runSimulation() {
        while(!eventQueue.empty()) {
            // Remove event.
            Event e = eventQueue.top();
            eventQueue.pop();

            processEvent(get_event_time(e), e);
        }
    }

    SimulationResults gatherResults() const {
        return SimulationResults {completedCustomerWaitTimes};
    }

public:

    BankSim3000(SimulationInput simulationInput) : simulationInput(simulationInput) { }

    SimulationResults run(size_t tellerCount) {
        setupSimulation(tellerCount);

        runSimulation();

        return gatherResults();
    }
};

int main() {
    // Do not change the input.
    SimulationInput SimulationInput00 = {{20, 6}, {22, 4}, {23, 2}, {30, 3}};

    BankSim3000 bankSim(SimulationInput00);

    cout << "Results with 1 teller: Average Wait Time = " << bankSim.run(1).averageWaitTime() << ", Max Wait Time = " << bankSim.run(1).maxWaitTime() << endl;
    cout << "Results with 2 tellers: Average Wait Time = " << bankSim.run(2).averageWaitTime() << ", Max Wait Time = " << bankSim.run(2).maxWaitTime() << endl;
    cout << "Results with 3 tellers: Average Wait Time = " << bankSim.run(3).averageWaitTime() << ", Max Wait Time = " << bankSim.run(3).maxWaitTime() << endl;
    cout << "Results with 4 tellers: Average Wait Time = " << bankSim.run(4).averageWaitTime() << ", Max Wait Time = " << bankSim.run(4).maxWaitTime() << endl;
    cout << "Results with 5 tellers: Average Wait Time = " << bankSim.run(5).averageWaitTime() << ", Max Wait Time = " << bankSim.run(5).maxWaitTime() << endl;

    return 0;
}
