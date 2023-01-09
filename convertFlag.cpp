#include <iostream>
#include <fstream>
#include "json.hpp"
#include <filesystem>
#include <regex>
#include <algorithm>
#include "constants.h"
using json = nlohmann::json;
namespace fs = std::filesystem;
const std::string OBJ_TYPES[] = {
    "OBJS",
    "OBJ ",
    "SOBS",
    "SOBJ",
    "STAS",
    "STAG"
};
std::vector<std::string> custom_print(255, "NONE");
int sort_val = -1;
bool doSort = false;
void printExtraInfo(
    std::string Stage, std::string room, std::string layer, std::string field,
    int data, int parsed_data, std::string type_info, json &stage_data, json &objData)
{
    if (doSort && parsed_data != sort_val)
    {
        return;
    }
    std::string id = objData["id"];
    std::printf("%6s: %s: %3s: id:%5s %s: 0x%08X -> 0x%02X", Stage.c_str(), room.c_str(), layer.c_str(), id.c_str() , field.c_str(), data, parsed_data);
    if (type_info == "story")
    {
        
        std::printf(" -> %s\n", STORY_FLAGS[parsed_data < 0x4FF ? parsed_data : 0].c_str());
    } 
    else if (type_info == "scene")
    {
        std::printf(" -> %s\n", SCENE_FLAGS[parsed_data].c_str());
    }
    else if (type_info == "custom")
    {
        std::printf(" -> %s\n", custom_print[parsed_data].c_str());
    }
    else if (type_info == "point")
    {   
        auto &pntData = stage_data["rooms"][room]["PNT "][parsed_data];
        std::printf(" -> (%9.2f, %9.2f, %9.2f)\n", (float)pntData["posx"], (float)pntData["posy"], (float)pntData["posz"]);
    }
    else if (type_info == "item")
    {
        std::printf(" -> %s\n", ITEM_ID[parsed_data].c_str());
    }
    else
    {
        std::printf("\n");
    }
}

void decodeFile(int mask, int shift, std::string field, std::string objName, std::string type_info)
{  
    std::string path = "C:\\Users\\elija\\Documents\\GitHub\\skywardsword-tools\\output\\stage\\";
    for (const auto &entry : fs::directory_iterator(path))
    {
        // std::cout << entry.path() << std::endl;
        std::ifstream f(entry.path());
        std::string Stage = entry.path().stem().string();
        json data = json::parse(f);
        //std::cout << data[objName] << std::endl;
        for (int room = 0; room < 63; room++)
        {
            char roomStr[4];
            std::snprintf(roomStr, 4, "r%.2i", room);
            for( int layer = 0; layer < 29; layer++)
            {
                std::string layerStr = "l" + std::to_string(layer);
                for (auto &objType : OBJ_TYPES)
                {
                    for (auto &objData : data["rooms"][roomStr]["LAY "][layerStr][objType])
                    {   
                        if (objData["name"] == objName)
                        {
                            if (field == "params1" || field == "params2")
                            {
                                std::string params = objData[field];
                                params.erase(std::remove(params.begin(), params.end(), '\"'), params.end());
                                params.erase(std::remove(params.begin(), params.end(), ' '), params.end());
                                int params_int = std::strtoul(params.c_str(), nullptr, 16);
                                int parsed_data = (params_int >> shift)&mask;
                                printExtraInfo(Stage,  roomStr, layerStr, field, params_int, parsed_data, type_info, data, objData);
                            }
                            else if (field == "posx" || field == "posy" || field == "posz" || field == "sizex" || field == "sizey" || field == "sizez")
                            {
                                float paramDat = objData[field];
                                int params_int = *(int *)&paramDat;
                                int parsed_data = (params_int >> shift)&mask;
                                printExtraInfo(Stage,  roomStr, layerStr, field, params_int, parsed_data, type_info, data, objData);
                            }
                            else if (field == "anglex" || field == "angley" || field == "anglez")
                            {
                                int paramDat = objData[field];
                                int params_int = (unsigned int)paramDat;
                                int parsed_data = (params_int >> shift)&mask;
                                printExtraInfo(Stage,  roomStr, layerStr, field, params_int, parsed_data, type_info, data, objData);
                            }
                            else {
                                std::printf("Invalid Field \n");
                            }
                        }
                    }
                }
            }
            for (auto &objData : data["rooms"][roomStr]["SOBJ"])
            {
                if (objData["name"] == objName)
                {
                    if (field == "params1" || field == "params2")
                    {
                        std::string params = objData[field];
                        params.erase(std::remove(params.begin(), params.end(), '\"'), params.end());
                        params.erase(std::remove(params.begin(), params.end(), ' '), params.end());
                        int params_int = std::strtoul(params.c_str(), nullptr, 16);
                        int parsed_data = (params_int >> shift)&mask;
                        printExtraInfo(Stage,  roomStr, "", field, params_int, parsed_data, type_info, data, objData);
                    }
                    else if (field == "posx" || field == "posy" || field == "posz" || field == "sizex" || field == "sizey" || field == "sizez")
                    {
                        float paramDat = objData[field];
                        int params_int = *(int *)&paramDat;
                        int parsed_data = (params_int >> shift)&mask;
                        printExtraInfo(Stage,  roomStr, "", field, params_int, parsed_data, type_info, data, objData);
                    }
                    else if (field == "anglex" || field == "angley" || field == "anglez")
                    {
                        int paramDat = objData[field];
                        int params_int = (unsigned int)paramDat;
                        int parsed_data = (params_int >> shift)&mask;
                        printExtraInfo(Stage,  roomStr, "", field, params_int, parsed_data, type_info, data, objData);
                    }
                    else {
                        std::printf("Invalid Field \n");
                    }
                }
            }
        }
    }
}
int main(int argc, char* argv[]){
    int mask = 0x00;
    int shift = 0x00;
    int input = 0x00;
    std::string field = "params1";
    std::string objName = "";
    bool decode = true;
    std::string type_info = "NONE";
    for(int i = 1; i < argc; i++)
    {
        std::string current_arg = argv[i];
        if (current_arg == "-m")
        {
            mask = std::strtoul(argv[++i], nullptr, 16);
            // std::cout << "Mask is: 0x" << std::hex << mask << std::endl;
        }
        else if (current_arg == "-s")
        {
            shift = std::strtoul(argv[++i], nullptr, 10);
            // std::cout << "Shift is: " << std::dec << shift << std::endl;
        }
        else if (current_arg == "-i")
        {
            input = std::strtoul(argv[++i], nullptr, 16);
            // std::cout << "Input is: 0x" << std::hex << input << std::endl;
        }
        else if (current_arg == "-f")
        {
            field = argv[++i];
        }
        else if (current_arg == "-n")
        {
            objName = argv[++i];
        }
        else if(current_arg == "ALL")
        {
            decode = false;
        }
        else if(current_arg == "-t")
        {
            type_info = argv[++i];
            if (type_info == "custom")
            {
                int num_choices = std::strtoul(argv[++i], nullptr, 10);
                for(int j = 0; j < num_choices && j < argc; j++) {
                    custom_print.at(j) = argv[++i];
                }
            }
        }
        else if (current_arg == "-sort")
        {
            sort_val = std::strtoul(argv[++i], nullptr, 16);
            doSort = true;
        }
        else if (current_arg == "-file")
        {

        }
    }
    if (decode)
    {
        int output = (input >> shift) & mask;
        std::cout << "Output: 0x" << std::hex << output << " (" << std::dec << (uint32_t)output << ")" << std::endl; 
    } else {
        if(objName != "")
        {
            decodeFile(mask, shift, field, objName, type_info);
        }
        else {
            std::cout << "ENTER A OBJ NAME" << std::endl;
        }
    }
}