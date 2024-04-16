#include "./include/sm.h"
#include <string.h>
#include <cmath>
#include <iostream>
#include <fstream>


SchemaManager &SchemaManager::instance(){
    static SchemaManager sm = SchemaManager();
    return sm;
}

RC SchemaManager::schemaInit(FileHandle fileHandle){ //pretty self explanatory 
    char* factPage = (char*) malloc(PAGE_SIZE);
    int numPages = 1;
    int numSlots = 0;

    memcpy(factPage+PAGE_SIZE-8, &numPages, 4);
    memcpy(factPage+PAGE_SIZE-4, &numSlots, 4);
    fileHandle.appendPage(factPage);
    free(factPage);


}

/*int updateGlossary(int glossNum, FileHandle fileHandle, int newKey, int newNum, int newTotal){ //not using anymore
    char* glossary = (char*) malloc(PAGE_SIZE);
    int glossaryPage = floor(glossNum/1024);
    fileHandle.readPage(glossaryPage, glossary);

    int adjustedNum = glossNum - (glossaryPage*1024);
    int offset = adjustedNum * 12;

    memcpy(glossary+offset, &newKey, 4);
    offset += 4;
    memcpy(glossary+offset, &newNum, 4);
    offset += 4;
    memcpy(glossary+offset, &newTotal, 4);

    fileHandle.writePage(glossaryPage, glossary);
    return 0;
}*/

int factInit(void* data, FileHandle fileHandle, int dataSize){
    char* newPage = (char*) malloc(PAGE_SIZE);

    int numSlots = 1;

    memcpy(newPage+PAGE_SIZE-4, &numSlots, 4);
    memcpy(newPage, data, dataSize);

    fileHandle.appendPage(newPage);
    free(newPage);

    char* oldPage = (char*) malloc(PAGE_SIZE);
    fileHandle.readPage(0, oldPage);

    int curPage;
    memcpy(&curPage, oldPage+PAGE_SIZE-8, 4);
    curPage++;
    memcpy(oldPage+PAGE_SIZE-8, &curPage, 4);
    fileHandle.writePage(0, oldPage);
    free(oldPage);

    

}

RC SchemaManager::insertFact(void* data, FileHandle fileHandle){
    void* alloFact = malloc(PAGE_SIZE);
    char* factPage = (char*) alloFact;

    fileHandle.readPage(0, factPage);
    
    int numPages;
    memcpy(&numPages, factPage+PAGE_SIZE-8, 4);
    fileHandle.readPage((numPages-1), factPage); //get index of last page

    int numSlots;
    memcpy(&numSlots, factPage+PAGE_SIZE-4, 4);
    int usedSpace = numSlots * 4; //because facts can only be integers in this case then the offset to the last slot will always be the number of slots * 4
    if(usedSpace+12 > PAGE_SIZE){ //if theres less than 12 bytes left on page add new page
        factInit(data, fileHandle, 4);
        free(factPage);
        return 0;
    }
    else{
        int insertOffset = numSlots*4; 
        memcpy(factPage+insertOffset, (char*) data, 4); //insert new fact. Size will always be 4 because only integers can be facts
        numSlots++; //incriment num slots
        memcpy(factPage+PAGE_SIZE-4, &numSlots, 4);
        fileHandle.writePage((numPages-1), factPage); 
        free(factPage);
        return 0;
    }

    
}

int getPage(int joinKey, FileHandle fileHandle){
    char* factPage = (char*) malloc(PAGE_SIZE);


    int factPageNum = floor(joinKey/1022); //1022 maximum records in fact page. record num/max records rounded down gices us the page number the fact is on
    int slotNum = joinKey - (factPageNum*1022)-1; //removing the number of pages * the max number of records from the record num to adjust it for cur page slot

    fileHandle.readPage(factPageNum, factPage);
    int offset = slotNum * 4;

    int pageNum;
    memcpy(&pageNum, factPage+offset, 4);
    return pageNum;   
}

int updateFactPage(int joinKey, int pageNum, FileHandle fileHandle){ //self explanatory
    char* factPage = (char*) malloc(PAGE_SIZE);

    int factPageNum = floor(joinKey/1022);
    int slotNum = joinKey - (factPageNum*1022)-1;

    fileHandle.readPage(factPageNum, factPage);
    int offset = slotNum * 4;

    memcpy(factPage+offset, &pageNum, 4);
    fileHandle.writePage(factPageNum, factPage);

    return 0;
}

int getNumAttrs(FileHandle fileHandle){ 
    char* factPage = (char*) malloc(PAGE_SIZE);

    fileHandle.readPage(0, factPage);

    int numPages;
    memcpy(&numPages, factPage+PAGE_SIZE-8, 4);
    int numAttrs = 0;

    for(int i = 0; i < numPages; i++){
        fileHandle.readPage(i, factPage); //Goes through every fact page and adds the number of slots on a page to the variabe num attrs

        int numSlots;
        memcpy(&numSlots, factPage+PAGE_SIZE-4, 4);
        numAttrs += numSlots;
    }
    free(factPage);
    return numAttrs;
}

int insertAttrData(Attribute attr, void* data, int pageNum, FileHandle fileHandle){
    int totalPages = fileHandle.getNumberOfPages();
    int numPages = ceil((attr.maxSize+4)/4096);
    int offset = 4;
    int nextPageNum = totalPages;
    for(int i = 0; i < numPages; i++){ //If multiple pages then it puts PAGE_SIZE-4 of the data onto the first page and then puts the next page number for the data on the last 4 bytes.
        char* newAttr = (char*) malloc(PAGE_SIZE);
        if(i == numPages-1){
            memcpy(newAttr, data, PAGE_SIZE-4);
            int eoa = -1;
            memcpy(newAttr+PAGE_SIZE-4, &eoa, 4);
        }
        else{
            memcpy(newAttr, data+(i*(PAGE_SIZE-4)), PAGE_SIZE-4);
            memcpy(newAttr+PAGE_SIZE-4, &nextPageNum, 4);
            nextPageNum++;
        }

        if(i == 0 && pageNum == -1){ //if the fact table says that the attr is numm then change it to the page number and only do this the first time.
            updateFactPage(attr.joinKey, fileHandle.getNumberOfPages(), fileHandle);
        }
        
        fileHandle.appendPage(newAttr);
        free(newAttr);
    }
    
}

RC SchemaManager::insertMessage(Attribute attr, void* data, FileHandle fileHandle){
   
    if(attr.joinKey == -1){ //if Inserting a fact 
        insertFact(data, fileHandle);
    }
    else{
        int totalPages = fileHandle.getNumberOfPages();
        if(attr.joinKey > getNumAttrs(fileHandle)){ //if its not joining as a fact but is the attr is still more than the num of attrs (IE. Inserting Attr 11 when fact table has 10 attrs then just add an attr and proceed as normal)
            cout << "Num Attrs: " << getNumAttrs(fileHandle) << endl;
            char* attrFact = (char*) malloc(4);
            memcpy(attrFact, &totalPages, 4);
            insertFact(attrFact, fileHandle);
            free(attrFact);

            insertAttrData(attr, data, totalPages, fileHandle);
        }
        else{
            int attrPageNum = getPage(attr.joinKey, fileHandle);
            insertAttrData(attr, data, attrPageNum, fileHandle);
        }
    }

}

RC SchemaManager::readAllData(int pageNum, Attribute attr, FileHandle fileHandle){ //Change data size variable? Store it on first page? Or incorporate it a different way? Or have it so attr details in a DB and its j read from there
    char* dataPage = (char*) malloc(PAGE_SIZE);
    int dataSize = attr.maxSize;
    if(dataSize > PAGE_SIZE){
        dataSize = PAGE_SIZE;
    }
    ofstream dataVals("Data.txt", fstream::app);
    int nextPage = pageNum;
    while(nextPage != -1){ //self explanatory
        fileHandle.readPage(nextPage, dataPage);

        if(attr.attrType == TypeInt){
            int attrValue;
            memcpy(&attrValue, dataPage, 4);
            dataVals << "Fact: " << attr.joinKey << endl;
            dataVals << "   Attribute Value: " << attrValue << endl;  
        }
        else if(attr.attrType == TypeReal){
            float attrValue; 
            memcpy(&attrValue, dataPage, 4);
            dataVals << "Fact: " << attr.joinKey << endl;
            dataVals << "   Attribute Value: " << attrValue << endl;  
        }
        else{
            int length;
            memcpy(&length, dataPage, 4);
            string attrValue;
            memcpy(&attrValue, dataPage+4, length);
            dataVals << "Fact: " << attr.joinKey << endl;
            dataVals << "   Attribute Value: " << attrValue << endl;  
        }

        memcpy(&nextPage, dataPage+PAGE_SIZE-4, 4);
    }
    free(dataPage);
    return 0;
}

RC SchemaManager::readAllFacts(FileHandle fileHandle){ //self explanatory.
    char* factTable = (char*) malloc(PAGE_SIZE);

    fileHandle.readPage(0, factTable);

    ofstream textFile("Output.txt");

    int numPages;
    memcpy(&numPages, factTable+PAGE_SIZE-8, 4);
    
    
    cout << "This is the number of pages: " << numPages << endl;


    for(int i = 0; i < numPages; i++){
        textFile << "The current fact page is: " << i << endl;
        fileHandle.readPage(i, factTable);
        int numFacts;
        memcpy(&numFacts, factTable+PAGE_SIZE-4, 4);

        int offset = 0;
        cout << "The number of facts is: " << numFacts << endl;
        for(int i = 0; i<numFacts; i++){
            int curFact;
            memcpy(&curFact, factTable+offset, 4);

            textFile << "This is the current fact: " << curFact << endl;
            Attribute attr((offset/4), 4, 0);
            readAllData(curFact, attr, fileHandle);
            offset += 4;
        }
    }

    free(factTable);
    return 0;
}