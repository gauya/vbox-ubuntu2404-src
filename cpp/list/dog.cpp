#include <iostream>
#include "dog.h"

// ... (위의 Dog 클래스 정의 포함) ...

class PedigreeManager {
public:
    // 모든 개들을 저장하는 리스트 (스마트 포인터로 소유권 관리)
    std::list<DogPtr> all_dogs;
    int next_id = 1; // ID 자동 할당을 위한 카운터

    DogPtr createDog(const std::string& name) {
        DogPtr new_dog = std::make_shared<Dog>(name, next_id++);
        all_dogs.push_back(new_dog);
        return new_dog;
    }

    // 혈통 관계 설정
    void setParents(DogPtr child, DogPtr parent1, DogPtr parent2) {
        child->addParent(parent1);
        child->addParent(parent2);
        parent1->addChild(child);
        parent2->addChild(child);
    }

    // 모든 개 정보 출력
    void printAllDogs() const {
        std::cout << "--- All Dogs in Pedigree ---\n";
        for (const auto& dog : all_dogs) {
            dog->printDogInfo();
        }
        std::cout << "----------------------------\n";
    }

    // 특정 개의 자식들을 순회하며 출력하는 함수 예시
    void printChildrenOf(DogPtr dog) const {
        std::cout << dog->name << "'s Children: ";
        if (dog->children.empty()) {
            std::cout << "None.\n";
            return;
        }
        bool first = true;
        for (const auto& child : dog->children) {
            if (!first) std::cout << ", ";
            std::cout << child->name;
            first = false;
        }
        std::cout << std::endl;
    }

    // 혈통 순회 (BFS/DFS) 예시 (추가적인 구현 필요)
    // 특정 개로부터 자식 또는 부모 방향으로 탐색할 수 있습니다.
};

int main() {
    PedigreeManager manager;

    // 개체 생성
    DogPtr grand_p1 = manager.createDog("Grandpa Poodle");
    DogPtr grand_p2 = manager.createDog("Grandma Poodle");
    DogPtr grand_s1 = manager.createDog("Grandpa Shih Tzu");
    DogPtr grand_s2 = manager.createDog("Grandma Shih Tzu");

    DogPtr father = manager.createDog("Father Dog");
    DogPtr mother = manager.createDog("Mother Dog");

    DogPtr child1 = manager.createDog("Child Dog 1");
    DogPtr child2 = manager.createDog("Child Dog 2");

    // 혈통 관계 설정
    manager.setParents(father, grand_p1, grand_p2);
    manager.setParents(mother, grand_s1, grand_s2);
    manager.setParents(child1, father, mother);
    manager.setParents(child2, father, mother); // child2는 child1의 형제

    // 모든 개 정보 출력
    manager.printAllDogs();

    // 특정 개 자식 출력
    manager.printChildrenOf(father);
    manager.printChildrenOf(mother);
    manager.printChildrenOf(child1); // 자식이 없으므로 "None."
    
    return 0;
}

