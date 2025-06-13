#include <fstream>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <iostream>
//#include <sys/stat.h>

int load_env(const std::string& filename = ".env") {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open " << filename << "\n";
        return -1;
    }
/*    
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) == -1) {
        perror("stat");
        return;
*/
    std::string line;
    while (getline(file, line)) {
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos || line[0] == '#') continue;
        std::string key = line.substr(0, eq_pos);
        std::string val = line.substr(eq_pos + 1);
        setenv(key.c_str(), val.c_str(), 1);  // Linux/macOS용. Windows는 _putenv_s 사용.

        std::cout << key << " : " << val << std::endl;
    }
    return 0;
}

std::string trim_env(const char* name) {
    const char* val = std::getenv(name);
    if (!val) throw std::runtime_error(std::string("환경변수 '") + name + "'이(가) 없습니다.");
    std::string s(val);
    s.erase(s.find_last_not_of(" \r\n\t") + 1);  // 오른쪽 개행·공백 제거

    // std::cout << s << std::endl;
    return s;
}

#include <libpq-fe.h>

int main() {
    if( load_env() == -1 ) return 1;  // .env 불러오기

    std::string conninfo = std::string(
        "host=" + trim_env("DB_HOST") +
        " port=" + trim_env("DB_PORT") +
        " user=" + trim_env("DB_USER") +
        " password=" + trim_env("DB_PASSWORD") +
        " dbname=" + trim_env("DB_NAME")
/*        
        "host=" + std::string(getenv("DB_HOST")) +
        " port=" + std::string(getenv("DB_PORT")) +
        " user=" + std::string(getenv("DB_USER")) +
        " password=" + std::string(getenv("DB_PASSWORD")) +
        " dbname=" + std::string(getenv("DB_NAME"))
*/
    );

    std::cout << "[" << conninfo << "]" << std::endl;
    PGconn* conn = PQconnectdb(conninfo.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << "\n";
        PQfinish(conn);
        return 1;
    }

    std::cout << "Connected to database successfully!\n";

    // ... 쿼리 수행 등 ...

    PQfinish(conn);
    return 0;
}

// g++ -std=c++17 -o db_conn dbconn.cpp -lpq -I/usr/include/postgresql

