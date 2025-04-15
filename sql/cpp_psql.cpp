#include <iostream>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

int main() {
    try {
        // DB 연결 설정
        connection conn("host=localhost dbname=gau user=gau password=qjemf");
        if (!conn.is_open()) {
            cerr << "DB 연결 실패!" << endl;
            return 1;
        }

        work txn(conn);

        // 1. 테이블 생성 (IF NOT EXISTS)
        txn.exec(
            "CREATE TABLE IF NOT EXISTS test_table ("
            "id SERIAL PRIMARY KEY,"
            "name VARCHAR(100),"
            "age INT,"
            "score FLOAT"
            ");"
        );

        // 2. 기존 데이터 조회
        result res = txn.exec("SELECT * FROM test_table");
        cout << "=== 기존 데이터 ===" << endl;
        for (auto row : res) {
            cout << row["id"].as<int>() << ", "
                 << row["name"].as<string>() << ", "
                 << row["age"].as<int>() << ", "
                 << row["score"].as<float>() << endl;
        }

        // 3. 데이터 수정 (모든 age +1, score * 1.1)
        txn.exec("UPDATE test_table SET age = age + 1, score = score * 1.1");

        // 4. 새 데이터 추가
        txn.exec_params(
            "INSERT INTO test_table (name, age, score) VALUES ($1, $2, $3)",
            "Charlie", 28, 95.5
        );

        // 5. 수정된 데이터 확인
        res = txn.exec("SELECT * FROM test_table ORDER BY id DESC LIMIT 5");
        cout << "\n=== 수정/추가 후 데이터 ===" << endl;
        for (auto row : res) {
            cout << row["id"].as<int>() << ", "
                 << row["name"].as<string>() << ", "
                 << row["age"].as<int>() << ", "
                 << row["score"].as<float>() << endl;
        }

        txn.commit();
        conn.close();

    } catch (const exception &e) {
        cerr << "오류 발생: " << e.what() << endl;
        return 1;
    }
    return 0;
}

// g++ -std=c++17 cpp_psql.cpp -lpqxx -o cpp_psqldemo
