
using namespace std;
#include <postgresql/libpq-fe.h>
#include <map>
#include <string>

typedef int RC;

class AttributeManager{

    public:
        static AttributeManager &instance();
        RC connect();
        RC addAttr(map<int, string> info);
        RC createTable();
        RC deleteTable();
        RC findAttrType(int attrId);
        
    private:
        PGconn* con = nullptr;
    
};