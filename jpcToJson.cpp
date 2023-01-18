#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>

#include "json.hpp"
#include "structures.h"


using namespace std;
using json = nlohmann::ordered_json;

/*
*  The Goal of this Program is to convert JPAC2_10 or JPAC2_11 to a json file format. Eventually the goal will be to allow edits.
*  NOTE: JPAC2_11 may not be the same as some fields may be differnent. This tool is also to help identify where differences may be
*  Credit To most of the File Formatting to Jasper over on https://github.com/magcius/noclip.website/blob/master/src/Common/JSYSTEM/JPA.ts
*/

enum RESOURCE_BLOCK {
    BEM1 = 'BEM1',
    BSP1 = 'BSP1',
    ESP1 = 'ESP1',
    ETX1 = 'ETX1',
    SSP1 = 'SSP1',
    FLD1 = 'FLD1',
    KFA1 = 'KFA1',
    TDB1 = 'TDB1',
};

JPAC parseJPC(DataBuffer &jpc);
void write_to_json(JPAC &jpc, string file_name);

int main(int argc, char** argv)
{
    string in_file_name = "";
    string out_file_name = "";
    for (int i = 1; i < argc; i++)
    {   
        string curr_string = argv[i];
        if (curr_string == "-i")
        {
            if (i < argc)
                in_file_name = (string)argv[i+1];
            else
                cout << "No In File Specifed. use -i [filepath]" << endl;
        }
        if (curr_string == "-o")
        {
            if (i+1 < argc)
                out_file_name = argv[i+1];
            else
                cout << "No Out File Specifed. use -o [filepath]" << endl;
        }
        i++;
    }
    if (in_file_name != "")
    {
        DataBuffer jpc(in_file_name);
        JPAC data = parseJPC(jpc);
        if (out_file_name == "")
        {
            out_file_name = "jpc.json";
        }
        write_to_json(data, out_file_name);
        cout << "Completed" << endl;
    }
    else 
        cout << "No File" << endl;
    return 0;
}

JPAC parseJPC(DataBuffer &jpc){
    string version = jpc.read_string(0x00, 8);
    u16 effectCount = jpc.read_u16(0x08);
    u16 textureCount = jpc.read_u16(0x0A);
    u32 textureTableOffset = jpc.read_u32(0x0C);

    cout << "Version  : " << version << endl;
    cout << "effectCnt: " << effectCount << endl;
    cout << "textCnt  : " << textureCount << endl;
    cout << "tblOffset: 0x" << hex << textureTableOffset << endl;

    vector<JPA_ResourceRaw> resources_raw;
    i32 currOffset = 0x10;
    for (i32 i = 0; i < effectCount; i++)
    {
        i32 beginOffset = currOffset;

        u16 resourceId = jpc.read_u16(currOffset+0x00);
        u16 blockCount = jpc.read_u16(currOffset+0x02);

        currOffset += 0x08; // skips first block name (just getting size)
        for (int j = 0; j < blockCount; j++)
        {
            u32 blockSize = jpc.read_u32(currOffset + 0x04);
            currOffset+=blockSize;
        }
        DataBuffer newVec = jpc.read_slice(beginOffset,  currOffset);
        resources_raw.push_back({resourceId, newVec});
    }
    cout << "Finished Resources" << endl;
    vector<JPA_Textures> textures;
    currOffset = textureTableOffset;
    for (i32 i = 0; i < textureCount; i++)
    {
        u32 textureSize = jpc.read_u32(currOffset+0x04);
        string textureName = jpc.read_string(currOffset+0x0C, 0x14, true);
        DataBuffer data = (jpc.read_slice(currOffset, textureSize+currOffset));
        currOffset+=textureSize;
        textures.push_back({(string)textureName.c_str(), data});
    }
    cout << "Finished Textures" << endl;

    vector<JPA_Resource> resources;

    for (JPA_ResourceRaw& rawRes : resources_raw)
    {
        DataBuffer data(rawRes.data);
        JPA_Resource res;
        // cout << "ResId" << rawRes.resourceId << " | size: 0x" << hex << data.getSize() << endl;

        res.resourceId = rawRes.resourceId;
        res.blockCount = data.read_u16(0x02);
        res.fieldBlockCount = data.read_u8(0x04);
        res.keyBlockCount = data.read_u8(0x05);
        res.tdb1Count = data.read_u8(0x06);

        u32 currOffset = 0x08;
        for (i32 i = 0; i < res.blockCount; i++)
        {
            u32 blockType = data.read_u32(currOffset);
            u32 blockSize = data.read_u32(currOffset+0x04);
            switch (blockType)
            {
                default:
                    cout << "UNKNOWN BLOCK" << endl;
                    break;
                case BEM1:
                    res.bem1 = data.read_u8_vec(currOffset, blockSize);
                    break;
                case BSP1:
                    res.bsp1 = data.read_u8_vec(currOffset, blockSize);
                    break;
                case ESP1:
                    res.esp1 = data.read_u8_vec(currOffset, blockSize);
                    break;
                case ETX1:
                    res.etx1 = data.read_u8_vec(currOffset, blockSize);
                    break;
                case SSP1:
                    res.ssp1 = data.read_u8_vec(currOffset, blockSize);
                    break;
                case FLD1:
                    res.fld1.push_back(data.read_u8_vec(currOffset, blockSize));
                    break;
                case KFA1:
                    res.kfa1.push_back(data.read_u8_vec(currOffset, blockSize));
                    break;
                case TDB1:
                    res.tdb1 = data.read_u16_vec(currOffset+0x8, res.tdb1Count);
                    break;
            }
            currOffset+=blockSize;
        }
        resources.push_back(res);
    }

    return {version, resources, textures};
}

void write_to_json(JPAC &jpc, string file_name)
{

    ofstream out(file_name);
    if(out.is_open())
    {
        out.clear();
        json j;
        j["version"] = jpc.version;
        for (int i = 0; i < jpc.resources.size(); i++)
        {
            JPA_Resource res = jpc.resources.at(i);
            vector<string> textureNames;
            for (auto& idx : res.tdb1)
            {
                textureNames.push_back(jpc.textures.at(idx).name);
            }
            j["Effects"][i] = {
                {"effectIdx", i},
                {"resourceId", res.resourceId},
                {"numBlocks", res.blockCount},
                {"numFLD1", res.fieldBlockCount},
                {"numKFA1", res.keyBlockCount},
                {"textureCnt", res.tdb1Count},
                // {"data", jpc.resources.at(i).data},
            };
            if (res.bem1.size() != 0)
                j["Effects"][i]["BEM1"] = 1;
            if (res.bsp1.size() != 0)
                j["Effects"][i]["BSP1"] = 1;
            if (res.esp1.size() != 0)
                j["Effects"][i]["ESP1"] = 1;
            if (res.etx1.size() != 0)
                j["Effects"][i]["ETX1"] = 1;
            if (res.ssp1.size() != 0)
                j["Effects"][i]["SSP1"] = 1;
            if (res.fld1.size() != 0)
                j["Effects"][i]["FLD1"] = res.fld1.size();
            if (res.kfa1.size() != 0)
                j["Effects"][i]["KFA1"] = res.kfa1.size();
            if (res.tdb1.size() != 0)
                j["Effects"][i]["TDB1"] = textureNames;
        }

        for (int i = 0; i < jpc.textures.size(); i++)
        {
            j["Textures"][i] = {
                {"textureIdx", i},
                {"name", jpc.textures.at(i).name},
                //{"data", jpc.resources.at(i).data},
            };
        }
        out << setw(2) << j;
    }
    else
    {
        cout << "Could not find File" << endl;
    }
    out.close();
    return;
}

