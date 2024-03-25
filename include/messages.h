#include <postgresql/libpq-fe.h>
#include "./tm.h"
#include <string>
#include <vector>

using namespace std;


class MessageManager{
    public:
        int insertIntoTable(string tableName, vector<Attribute> &attrDesc, const void *inBuffer);
        int insertIntoAttr();
        int insertIntoSensor();
};