#include <string>
#include <vector> // 자식 목록을 위해 std::vector 사용
#include <list>   // 전체 개체 목록을 위해 std::list 사용
#include <memory> // 스마트 포인터를 위해

// 전방 선언: Dog 클래스가 Dog의 포인터를 가질 수 있도록
class Dog;

// 스마트 포인터 타입 정의 (편의성)
using DogPtr = std::shared_ptr<Dog>;
using DogWeakPtr = std::weak_ptr<Dog>; // 부모는 약한 참조를 사용하는 것이 순환 참조를 방지하는 좋은 방법입니다.

class Dog {
public:
    std::string name;
    int id; // 고유 식별자
    // 기타 정보 (품종, 생일 등)

    // 부모견 (두 마리이므로 vector 또는 array 사용)
    // weak_ptr 사용: 부모가 자식을 소유하지 않으므로 순환 참조 방지
    std::vector<DogWeakPtr> parents;

    // 자식견들 (std::list 또는 std::vector 사용 가능)
    // 부모가 자식을 소유하는 개념이라면 shared_ptr
    // shared_ptr 대신 unique_ptr을 사용하면 단일 소유권을 명확히 합니다.
    // 여기서는 여러 부모가 있을 수 있으므로 shared_ptr이 더 자연스러울 수 있습니다.
    std::list<DogPtr> children; // <-- 여기에 std::list 활용

    // 생성자
    Dog(const std::string& name, int id) : name(name), id(id) {
        parents.reserve(2); // 부모는 최대 2마리
    }

    // 부모 추가 함수
    void addParent(DogPtr parent) {
        if (parents.size() < 2) {
            parents.push_back(parent);
        } else {
            // 에러 처리 또는 예외 발생
            // std::cerr << "Error: Cannot add more than two parents.\n";
        }
    }

    // 자식 추가 함수
    void addChild(DogPtr child) {
        children.push_back(child);
    }

    // 편의를 위한 출력 함수
    void printDogInfo() const {
        std::cout << "ID: " << id << ", Name: " << name;
        std::cout << ", Parents: [";
        bool first = true;
        for (const auto& parent_weak : parents) {
            if (auto parent_shared = parent_weak.lock()) { // 약한 참조를 강한 참조로 변환
                if (!first) std::cout << ", ";
                std::cout << parent_shared->name;
                first = false;
            }
        }
        std::cout << "]";
        std::cout << ", Children Count: " << children.size() << std::endl;
    }
};
