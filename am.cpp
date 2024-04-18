#include "./include/am.h"
#include <iostream>

AttributeManager &AttributeManager::instance(){
    static AttributeManager am = AttributeManager();
    return am;
}

RC AttributeManager::connect() {
    PGconn *conn = nullptr;
    conn = PQconnectdb("dbname=messages user=postgres password=samrith123 host=/var/run/postgresql");
    if (PQstatus(conn) != CONNECTION_OK) {
        cerr << "Connection to database failed: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        return -1;
    }
    con = conn;
    return 0;
}

RC AttributeManager::addAttr(map<int, string> info) {
    string sql = "INSERT INTO attributes (type, name) VALUES ($1, $2)";
    PGresult *res = PQprepare(con, "", sql.c_str(), 2, NULL);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "Preparation of statement failed: " << PQerrorMessage(con) << endl;
        PQclear(res);
        return -1;
    }
    PQclear(res);

    for (const auto& pair : info) {
        const char *paramValues[2] = {to_string(pair.first).c_str(), pair.second.c_str()};
        res = PQexecPrepared(con, "", 2, paramValues, NULL, NULL, 0);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            cerr << "Insertion failed: " << PQerrorMessage(con) << endl;
            PQclear(res);
            return -1;
        }
        PQclear(res);
    }
    return 0;
}

RC AttributeManager::createTable() {
    if(con == nullptr){connect();}
    const char *query = "CREATE TABLE attributes (ID SERIAL PRIMARY KEY NOT NULL, NAME VARCHAR(255) NOT NULL, TYPE INT NOT NULL)";
    PGresult *res = PQexec(con, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "Failed to create table: " << PQerrorMessage(con) << endl;
        PQclear(res);
        return -1;
    }
    PQclear(res);
}

RC AttributeManager::deleteTable(){ //just for testing purposes.
    if(con==nullptr){connect();}
    const char *query = "DROP TABLE attributes";
    PGresult *res = PQexec(con, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "Failed to delete table: " << PQerrorMessage(con) << endl;
        PQclear(res);
        return -1;
    }
    PQclear(res);   
    return 0;
}

RC AttributeManager::findAttrType(int attrId){
    attrId++;
    if(con==nullptr){connect();}
    const string query = "SELECT * FROM attributes WHERE id=" + to_string(attrId);
    PGresult *res = PQexec(con, query.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "Query failed: " << PQerrorMessage(con) << endl;
        PQclear(res);
        return -1;
    }
    return atoi(PQgetvalue(res, 0, 2));
}
