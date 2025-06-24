#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <memory>
#include <chrono> // For std::chrono::year_month_day
#include <iomanip> // For std::put_time, std::get_time (if using old C-style time) or formatting chrono
#include <type_traits>
#include <sstream> // For converting chrono dates to string for display

// --- Forward Declarations and Type Aliases ---
class Dog;
using DogPtr = std::shared_ptr<Dog>;
using DogWeakPtr = std::weak_ptr<Dog>;

// Dog Gender enum (unchanged)
enum class Gender {
    MALE,
    FEMALE,
    UNKNOWN
};

std::string genderToString(Gender g) {
    switch (g) {
        case Gender::MALE: return "Male";
        case Gender::FEMALE: return "Female";
        case Gender::UNKNOWN: return "Unknown";
        default: return "Invalid";
    }
}

// --- Date Conversion Helpers (Optional but useful) ---
// Convert ymd to string "YYYY-MM-DD"
std::string ymdToString(const std::chrono::year_month_day& ymd) {
    if (!ymd.ok()) return "Invalid Date";
    std::ostringstream oss;
    oss << ymd.year() << "-"
        << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.month()) << "-"
        << std::setw(2) << std::setfill('0') << static_cast<unsigned>(ymd.day());
    return oss.str();
}

// Convert string "YYYY-MM-DD" to ymd
std::chrono::year_month_day stringToYmd(const std::string& s) {
    std::istringstream iss(s);
    int year, month, day;
    char dash1, dash2;
    iss >> year >> dash1 >> month >> dash2 >> day;
    if (iss.fail() || dash1 != '-' || dash2 != '-') {
        return std::chrono::year_month_day{}; // Return invalid date
    }
    return std::chrono::year_month_day(std::chrono::year(year), std::chrono::month(month), std::chrono::day(day));
}

// --- Dog Class Definition ---
class Dog : public std::enable_shared_from_this<Dog> {
public:
    std::string name;
    int id;
    Gender gender;
    std::chrono::year_month_day birth_date;
    std::chrono::year_month_day death_date; // Default-initialized to invalid date if not set

    struct Parents {
        DogWeakPtr father;
        DogWeakPtr mother;
    } parents;
    
    std::list<DogPtr> children;

    // Owner and Adoption Data
    // Assuming Owner is just a string name for simplicity now.
    // Could be a separate Owner class/struct if more detail is needed.
    std::string owner_name;
    std::chrono::year_month_day adoption_date; // Default-initialized to invalid date if not set

    // Modified Constructor: now takes std::chrono::year_month_day objects
    Dog(const std::string& name, Gender gender,
        const std::chrono::year_month_day& birth_date, int id_val)
        : name(name), gender(gender), birth_date(birth_date), id(id_val),
          death_date(std::chrono::year_month_day{}), // Initialize as invalid date
          adoption_date(std::chrono::year_month_day{}) // Initialize as invalid date
    {
        if (!birth_date.ok()) {
            std::cerr << "Warning: Invalid birth date provided for " << name << ". ID: " << id << std::endl;
        }
    }
    virtual ~Dog() = default;

    // --- Age and Date Calculations ---

    // Calculate current age in years
    int getAgeInYears(const std::chrono::year_month_day& current_date) const {
        if (!birth_date.ok()) return -1; // Invalid birth date
        
        auto age_years = current_date.year() - birth_date.year();
        // Adjust for birthday not yet passed this year
        if (current_date.month() < birth_date.month() ||
            (current_date.month() == birth_date.month() && current_date.day() < birth_date.day())) {
            age_years--;
        }
        return static_cast<int>(age_years.count());
    }

    // Setters for death_date and adoption_date (with validation)
    void setDeathDate(const std::chrono::year_month_day& date) {
        if (!date.ok()) {
            std::cerr << "Error: Invalid death date provided for " << name << ".\n";
            return;
        }
        if (date < birth_date) {
            std::cerr << "Error: Death date (" << ymdToString(date) << ") cannot be before birth date (" << ymdToString(birth_date) << ") for " << name << ".\n";
            return;
        }
        death_date = date;
    }

    void setAdoptionDate(const std::string& owner, const std::chrono::year_month_day& date) {
        if (!date.ok()) {
            std::cerr << "Error: Invalid adoption date provided for " << name << ".\n";
            return;
        }
        if (date < birth_date) {
            std::cerr << "Error: Adoption date (" << ymdToString(date) << ") cannot be before birth date (" << ymdToString(birth_date) << ") for " << name << ".\n";
            return;
        }
        owner_name = owner;
        adoption_date = date;
    }

    // Check if the dog is alive
    bool isAlive() const {
        return !death_date.ok(); // death_date is invalid if dog is alive
    }

    // --- Parent/Child Relationship Management ---
    void addParent(DogPtr parent) {
        if (!parent) {
            std::cerr << "Error: Attempted to add a null parent to " << name << ".\n";
            return;
        }
        // Simplified for now, relying on setFather/setMother from PedigreeManager
        // If this method is called directly, ensure proper gender handling.
        if (parent->gender == Gender::MALE) {
            setFather(parent);
        } else if (parent->gender == Gender::FEMALE) {
            setMother(parent);
        } else {
            std::cerr << "Error: Cannot assign parent " << parent->name << " with unknown gender to " << name << ".\n";
        }
    }

    void addChild(DogPtr child) {
        if (!child) {
            std::cerr << "Error: Attempted to add a null child to " << name << ".\n";
            return;
        }
        children.push_back(child);
    }

    void setFather(DogPtr father_dog) {
        if (!father_dog || father_dog->gender != Gender::MALE) {
            std::cerr << "Error: Invalid father dog for " << name << ".\n";
            return;
        }
        // Validation: Father must be older or born at the same time (not younger)
        if (birth_date < father_dog->birth_date) {
            std::cerr << "Error: " << name << "'s birth date (" << ymdToString(birth_date)
                      << ") cannot be before father " << father_dog->name << "'s birth date ("
                      << ymdToString(father_dog->birth_date) << ").\n";
            return;
        }
        parents.father = father_dog;
        if (auto self_shared = shared_from_this()) {
            father_dog->addChild(self_shared);
        }
    }

    void setMother(DogPtr mother_dog) {
        if (!mother_dog || mother_dog->gender != Gender::FEMALE) {
            std::cerr << "Error: Invalid mother dog for " << name << ".\n";
            return;
        }
        // Validation: Mother must be older or born at the same time (not younger)
        if (birth_date < mother_dog->birth_date) {
            std::cerr << "Error: " << name << "'s birth date (" << ymdToString(birth_date)
                      << ") cannot be before mother " << mother_dog->name << "'s birth date ("
                      << ymdToString(mother_dog->birth_date) << ").\n";
            return;
        }
        parents.mother = mother_dog;
        if (auto self_shared = shared_from_this()) {
            mother_dog->addChild(self_shared);
        }
    }
    
    // --- Print Information ---
    virtual void printInfo() const {
        std::cout << "ID: " << id
                  << ", Name: " << name
                  << ", Gender: " << genderToString(gender)
                  << ", Birth: " << ymdToString(birth_date);
        
        if (!death_date.ok()) { // Dog is alive
            std::cout << ", Age: " << getAgeInYears(std::chrono::year_month_day(std::chrono::ceil<std::chrono::days>(std::chrono::system_clock::now()))) << " years"; // Calculate age to today
        } else {
            std::cout << ", Death: " << ymdToString(death_date);
        }

        std::cout << ", Parents: [";
        bool first_parent = true;
        if (auto father_shared = parents.father.lock()) {
            std::cout << "F: " << father_shared->name;
            first_parent = false;
        } else if (!parents.father.expired()) { // weak_ptr is valid but points to dead obj
            std::cout << "F: [Unknown/Deceased]";
            first_parent = false;
        }

        if (auto mother_shared = parents.mother.lock()) {
            if (!first_parent) std::cout << ", ";
            std::cout << "M: " << mother_shared->name;
        } else if (!parents.mother.expired()) {
            if (!first_parent) std::cout << ", ";
            std::cout << "M: [Unknown/Deceased]";
        }
        std::cout << "]";
        std::cout << ", Children Count: " << children.size();

        if (!owner_name.empty()) {
            std::cout << ", Owner: " << owner_name << ", Adopted: " << ymdToString(adoption_date);
        }
    }
};

// --- Derived Dog Classes (unchanged, just constructor call adapts) ---
class Poodle : public Dog {
public:
    std::string fur_style;

    Poodle(const std::string& name, Gender gender, const std::chrono::year_month_day& birth_date,
           const std::string& fur_style_val, int id_val)
        : Dog(name, gender, birth_date, id_val), fur_style(fur_style_val) {}

    void printInfo() const override {
        Dog::printInfo();
        std::cout << "  - Breed: Poodle, Fur Style: " << fur_style << std::endl;
    }
    void performPoodleTrick() const {
        std::cout << name << " is performing a poodle trick!\n";
    }
};

class Bulldog : public Dog {
public:
    int wrinkle_count;

    Bulldog(const std::string& name, Gender gender, const std::chrono::year_month_day& birth_date,
            int wrinkles, int id_val)
        : Dog(name, gender, birth_date, id_val), wrinkle_count(wrinkles) {}

    void printInfo() const override {
        Dog::printInfo();
        std::cout << "  - Breed: Bulldog, Wrinkles: " << wrinkle_count << std::endl;
    }
};

// --- PedigreeManager Class (createDog adjusted for chrono) ---
class PedigreeManager {
public:
    std::list<DogPtr> all_dogs;
    int next_id = 1;

    template<typename T, typename... Args>
    DogPtr createDog(Args&&... args) {
        static_assert(std::is_base_of<Dog, T>::value, "T must derive from Dog");
        DogPtr new_dog = std::make_shared<T>(std::forward<Args>(args)..., next_id++);
        all_dogs.push_back(new_dog);
        return new_dog;
    }

    void setParents(DogPtr child, DogPtr father_dog, DogPtr mother_dog) {
        if (!child || !father_dog || !mother_dog) {
            std::cerr << "Error: Null dog pointer(s) provided for parent setting.\n";
            return;
        }
        if (father_dog->gender != Gender::MALE || mother_dog->gender != Gender::FEMALE) {
            std::cerr << "Error: Invalid genders for parents. Father must be Male, Mother must be Female.\n";
            return;
        }
        // These calls will now trigger the birth date validation in Dog::setFather/setMother
        child->setFather(father_dog);
        child->setMother(mother_dog);
    }

    // --- Breeding Check ---
    // A more robust breeding check would consider age, health, genetic compatibility, etc.
    // For now, it checks if they are alive and of correct gender.
    // Recommended breeding age (example): 1 to 8 years old for both
    static constexpr int MIN_BREEDING_AGE = 1;
    static constexpr int MAX_BREEDING_AGE = 8;

    bool canBreed(DogPtr male_dog, DogPtr female_dog) const {
        if (!male_dog || !female_dog) {
            std::cerr << "Error: Null dog pointer for breeding check.\n";
            return false;
        }
        if (male_dog->gender != Gender::MALE || female_dog->gender != Gender::FEMALE) {
            // std::cout << male_dog->name << " and " << female_dog->name << " cannot breed due to gender mismatch.\n";
            return false; // Genders must be male and female
        }
        if (!male_dog->isAlive() || !female_dog->isAlive()) {
            // std::cout << male_dog->name << " or " << female_dog->name << " is not alive.\n";
            return false; // Both must be alive
        }

        // Get current date for age calculation
        auto today = std::chrono::year_month_day(std::chrono::ceil<std::chrono::days>(std::chrono::system_clock::now()));
        int male_age = male_dog->getAgeInYears(today);
        int female_age = female_dog->getAgeInYears(today);

        if (male_age < MIN_BREEDING_AGE || male_age > MAX_BREEDING_AGE) {
            // std::cout << male_dog->name << " is outside ideal breeding age (" << male_age << " years).\n";
            return false;
        }
        if (female_age < MIN_BREEDING_AGE || female_age > MAX_BREEDING_AGE) {
            // std::cout << female_dog->name << " is outside ideal breeding age (" << female_age << " years).\n";
            return false;
        }
        
        // Add more complex checks here (e.g., inbreeding prevention by checking common ancestors)
        // This would involve traversing the pedigree tree upwards.

        return true; // All basic conditions met
    }

    void printAllDogs() const {
        std::cout << "\n--- All Dogs in Pedigree ---\n";
        for (const auto& dog : all_dogs) {
            dog->printInfo();
            std::cout << std::endl; // Added newline for better readability per dog
        }
        std::cout << "----------------------------\n";
    }

    DogPtr findDogById(int id) const {
        for (const auto& dog : all_dogs) {
            if (dog->id == id) {
                return dog;
            }
        }
        return nullptr;
    }
};

// --- Main Function for Demonstration ---
int main() {
    PedigreeManager manager;

    // Get today's date for age calculations in example output
    auto today = std::chrono::year_month_day(std::chrono::ceil<std::chrono::days>(std::chrono::system_clock::now()));

    // Create dogs using std::chrono::year_month_day for birth_date
    // IMPORTANT: Explicitly convert string literals to std::string
    DogPtr grand_p1 = manager.createDog<Poodle>(std::string("Grandpa Poodle"), Gender::MALE, stringToYmd("2010-01-01"), std::string("Fluffy Cut"));
    DogPtr grand_p2 = manager.createDog<Poodle>(std::string("Grandma Poodle"), Gender::FEMALE, stringToYmd("2010-02-01"), std::string("Lion Cut"));
    
    DogPtr grand_s1 = manager.createDog<Bulldog>(std::string("Grandpa Bully"), Gender::MALE, stringToYmd("2011-03-01"), 15);
    DogPtr grand_s2 = manager.createDog<Bulldog>(std::string("Grandma Bully"), Gender::FEMALE, stringToYmd("2011-04-01"), 12);

    DogPtr father = manager.createDog<Dog>(std::string("Father Mix"), Gender::MALE, stringToYmd("2015-05-01"));
    DogPtr mother = manager.createDog<Dog>(std::string("Mother Mix"), Gender::FEMALE, stringToYmd("2015-06-01"));

    DogPtr child1 = manager.createDog<Dog>(std::string("Child Mix 1"), Gender::MALE, stringToYmd("2019-07-01"));
    DogPtr child2 = manager.createDog<Dog>(std::string("Child Mix 2"), Gender::FEMALE, stringToYmd("2019-08-01"));

    // Set up pedigree relationships
    manager.setParents(father, grand_p1, grand_p2);
    manager.setParents(mother, grand_s1, grand_s2);
    manager.setParents(child1, father, mother);
    manager.setParents(child2, father, mother);

    // Test owner and adoption data
    child1->setAdoptionDate("Alice Smith", stringToYmd("2019-10-15"));

    // Test death date
    grand_p1->setDeathDate(stringToYmd("2024-01-15"));

    // Test breeding eligibility
    std::cout << "\n--- Breeding Eligibility Checks ---\n";
    std::cout << "Can " << father->name << " and " << mother->name << " breed? "
              << (manager.canBreed(father, mother) ? "Yes" : "No") << std::endl; // Should be Yes

    // Test a dog too young
    DogPtr young_dog = manager.createDog<Dog>(std::string("Young Pup"), Gender::MALE, today); // Note: No breed-specific arg for base Dog
    std::cout << "Can " << young_dog->name << " and " << child2->name << " breed? "
              << (manager.canBreed(young_dog, child2) ? "Yes" : "No") << std::endl; // Should be No (too young)

    // Test a dog that died
    std::cout << "Can " << grand_p1->name << " and " << grand_p2->name << " breed? "
              << (manager.canBreed(grand_p1, grand_p2) ? "Yes" : "No") << std::endl; // Should be No (Grandpa Poodle died)

    // Demonstrate birth date error for parent (uncomment to test)
    // DogPtr future_dad = manager.createDog<Dog>(std::string("Future Dad"), Gender::MALE, stringToYmd("2030-01-01"));
    // manager.setParents(father, future_dad, mother); // This will print an error about birth dates

    // Print all dog info
    manager.printAllDogs();

    // Access and call unique methods via dynamic_pointer_cast
    if (auto poodle = std::dynamic_pointer_cast<Poodle>(grand_p1)) {
        poodle->performPoodleTrick();
    }
    std::cout << std::endl;

    return 0;
}
