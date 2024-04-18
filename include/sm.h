#include "pfm.h"
#include "am.h"
#include <vector>
#include <postgresql/libpq-fe.h>
using namespace std;

typedef enum {
    TypeInt = 0, TypeReal, TypeVarChar
} AttrType;

struct Attribute{
    int joinKey; //Attr id
    int maxSize;
    AttrType attrType;

    Attribute(int x, int y, int z) : joinKey(x), maxSize(y), attrType(AttrType(z)){};
};




class SchemaManager{
    public:    
        static SchemaManager &instance();                            

        RC schemaInit(FileHandle fileHandle, int sensorId);
        RC insertFact(void* data, FileHandle fileHandle, Attribute attr);
        RC insertMessage(Attribute attr, void* data, FileHandle fileHandle);
        RC readAllFacts(FileHandle fileHandle);
        RC readAllData(int pageNum, Attribute attr, FileHandle fileHandle);
    


};