#include "./include/sm.h"
#include <string.h>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <iostream>


int main(int argc, char const *argv[])
{
    PagedFileManager &pfm = PagedFileManager::instance();
    FileHandle fileHandle;

    pfm.createFile("messageSchema.msg");
    pfm.openFile("messageSchema.msg", fileHandle);

    pfm.destroyFile("Data.txt");
    pfm.destroyFile("Output.txt");

    SchemaManager &sm = SchemaManager::instance();
    sm.schemaInit(fileHandle, 1);


    //Pre coded:

    /*char* attrData = (char*) malloc(4);

    for(int j = 1; j <= 11; j++){
        int attr = j*j;
        memcpy(attrData, &attr, 4);

        Attribute newAttr(j, 4, 0);
        sm.insertMessage(newAttr, attrData, fileHandle);
    }
    int nullFact = -1;
    memcpy(attrData, &nullFact, 4);
    Attribute curAttr(1500, 4, 0);
    sm.insertMessage(curAttr, attrData, fileHandle);
    free(attrData);*/

    //Randomly Generated Data:

    srand(time(NULL));

    AttributeManager &am = AttributeManager::instance();
    am.connect();
    am.createTable();

    string attrNames[] = {"temperature", "humidity", "pressure", "light_intensity", "altitude", "wind_speed", "wind_direction", "rainfall","carbon_dioxide_level", "oxygen_level", "carbon_monoxide_level", "nitrogen_dioxide_level", "ozone_level", "pollution_level", "noise_level", "vibration_level", "motion_detected", "proximity", "battery_level", "power_consumption", "energy_usage", "voltage", "current", "frequency", "power_factor", "load", "moisture_level", "soil_pH", "water_level", "flow_rate", "tank_capacity", "liquid_quality", "gas_concentration", "particulate_matter", "smoke_density", "fire_detection", "occupancy_status", "occupancy_count", "people_count", "vehicle_count", "object_count", "door_status", "window_status", "lock_status", "flood_level", "leak_detection", "security_status", "intrusion_detection", "internal_temperature", "external_temperature", "water_temperature", "soil_temperature", "surface_temperature", "air_temperature", "body_temperature", "food_temperature", "ambient_temperature", "coolant_temperature", "engine_temperature", "exhaust_temperature", "inlet_temperature", "outlet_temperature", "core_temperature", "case_temperature", "junction_temperature", "chip_temperature", "bed_temperature", "mold_temperature", "process_temperature", "head_temperature", "hotend_temperature", "coldend_temperature", "printer_temperature", "top_bed_temperature", "bottom_bed_temperature", "filament_temperature", "left_nozzle_temperature", "right_nozzle_temperature", "center_nozzle_temperature", "front_nozzle_temperature", "rear_nozzle_temperature", "inner_nozzle_temperature", "outer_nozzle_temperature", "extruder_nozzle_temperature", "bed_nozzle_temperature", "chamber_nozzle_temperature", "environment_nozzle_temperature", "surroundings_nozzle_temperature", "support_nozzle_temperature", "material_nozzle_temperature", "layer_nozzle_temperature", "speed_nozzle_temperature", "flow_nozzle_temperature", "pressure_nozzle_temperature", "size_nozzle_temperature", "color_nozzle_temperature", "quality_nozzle_temperature", "settings_nozzle_temperature", "offset_nozzle_temperature", "adjustment_nozzle_temperature", "calibration_nozzle_temperature", "compensation_nozzle_temperature", "tolerance_nozzle_temperature", "sensitivity_nozzle_temperature", "accuracy_nozzle_temperature", "precision_nozzle_temperature", "stability_nozzle_temperature", "consistency_nozzle_temperature", "uniformity_nozzle_temperature", "distribution_nozzle_temperature", "variation_nozzle_temperature", "fluctuation_nozzle_temperature", "drift_nozzle_temperature", "noise_nozzle_temperature", "error_nozzle_temperature", "deviation_nozzle_temperature", "offsetting_nozzle_temperature", "correcting_nozzle_temperature", "tracking_nozzle_temperature", "feedback_nozzle_temperature", "control_nozzle_temperature", "regulation_nozzle_temperature", "monitoring_nozzle_temperature", "logging_nozzle_temperature", "reporting_nozzle_temperature"};

    for(int i = 0; i <= 100; i++){
        int type = rand() % 3 + 1;
        //cout << type << endl;

        if(type == 1){
            int intData = rand() % 1000 + 1;
            char* intPage = (char*) malloc(4);
            memcpy(intPage, &intData, 4);
            Attribute intAttr(i, 4, 0);
            sm.insertMessage(intAttr, intPage, fileHandle);
            free(intPage);

            map<int, string> attributes = {{0, attrNames[rand()%125]}};
            am.addAttr(attributes);         
        }
        else if(type == 2){
            int decimalData = rand() % 9999 + 1000;
            float floatData = (rand()%100+1)+(.0001*decimalData);
            char* floatPage = (char*) malloc(4);
            memcpy(floatPage, &floatData, 4);
            Attribute floatAttr(i, 4, 1);    
            sm.insertMessage(floatAttr, floatPage, fileHandle);
            free(floatPage);   

            map<int, string> attributes = {{1, attrNames[rand()%125]}};
            am.addAttr(attributes); 
        }
        else{
            int length = rand() % 20 + 1;
            string strData = "";
            char alphabet[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
            for(int i = 0; i < length; i++){
                strData += alphabet[rand()%25+1];
            }
            char* strPage = (char*) malloc(length+4);
            memcpy(strPage, &length, 4);
            memcpy(strPage+4, strData.c_str(), length);
            
            Attribute strAttr(i, length+4, 2);
            sm.insertMessage(strAttr, strPage, fileHandle);
            free(strPage);
 

            map<int, string> attributes = {{2, attrNames[rand()%125]}};
            am.addAttr(attributes);
        }
    }


    sm.readAllFacts(fileHandle);

    pfm.closeFile(fileHandle);
    pfm.destroyFile("messageSchema.msg");
    am.deleteTable();

    return 0;
}
