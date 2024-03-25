#include <postgresql/libpq-fe.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>


using namespace std;

struct Attribute{
    string name;
    int id;
    int type; // 0 = int, 1 = float, 2 = varchar
};

struct Sensor{
    int id;
    vector<int> attrIds;
};

class TableManager{
    public:
        static TableManager &instance();
        int dbConnect(string dbName);
        int createCatalogs(vector<Attribute> attrs, vector<Sensor> sensors);
        
        PGconn *con = nullptr;
        string tableName;

    protected:
        TableManager();
        ~TableManager();
};