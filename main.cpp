#include "./include/sm.h"
#include <string.h>
#include <fstream>


int main(int argc, char const *argv[])
{
    PagedFileManager &pfm = PagedFileManager::instance();
    FileHandle fileHandle;

    pfm.createFile("messageSchema.msg");
    pfm.openFile("messageSchema.msg", fileHandle);

    pfm.destroyFile("Data.txt");
    pfm.destroyFile("Output.txt");

    Attribute attr(-1, 4, 0);
    SchemaManager &sm = SchemaManager::instance();
    sm.schemaInit(fileHandle);

    char* factData = (char*) malloc(4);
    
    for(int i = 1; i <= 10; i++){
        int fact = -1;
        memcpy(factData, &fact, 4);
        sm.insertMessage(attr, factData, fileHandle);
    }
    free(factData);

    char* attrData = (char*) malloc(4);

    for(int j = 1; j <= 11; j++){
        int attr = j*j;
        memcpy(attrData, &attr, 4);

        Attribute newAttr(j, 4, 0);
        sm.insertMessage(newAttr, attrData, fileHandle);
    }
    free(attrData);

    sm.readAllFacts(fileHandle);

    pfm.closeFile(fileHandle);
    pfm.destroyFile("messageSchema.msg");

    return 0;
}
