#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <memory>    // std::shared_ptr, std::weak_ptr, std::enable_shared_from_this
#include <chrono>    // For potential date management (not fully used here)
#include <iomanip>   // For formatting (not fully used here)
#include <type_traits> // For static_assert in createDog

// Forward declaration
class Dog;

// Smart pointer type definitions
using DogPtr = std::shared_ptr<Dog>;
using DogWeakPtr = std::weak_ptr<Dog>;

// Dog Gender enum
enum class Gender {
    MALE,
    FEMALE,
    UNKNOWN
};

// Helper to convert Gender to string
std::string genderToString(Gender g) {
    switch (g) {
        case Gender::MALE: return "Male";
        case Gender::FEMALE: return "Female";
        case Gender::UNKNOWN: return "Unknown";
        default: return "Invalid";
    }
}

class Dog : public std::enable_shared_from_this<Dog> {
public:
    std::string name;
    int id; // ID is still a member
    Gender gender;
    std::string birth_date;
    std::string death_date;

    struct Parents {
        DogWeakPtr father;
        DogWeakPtr mother;
    } parents; // Dog 클래스 멤버로 parents 구조체 인스턴스 선언
    
    std::list<DogPtr> children;

    // Modified Constructor: id is now the last parameter
    Dog(const std::string& name, Gender gender, const std::string& birth_date, int id_val)
        : name(name), gender(gender), birth_date(birth_date), id(id_val) { // Initialize id here
    }
    virtual ~Dog() = default;

    // Add a parent to this dog. This dog becomes a child of 'parent'.
    void addParent(DogPtr parent) {
        if (!parent) {
            std::cerr << "Error: Attempted to add a null parent to " << name << ".\n";
            return;
        }
        if (parent->gender == Gender::MALE) {
            setFather(parent);
        } else if (parent->gender == Gender::FEMALE) {
            setMother(parent);
        } else {
            std::cerr << "Error: Cannot assign parent " << parent->name << " with unknown gender to " << name << ".\n";
        }
    
    }

    // Add a child to this dog. This dog becomes a parent of 'child'.
    void addChild(DogPtr child) {
        if (!child) {
            std::cerr << "Error: Attempted to add a null child to " << name << ".\n";
            return;
        }
        children.push_back(child);
    }

    // Check if the dog is alive
    bool isAlive() const {
        return death_date.empty();
    }

    // Set the death date
    void setDeathDate(const std::string& date) {
        death_date = date;
    }

    bool canBreed(DogPtr male_dog, DogPtr female_dog) {
        return male_dog->gender == Gender::MALE && female_dog->gender == Gender::FEMALE 
          && male_dog->isAlive() && female_dog->isAlive();
    }
    // 아버지 설정 (새로운 함수)
    void setFather(DogPtr father_dog) {
        if (!father_dog || father_dog->gender != Gender::MALE) {
            std::cerr << "Error: Invalid father dog for " << name << ".\n";
            return;
        }
        parents.father = father_dog;
        // 부모 측에서도 자식을 추가 (양방향 연결)
        if (auto self_shared = shared_from_this()) {
            father_dog->addChild(self_shared);
        }
    }

    // 어머니 설정 (새로운 함수)
    void setMother(DogPtr mother_dog) {
        if (!mother_dog || mother_dog->gender != Gender::FEMALE) {
            std::cerr << "Error: Invalid mother dog for " << name << ".\n";
            return;
        }
        parents.mother = mother_dog;
        // 부모 측에서도 자식을 추가 (양방향 연결)
        if (auto self_shared = shared_from_this()) {
            mother_dog->addChild(self_shared);
        }
    }

    
    // Virtual function to print common info, can be overridden by derived classes
    virtual void printInfo() const {
        std::cout << "ID: " << id
                  << ", Name: " << name
                  << ", Gender: " << genderToString(gender)
                  << ", Birth: " << birth_date;
        if (!death_date.empty()) {
            std::cout << ", Death: " << death_date;
        }
        std::cout << ", Parents: [";
        bool first_parent = true;
        if (auto father_shared = parents.father.lock()) { // 아버지 존재 확인
            std::cout << "F: " << father_shared->name;
            first_parent = false;
        } else if (!parents.father.expired()) { // weak_ptr가 유효하지만 lock 실패 (nullptr)
            std::cout << "F: [Unknown/Deceased]";
            first_parent = false;
        }

        if (auto mother_shared = parents.mother.lock()) { // 어머니 존재 확인
            if (!first_parent) std::cout << ", ";
            std::cout << "M: " << mother_shared->name;
        } else if (!parents.mother.expired()) { // weak_ptr가 유효하지만 lock 실패 (nullptr)
            if (!first_parent) std::cout << ", ";
            std::cout << "M: [Unknown/Deceased]";
        }
        std::cout << "]";
        std::cout << ", Children Count: " << children.size();
    }
};

// --- Derived Dog Classes ---

class Poodle : public Dog {
public:
    std::string fur_style;

    // Modified Constructor: id_val passed as last arg to Dog constructor
    Poodle(const std::string& name, Gender gender, const std::string& birth_date, const std::string& fur_style_val, int id_val)
        : Dog(name, gender, birth_date, id_val), fur_style(fur_style_val) {} // Pass id_val to base
    // ... rest of Poodle class ...
    void printInfo() const override {
        Dog::printInfo(); // Call base class printInfo
        std::cout << "  - Breed: Poodle, Fur Style: " << fur_style << std::endl;
    }
    
    void performPoodleTrick() const {
        std::cout << name << " is performing a poodle trick!\n";
    }

};

class Bulldog : public Dog {
public:
    int wrinkle_count;

    // Modified Constructor: id_val passed as last arg to Dog constructor
    Bulldog(const std::string& name, Gender gender, const std::string& birth_date, int wrinkles, int id_val)
        : Dog(name, gender, birth_date, id_val), wrinkle_count(wrinkles) {} // Pass id_val to base
    // ... rest of Bulldog class ...
    void printInfo() const override {
        Dog::printInfo(); // Call base class printInfo
        std::cout << "  - Breed: Bulldog, Wrinkles: " << wrinkle_count << std::endl;
    }

};

// --- PedigreeManager Class ---
class PedigreeManager {
public:
    std::list<DogPtr> all_dogs; // List of all dogs, owning the shared_ptr
    int next_id = 1; // Auto-incrementing ID

    // Templated function to create any Dog-derived class
    template<typename T, typename... Args>
    DogPtr createDog(Args&&... args) {
        // Ensure T is a Dog or a class derived from Dog
        static_assert(std::is_base_of<Dog, T>::value, "T must derive from Dog");
        
        // Pass id as the first argument to the constructor, then forward others
        // Note: The Dog constructor now expects id as the second parameter.
        // We need to adjust args based on Dog constructor signature.
        // A common pattern is to make id an internal detail or last param.
        // For simplicity, let's assume T's constructor takes (name, gender, birth_date, ..., id)
        // Or make id a default argument in Dog constructor if possible.
        // For this example, let's adjust createDog to pass args first, then id.
        
        DogPtr new_dog = std::make_shared<T>(std::forward<Args>(args)..., next_id++); 
        all_dogs.push_back(new_dog);
        return new_dog;
    }

    // Set up bidirectional parent-child relationship
    
    void setParents(DogPtr child, DogPtr father_dog, DogPtr mother_dog) {
        if (!child || !father_dog || !mother_dog) {
            std::cerr << "Error: Null dog pointer(s) provided for parent setting.\n";
            return;
        }
        if (father_dog->gender != Gender::MALE || mother_dog->gender != Gender::FEMALE) {
            std::cerr << "Error: Invalid genders for parents. Father must be Male, Mother must be Female.\n";
            return;
        }
        child->setFather(father_dog);
        child->setMother(mother_dog);
    }

    // Print info for all dogs
    void printAllDogs() const {
        std::cout << "\n--- All Dogs in Pedigree ---\n";
        for (const auto& dog : all_dogs) {
            dog->printInfo(); // Polymorphically calls the correct printInfo
        }
        std::cout << "----------------------------\n";
    }

    // Find a dog by ID
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

    // Constructor args are: name, gender, birth_date, [breed specific args], id (added by createDog)
    // The arguments you supply to createDog should *not* include the ID.
    // createDog will automatically add the ID generated by next_id++.
    DogPtr grand_p1 = manager.createDog<Poodle>("Grandpa Poodle", Gender::MALE, "2010-01-01", "Fluffy Cut");
    DogPtr grand_p2 = manager.createDog<Poodle>("Grandma Poodle", Gender::FEMALE, "2010-02-01", "Lion Cut");

    DogPtr grand_s1 = manager.createDog<Bulldog>("Grandpa Bully", Gender::MALE, "2011-03-01", 15);
    DogPtr grand_s2 = manager.createDog<Bulldog>("Grandma Bully", Gender::FEMALE, "2011-04-01", 12);

    DogPtr father = manager.createDog<Dog>("Father Mix", Gender::MALE, "2015-05-01");
    DogPtr mother = manager.createDog<Dog>("Mother Mix", Gender::FEMALE, "2015-06-01");

    DogPtr child1 = manager.createDog<Dog>("Child Mix 1", Gender::MALE, "2019-07-01");
    DogPtr child2 = manager.createDog<Dog>("Child Mix 2", Gender::FEMALE, "2019-08-01");

    // Set up pedigree relationships
    manager.setParents(father, grand_p1, grand_p2);
    manager.setParents(mother, grand_s1, grand_s2);
    manager.setParents(child1, father, mother);
    manager.setParents(child2, father, mother);

    // Print all dog info
    manager.printAllDogs();

    // Access and call unique methods via dynamic_pointer_cast
    if (auto poodle = std::dynamic_pointer_cast<Poodle>(grand_p1)) {
        poodle->performPoodleTrick();
    }
    std::cout << std::endl;

    // Simulate death
    grand_p1->setDeathDate("2024-01-15");
    std::cout << "\n--- After setting Grandpa Poodle's death date ---\n";
    grand_p1->printInfo();
    std::cout << std::endl;

    // Demonstrate weak_ptr behavior after a shared_ptr goes out of scope
    // This is more complex to show directly in main as manager.all_dogs
    // holds the shared_ptrs. If you remove a dog from all_dogs, its parents'
    // weak_ptrs would show [Unknown/Deceased].
    // For example, if manager.all_dogs.remove(grand_p2); was called,
    // and then father's info was printed, grand_p2 would show as deceased in father's parents list.

    return 0;
}
