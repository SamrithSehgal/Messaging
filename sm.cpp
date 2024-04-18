#include "./include/sm.h"
#include <string.h>
#include <cmath>
#include <iostream>
#include <fstream>


SchemaManager &SchemaManager::instance(){
    static SchemaManager sm = SchemaManager();
    return sm;
}

RC SchemaManager::schemaInit(FileHandle fileHandle, int sensorId){ //pretty self explanatory 
    char* factPage = (char*) malloc(PAGE_SIZE);
    int numPages = 1;
    int numSlots = 1;
    int eop = -1;

    memcpy(factPage, &sensorId, 4);
    memcpy(factPage+PAGE_SIZE-12, &eop, 4);
    memcpy(factPage+PAGE_SIZE-8, &numPages, 4);
    memcpy(factPage+PAGE_SIZE-4, &numSlots, 4);
    fileHandle.appendPage(factPage);
    free(factPage);


    


}

int updateFactSpot(FileHandle fileHandle){
    int eop = 0;
    char* finalPage = (char*) malloc(PAGE_SIZE);
    int curPageNum = 0;
    int numPages;
    fileHandle.readPage(0, finalPage);
    memcpy(&numPages, finalPage+PAGE_SIZE-8, 4);
    
    for(int i = 0; i < (numPages-1); i++){
        fileHandle.readPage(eop, finalPage);
        memcpy(&eop, finalPage+PAGE_SIZE-12, 4);
        if(eop != -1){
            curPageNum = eop;
        }
    }
    int totalPages = fileHandle.getNumberOfPages() - 1;
    memcpy(finalPage+PAGE_SIZE-12, &totalPages, 4);
    //cout << "EOP(update): " << eop << endl;
    cout << "Cur Page: " << curPageNum << endl;
    fileHandle.writePage(curPageNum, finalPage);
    free(finalPage);
    return eop;
}

int findFactSpot(FileHandle fileHandle){
    int eop = 0;
    char* finalPage = (char*) malloc(PAGE_SIZE);
    int numPages;
    fileHandle.readPage(0, finalPage);
    memcpy(&numPages, finalPage+PAGE_SIZE-8, 4);
    
    for(int i = 0; i < (numPages-1); i++){
        fileHandle.readPage(eop, finalPage);
        memcpy(&eop, finalPage+PAGE_SIZE-12, 4);
    }
    //cout << "EOP(find): " << eop << endl;
    free(finalPage);
    return eop;  
}

int factInit(void* data, FileHandle fileHandle, int dataSize){

    char* newPage = (char*) malloc(PAGE_SIZE);

    int numSlots = 0;
    int eop = -1;

    memcpy(newPage+PAGE_SIZE-4, &numSlots, 4);
    memcpy(newPage+PAGE_SIZE-12, &eop, 4);
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

    updateFactSpot(fileHandle);

}

RC SchemaManager::insertFact(void* data, FileHandle fileHandle, Attribute attr){
    void* alloFact = malloc(PAGE_SIZE);
    char* factPage = (char*) alloFact;

    fileHandle.readPage(0, factPage);
    
    int numPages;
    memcpy(&numPages, factPage+PAGE_SIZE-8, 4);
    int pageNumber = findFactSpot(fileHandle);
    fileHandle.readPage(pageNumber, factPage); //get index of last page

    int numSlots;
    memcpy(&numSlots, factPage+PAGE_SIZE-4, 4);
    int usedSpace = numSlots * 4; //because facts can only be integers in this case then the offset to the last slot will always be the number of slots * 4

    int inbetween = attr.joinKey - numSlots;
    int ibOffset = usedSpace;
    int nullFact = -1;
    while(inbetween > 1){
        if(numSlots == 1021){
            
            numPages++;
            memcpy(factPage+PAGE_SIZE-4, &numSlots, 4);
            fileHandle.writePage(pageNumber, factPage);
            factInit(data, fileHandle, 4);
            fileHandle.readPage(fileHandle.getNumberOfPages(), factPage);
            numSlots = 0;
            ibOffset = 0;

            int curPageNum;
            memcpy(&curPageNum, data, 4);
            curPageNum++;
            memcpy(data, &curPageNum, 4);
        }
        else{
            memcpy(factPage+ibOffset, &nullFact, 4);
            ibOffset += 4;
            numSlots++;
            inbetween--;
        }
    }

    pageNumber = findFactSpot(fileHandle);

    if(usedSpace+16 > PAGE_SIZE){ //if theres less than 12 bytes left on page add new page
        factInit(data, fileHandle, 4);
        fileHandle.readPage(pageNumber, factPage);
        memcpy(factPage, data, 4);
        fileHandle.writePage(pageNumber, factPage);
        free(factPage);
        return 0;
    }
    else{
        int insertOffset = ibOffset; 
        memcpy(factPage+insertOffset, (char*) data, 4); //insert new fact. Size will always be 4 because only integers can be facts
        numSlots++; //incriment num slots
        memcpy(factPage+PAGE_SIZE-4, &numSlots, 4);
        fileHandle.writePage(pageNumber, factPage); 
        free(factPage);
        return 0;
    }

    
}

int getPage(int joinKey, FileHandle fileHandle){
    char* factPage = (char*) malloc(PAGE_SIZE);


    int factPageNum = floor(joinKey/1021); //1021 maximum records in fact page. record num/max records rounded down gices us the page number the fact is on
    int slotNum = joinKey - (factPageNum*1021)-1; //removing the number of pages * the max number of records from the record num to adjust it for cur page slot

    fileHandle.readPage(factPageNum, factPage);
    int offset = slotNum * 4;

    int pageNum;
    memcpy(&pageNum, factPage+offset, 4);
    return pageNum;   
}

int updateFactPage(int joinKey, int pageNum, FileHandle fileHandle){ //self explanatory
    char* factPage = (char*) malloc(PAGE_SIZE);

    int factPageNum = floor(joinKey/1021);
    int slotNum = joinKey - (factPageNum*1021)-1;

    int eop = 0;
    for(int i = 0; i < factPageNum; i++){
        fileHandle.readPage(eop, factPage);
        memcpy(&eop, factPage+PAGE_SIZE-12, 4);
    }

    fileHandle.readPage(eop, factPage);
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
    int eop = 0;

    for(int i = 0; i < numPages; i++){
        fileHandle.readPage(eop, factPage); //Goes through every fact page and adds the number of slots on a page to the variabe num attrs
        memcpy(&eop, factPage+PAGE_SIZE-12, 4);
        int numSlots;
        memcpy(&numSlots, factPage+PAGE_SIZE-4, 4);
        numAttrs += numSlots;
    }
    free(factPage);
    return numAttrs;
}

int insertAttrData(Attribute attr, void* data, int pageNum, FileHandle fileHandle){
    int totalPages = fileHandle.getNumberOfPages();
    int numPages = ceil((attr.maxSize+4)/4096) + 1;
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

        if(i == 0 && pageNum == -1){ //if the fact table says that the attr is null then change it to the page number and only do this the first time.
            updateFactPage(attr.joinKey, fileHandle.getNumberOfPages(), fileHandle);
        }
        
        fileHandle.appendPage(newAttr);
        free(newAttr);
    }
    
}

RC SchemaManager::insertMessage(Attribute attr, void* data, FileHandle fileHandle){


    attr.joinKey++; //Add 1 to attr id cuz sensor id is first fact in table
    int totalPages = fileHandle.getNumberOfPages();
    if(attr.joinKey > getNumAttrs(fileHandle)){ //if its not joining as a fact but is the attr is still more than the num of attrs (IE. Inserting Attr 11 when fact table has 10 attrs then just add an attr and proceed as normal)
        char* attrFact = (char*) malloc(4);
        memcpy(attrFact, &totalPages, 4);
        insertFact(attrFact, fileHandle, attr);
        free(attrFact);

        insertAttrData(attr, data, totalPages, fileHandle);
    }
    else{
        int attrPageNum = getPage(attr.joinKey, fileHandle);
        insertAttrData(attr, data, attrPageNum, fileHandle);
    }

}

RC SchemaManager::readAllData(int pageNum, Attribute attr, FileHandle fileHandle){ //Change data size variable? Store it on first page? Or incorporate it a different way? Or have it so attr details in a DB and its j read from there
    char* dataPage = (char*) malloc(PAGE_SIZE);
    ofstream dataVals("Data.txt", fstream::app);
    int nextPage = pageNum;
    while(nextPage != -1){ //self explanatory
        //cout << "Page: " << pageNum << endl;
        fileHandle.readPage(nextPage, dataPage);
        if(attr.attrType == TypeInt){
            int intValue;
            memcpy(&intValue, dataPage, 4);
            dataVals << "Fact: " << attr.joinKey << endl;
            dataVals << "   Attribute Value: " << intValue << endl;  
        }
        else if(attr.attrType == TypeReal){
            float floatValue; 
            memcpy(&floatValue, dataPage, 4);
            dataVals << "Fact: " << attr.joinKey << endl;
            dataVals << "   Attribute Value: " << floatValue << endl;  
        }
        else{
            int length;
            memcpy(&length, dataPage, 4);
            char* strBuffer = new char[length+1];
            memcpy(strBuffer, dataPage+4, length);
            strBuffer[length] = '\0';

            string strValue(strBuffer);
            dataVals << "Fact: " << attr.joinKey << endl;
            dataVals << "   Attribute Value: " << strValue << endl;  
        }

        memcpy(&nextPage, dataPage+PAGE_SIZE-4, 4);
    }
    free(dataPage);
    return 0;
}

RC SchemaManager::readAllFacts(FileHandle fileHandle){ //self explanatory.
    char* factTable = (char*) malloc(PAGE_SIZE);
    AttributeManager &am = AttributeManager::instance();

    fileHandle.readPage(0, factTable);

    ofstream textFile("Output.txt");

    int numPages;
    memcpy(&numPages, factTable+PAGE_SIZE-8, 4);
    
    
    cout << "This is the number of pages: " << numPages << endl;

    int eop = 0;
    int curPageCount = 0;

    while(eop != -1){
        textFile << "The current fact page is: " << eop << endl;
        fileHandle.readPage(eop, factTable);
        memcpy(&eop, factTable+PAGE_SIZE-12, 4);
        int numFacts;
        memcpy(&numFacts, factTable+PAGE_SIZE-4, 4);

        int offset = 0;
        //cout << "The number of facts is: " << numFacts << endl;
        for(int j = 0; j<numFacts; j++){
            int curFact;
            memcpy(&curFact, factTable+offset, 4);

            textFile << "This is the current fact: " << curFact << endl;

            if(!(eop == 0 && j == 0) && curFact != -1){
                int attrId = (offset/4)+(curPageCount*1021);
                int maxSize = 4;
                int dataType = am.findAttrType(attrId);

                if(dataType == 2){maxSize = 50;}
                
                Attribute attr(attrId, maxSize, dataType);
                //cout << "Attr Id: " << attrId << " Max Size: " << maxSize << " Data Type: " << dataType << endl;
                readAllData(curFact, attr, fileHandle);
            }
            offset += 4;
        }
        curPageCount++;
    }

    free(factTable);
    return 0;
}