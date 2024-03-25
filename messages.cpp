#include "./include/messages.h"

    
vector<Attribute> getAllAttrs(){
    //Get all attrs and create attr descriptor;
}

int MessageManager::insertIntoTable(string tableName, vector<Attribute> &attrDesc, const void* inBuffer){
    char* curPage = (char*) inBuffer;
    
    vector<Attribute> allAttrs = getAllAttrs();

    int numAttrs = allAttrs.size();

    int x = 0;

    for(int i = 0; i < numAttrs; i++){
        Attribute curAttr = allAttrs[i];
        Attribute insertAttr = attrDesc[x];

        
    }
    

}


