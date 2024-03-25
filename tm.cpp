#include "./include/tm.h"


TableManager& TableManager::instance(){
    static TableManager tm = TableManager();
    return tm;
}

TableManager::TableManager() = default;

TableManager::~TableManager() = default;

int TableManager::dbConnect(string dbName){
    string conString = "dbname=" + dbName + " user=postgres password=samrith123 host=/var/run/postgresql";
    con = PQconnectdb(conString.c_str());
    if(PQstatus(con) != CONNECTION_OK){
        con = nullptr;
        cout << "Issue in connection" << endl;
        return -1;
    }
    return 0;
}

int updateAttrs(vector<Attribute> attrs, PGconn* con){
    string getQuery = "SELECT * FROM attributes";
    PGresult *getRes = PQexec(con, getQuery.c_str());
    if(PQresultStatus(getRes) != PGRES_TUPLES_OK){
        cout << "Issue getting attributes: " << PQerrorMessage(con) << endl;
        return -1;
    }
    int numRows = PQntuples(getRes);
    int numCols = 3;

    vector<Attribute> allAttrs;

    for(int i = 0; i < numRows; i++){
        Attribute curAttr;
        curAttr.id = std::stoi(PQgetvalue(getRes, i, 0));
        curAttr.name = PQgetvalue(getRes, i, 1);
        curAttr.type = std::stoi(PQgetvalue(getRes, i, 2));
        allAttrs.push_back(curAttr);
    }
    PQclear(getRes);

    for(int i = 0; i < attrs.size(); i++){
        Attribute searchAttr = attrs[i];

        auto attrSearch = std::find_if(allAttrs.begin(), allAttrs.end(), [&searchAttr](const Attribute &attr){
            return attr.id == searchAttr.id;
        });
        if(attrSearch == allAttrs.end()){
            string insertStr = "INSERT INTO attributes (id, name, type) VALUES ($1, $2, $3)";
            PGresult* prepedStr = PQprepare(con, "", insertStr.c_str(), 3, NULL);
            if(PQresultStatus(prepedStr) != PGRES_COMMAND_OK){
                cout << "Issue prepping" << PQerrorMessage(con) << endl;
                return -1;
            }
            
            vector<const char*> insertValues = {
                to_string(attrs[i].id).c_str(),
                attrs[i].name.c_str(),
                to_string(attrs[i].type).c_str()
            };

            PGresult* insertRes = PQexecPrepared(con, "", 3, insertValues.data(), NULL, NULL, 0);
            if(PQresultStatus(insertRes) != PGRES_COMMAND_OK){
                cout << "Issue inserting" << endl;
                return -1;
            }
            PQclear(insertRes);
        }
    }
}

int updateSensors(vector<Sensor> sensors, PGconn* con){
    string getQuery = "SELECT * FROM sensors";
    PGresult *getRes = PQexec(con, getQuery.c_str());
    if(PQresultStatus(getRes) != PGRES_TUPLES_OK){
        cout << "Issue getting sensors: " << PQerrorMessage(con) << endl;
        return -1;
    }
    int numRows = PQntuples(getRes);
    int numCols = 3;

    vector<Sensor> allSensors;

    for(int i = 0; i < numRows; i++){
        Sensor curSensor;
        curSensor.id = std::stoi(PQgetvalue(getRes, i, 0));
        
        char* attrIdsStr = PQgetvalue(getRes, i, 1);
        vector<int> attrIds;
        char* tempPtr = strtok(attrIdsStr, "{}");
        while(tempPtr != nullptr){
            int curId = stoi(tempPtr);
            attrIds.push_back(curId);
            tempPtr = strtok(nullptr, ",");
        }
        curSensor.attrIds = attrIds;
        allSensors.push_back(curSensor);
    }
    PQclear(getRes);

    for(int i = 0; i < sensors.size(); i++){
        Sensor searchSensor = sensors[i];

        auto attrSearch = std::find_if(allSensors.begin(), allSensors.end(), [&searchSensor](const Sensor &sensor){
            return (sensor.id == searchSensor.id);
        });
        if(attrSearch == allSensors.end()){
            string insertStr = "INSERT INTO sensors (id, attrs) VALUES ($1, $2)";
            PGresult* prepedStr = PQprepare(con, "", insertStr.c_str(), 3, NULL);
            if(PQresultStatus(prepedStr) != PGRES_COMMAND_OK){
                cout << "Issue prepping" << PQerrorMessage(con) << endl;
                return -1;
            }
            
            string sensorString = "{";
            for(int x = 0; x < sensors[i].attrIds.size(); x++){
                sensorString += to_string(sensors[i].attrIds[x]);
                if(x != sensors[i].attrIds.size()-1){
                    sensorString += ",";
                }
            }
            sensorString += "}";

            vector<const char*> insertValues = {
                to_string(sensors[i].id).c_str(),
                sensorString.c_str()
            };

            PGresult* insertRes = PQexecPrepared(con, "", 2, insertValues.data(), NULL, NULL, 0);
            if(PQresultStatus(insertRes) != PGRES_COMMAND_OK){
                cout << "Issue inserting Sensors" << endl;
                return -1;
            }
            PQclear(insertRes);
        }
    }
}

int TableManager::createCatalogs(vector<Attribute> attrs, vector<Sensor> sensors){
    string tableName = "attributes";
    string existsQuery = "SELECT 1 FROM pg_catalog.pg_tables WHERE tablename = '" + tableName + "'";

    PGresult *res = PQexec(con, existsQuery.c_str());

    if(PQresultStatus(res) != PGRES_TUPLES_OK){
        cout << "Error checking existance: " << PQerrorMessage(con) << endl;
        return -1;
    }

    bool attrExists = (PQntuples(res) > 0); 
    cout << "Attr Exists: " << attrExists << endl;
    PQclear(res);

    if(attrExists && attrs.size() != 0){
        updateAttrs(attrs, con);
    }
    else{
        string createQuery = "CREATE TABLE attributes (id INT PRIMARY KEY UNIQUE NOT NULL, name VARCHAR(50) UNIQUE NOT NULL, type INT NOT NULL)"; //
        PGresult* createRes = PQexec(con, createQuery.c_str());
        if(PQresultStatus(createRes) != PGRES_COMMAND_OK){
            cout << "Issue creating: " << PQerrorMessage(con) << endl;
            return -1;
        }
        PQclear(createRes);
        createCatalogs(attrs, sensors);
    }

    tableName = "sensors";
    existsQuery = "SELECT 1 FROM pg_catalog.pg_tables WHERE tablename = '" + tableName + "'";

    PGresult *sensorRes = PQexec(con, existsQuery.c_str());

    if(PQresultStatus(sensorRes) != PGRES_TUPLES_OK){
        cout << "Error checking existance(Sensors): " << PQerrorMessage(con) << endl;
        return -1;
    }

    bool sensorExists = (PQntuples(res) > 0); 
    cout << "Sensor Exists: " << sensorExists << endl; //fix error, Prob caused by the fact that it runs createCatalogs again?
    PQclear(sensorRes);

    if(sensorExists && sensors.size() != 0){
        updateSensors(sensors, con);
    }
    else{
        string createSensorQuery = "CREATE TABLE sensors (id INT PRIMARY KEY UNIQUE NOT NULL, attrs INTEGER[])"; //
        PGresult* createSensorRes = PQexec(con, createSensorQuery.c_str());
        if(PQresultStatus(createSensorRes) != PGRES_COMMAND_OK){
            cout << "Issue Creating(Sensor): " << PQerrorMessage(con) << endl;
            return -1;
        }
        PQclear(createSensorRes);
        createCatalogs(attrs, sensors);
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    vector<Attribute> attrDesc;
    Attribute curAttr;
    
    curAttr.id = 0;
    curAttr.name = "Time";
    curAttr.type = 0;
    attrDesc.push_back(curAttr);

    curAttr.id = 1;
    curAttr.name = "Png";
    curAttr.type = 2;
    attrDesc.push_back(curAttr);

    curAttr.id = 2;
    curAttr.name = "Dollars";
    curAttr.type = 1;
    attrDesc.push_back(curAttr);

    curAttr.id = 3;
    curAttr.name = "Array";
    curAttr.type = 0;
    attrDesc.push_back(curAttr);


    vector<Sensor> sensorDesc;
    Sensor curSensor;

    curSensor.id = 0;
    curSensor.attrIds = {1, 3};
    sensorDesc.push_back(curSensor);

    curSensor.id = 1;
    curSensor.attrIds = {2, 3};
    sensorDesc.push_back(curSensor);

    curSensor.id = 2;
    curSensor.attrIds = {1, 2}; //Add attr id validation. Make sure that the attr Ids do exist and if they dont decide on either appending them or returning error
    sensorDesc.push_back(curSensor);





    TableManager &tm = TableManager::instance();
    
    if(tm.dbConnect("messages") != -1){
        tm.createCatalogs(attrDesc, sensorDesc);
    }



    return 0;
}

