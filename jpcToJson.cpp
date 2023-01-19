#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>

#include "json.hpp"
#include "structures.h"



using namespace std;
using json = nlohmann::json;
// using JPA_BEM1 = jpaBEM1::JPA_BEM1;

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

JPA_BEM1 parse_bem1(DataBuffer &data);
JPA_BSP1 parse_bsp1(DataBuffer &data);
JPA_ESP1 parse_esp1(DataBuffer &data);
JPA_ETX1 parse_etx1(DataBuffer &data);
JPA_SSP1 parse_ssp1(DataBuffer &data);
JPA_FLD1 parse_fld1(DataBuffer &data);
JPA_KFA1 parse_kfa1(DataBuffer &data);
JPA_TDB1 parse_tsb1(DataBuffer &data);

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
        DataBuffer block;
        u32 currOffset = 0x08;
        for (i32 i = 0; i < res.blockCount; i++)
        {
            u32 blockType = data.read_u32(currOffset);
            u32 blockSize = data.read_u32(currOffset+0x04);
            block = data.read_slice(currOffset, currOffset+blockSize);
            switch (blockType)
            {
                default:
                    cout << "UNKNOWN BLOCK" << endl;
                    break;
                case BEM1:
                    res.bem1.push_back(parse_bem1(block));
                    break;
                case BSP1:
                    res.bsp1.push_back(parse_bsp1(block));
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

JPA_BEM1 parse_bem1(DataBuffer &data)
{
    u32 bem1_start = data.read_u32(0x0);
    u32 bem1_length = data.read_u32(0x04);
    assert(bem1_start == BEM1);
    assert(bem1_length == data.getSize());
    assert(bem1_length == 0x7C); // the size expected for these files

    JPA_BEM1 bem1;
    
    bem1.emitFlags      = data.read_u32(0x08);
    bem1.volumeType = (bem1.emitFlags >> 8) & 0x7;
    bem1.emitterScl     = data.read_vec3f(0x10);
    bem1.emitterTrs     = data.read_vec3f(0x1C);
    bem1.emitterDir     = data.read_vec3f(0x28);
    bem1.initialVelOmni   = data.read_f32(0x34);
    bem1.initialVelAxis   = data.read_f32(0x38);
    bem1.initialVelRndm   = data.read_f32(0x3C);
    bem1.initialVelDir    = data.read_f32(0x40); 
    bem1.spread           = data.read_f32(0x44);
    bem1.initialVelRatio  = data.read_f32(0x48);
    bem1.rate             = data.read_f32(0x4C);   
    bem1.rateRndm         = data.read_f32(0x50);
    bem1.lifeTimeRndm     = data.read_f32(0x54);
    bem1.volumeSweep      = data.read_f32(0x58);
    bem1.volumeMinRad     = data.read_f32(0x5C);
    bem1.airResist        = data.read_f32(0x60);
    bem1.momentRndm       = data.read_f32(0x64);
    bem1.emitterRot     = data.read_vec3f(0x68);
    bem1.maxFrame         = data.read_i16(0x6E);
    bem1.startFrame       = data.read_i16(0x70);
    bem1.lifeTime         = data.read_i16(0x72);
    bem1.volumeSize       = data.read_i16(0x74);
    bem1.divNumber        = data.read_i16(0x76);
    bem1.rateStep         = data.read_u8(0x78);
    bem1.moment = 1.0;

    return bem1;
}

JPA_BSP1 parse_bsp1(DataBuffer &data)
{    
    u32 bsp1_start = data.read_u32(0x0);
    u32 bsp1_length = data.read_u32(0x04);
    assert(bsp1_start == BSP1);
    assert(bsp1_length == data.getSize());
    JPA_BSP1 bsp1;
    
    u32 flags = data.read_u32(0x08);
    u8 shapeType = (flags & 0xF);
    u8 dirType = (flags >> 4) & 0x7;
    u8 rotType = (flags >> 7) & 0x7;
    u8 planeType = (flags >> 10) & 0x1;

    f32 tilingS = !!((flags>>0x19) & 0x19) ? 2.0 : 1.0;
    f32 tilingT = !!((flags>>0x1A) & 0x1A) ? 2.0 : 1.0;

    vector<f32> baseSize;
    bool isDrawFwdAhead  = !!(flags & 0x00200000);
    bool isDrawPrntAhead = !!(flags & 0x00400000);
    bool isNoDrawParent  = !!(flags & 0x08000000);
    bool isNoDrawChild   = !!(flags & 0x10000000); 

    // TEV/PE Settings
    u8 colorInSelect = (flags >>  0xF) & 0x07;
    u8 alphaInSelect = (flags >> 0x12) & 0x01;
    u16 blendModeFlags = data.read_u16(0x18);
    u8 alphaCompareFlags = data.read_u8(0x1A);
    u8 alphaRef0 = data.read_u8(0x1B);
    u8 alphaRef1 = data.read_u8(0x1C);
    u8 zModeFlags = data.read_u8(0x1D);

    u8 anmRndm = data.read_u8(0x2E);

    // Texture Palette Animation
    bool isGlblTexAnm  = !!(flags & 0x00004000);
    u8 texFlags = data.read_u8(0x1E);
    u8 texCalcIdxType = (texFlags >> 2) & 0x7;
    u8 texIdx = data.read_u8(0x20);
    u8 texIdxLoopOfstMask = data.read_u8(0x30);
    
    u8 texIdxAnmCount = data.read_u8(0x1F);

    // Texture Coordinate Animation
    bool isEnableProjection   = !!(flags & 0x00100000);
    bool isEnableTexScrollAnm = !!(flags & 0x01000000);

    f32 texInitTransX = 0;
    f32 texInitTransY = 0;
    f32 texInitScaleX = 0;
    f32 texInitScaleY = 0;
    f32 texInitRot = 0;
    f32 texIncTransX = 0;
    f32 texIncTransY = 0;
    f32 texIncScaleX = 0;
    f32 texIncScaleY = 0;
    f32 texIncRot = 0;
    i32 extraOff = 0x34;
    if (isEnableTexScrollAnm)
    {
        texInitTransX = data.read_f32(extraOff + 0x00);
        texInitTransY = data.read_f32(extraOff + 0x04);
        texInitScaleX = data.read_f32(extraOff + 0x08);
        texInitScaleY = data.read_f32(extraOff + 0x0C);
        texInitRot    = data.read_f32(extraOff + 0x10);
        texIncTransX  = data.read_f32(extraOff + 0x14);
        texIncTransY  = data.read_f32(extraOff + 0x18);
        texIncScaleX  = data.read_f32(extraOff + 0x1C);
        texIncScaleY  = data.read_f32(extraOff + 0x20);
        texIncRot     = data.read_f32(extraOff + 0x24);
        extraOff += 0x28;
    }

    vector<u8> texIdxAnimData;
    if (!!(texFlags & 0x1))
        texIdxAnimData = data.read_u8_vec(extraOff, texIdxAnmCount);

    // Color Animation Settings
    bool isGlblClrAnm  = !!(flags & 0x00001000);
    u8 colorFlags = data.read_u8(0x21);
    u8 colorCalcIdxType = (colorFlags >> 4) & 0x7;
    u8 colorLoopOfstMask = data.read_u8(0x30);
    Color colorPrm = data.read_color(0x26);
    Color colorEnv = data.read_color(0x2A);
    u16 colorAnimMaxFrm = data.read_u16(0x24);
    vector<pair<u16, Color>> colorPrmAnimData;
    vector<pair<u16, Color>> colorEnvAnimData;
    if (!!(colorFlags & 0x02))
    {
        u16 colorPrmDataOffs = data.read_u16(0xC);
        u8 colorPrmDataCount = data.read_u8(0x22);
        for(int i = 0; i < colorPrmDataCount; i++)
        {
            colorPrmAnimData.push_back({
                data.read_u16(colorPrmDataOffs + 6*i),
                data.read_color(colorPrmDataOffs + 6*i + 2)
            });
        }
    }   
    if (!!(colorFlags & 0x08))
    {
        u16 colorEnvDataOffs = data.read_u16(0xE);
        u8 colorEnvDataCount = data.read_u8(0x23);
        for(int i = 0; i < colorEnvDataCount; i++)
        {
            colorPrmAnimData.push_back({
                data.read_u16(colorEnvDataOffs + 6*i),
                data.read_color(colorEnvDataOffs + 6*i + 2)
            });
        }
    }
    bsp1 = {
                shapeType, dirType, rotType, planeType, baseSize, tilingS, tilingT, isDrawFwdAhead, isDrawPrntAhead, isNoDrawParent, isNoDrawChild,
                colorInSelect, alphaInSelect, blendModeFlags, alphaCompareFlags, alphaRef0, alphaRef1, zModeFlags,
                anmRndm,
                isGlblTexAnm, texCalcIdxType, texIdx, texIdxAnimData, texIdxLoopOfstMask,
                isEnableTexScrollAnm, isEnableProjection,
                texInitTransX, texInitTransY, texInitScaleX, texInitScaleY, texInitRot,
                texIncTransX, texIncTransY, texIncScaleX, texIncScaleY, texIncRot,
                isGlblClrAnm, colorCalcIdxType, colorPrm, colorEnv, colorEnvAnimData, colorPrmAnimData, colorAnimMaxFrm, colorLoopOfstMask,
            };
    return bsp1;
}

JPA_ESP1 parse_esp1(DataBuffer &data)
{
    JPA_ESP1 bsp1;
    return bsp1;
}

JPA_ETX1 parse_etx1(DataBuffer &data)
{
    JPA_ETX1 bsp1;
    return bsp1;
}

JPA_SSP1 parse_ssp1(DataBuffer &data)
{
    JPA_SSP1 bsp1;
    return bsp1;
}

JPA_FLD1 parse_fld1(DataBuffer &data)
{
    JPA_FLD1 bsp1;
    return bsp1;
}

JPA_KFA1 parse_kfa1(DataBuffer &data)
{
    JPA_KFA1 bsp1;
    return bsp1;
}

JPA_TDB1 parse_tsb1(DataBuffer &data)
{
    JPA_TDB1 tdb1;
    return tdb1;
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
                // {"numBlocks", res.blockCount},
                // {"numFLD1", res.fieldBlockCount},
                // {"numKFA1", res.keyBlockCount},
                // {"textureCnt", res.tdb1Count},
                // {"data", jpc.resources.at(i).data},
            };
            if (res.bem1.size() != 0){
                // JPA_BEM1 bem1 = res.bem1.at(0);
                // json j1 = bem1;
                j["Effects"][i]["BEM1"] = res.bem1.at(0);
            }
            if (res.bsp1.size() != 0)
                j["Effects"][i]["BSP1"] = res.bsp1.at(0);
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

        // for (int i = 0; i < jpc.textures.size(); i++)
        // {
        //     j["Textures"][i] = {
        //         {"textureIdx", i},
        //         {"name", jpc.textures.at(i).name},
        //         //{"data", jpc.resources.at(i).data},
        //     };
        // }
        out << setw(4) << j;
    }
    else
    {
        cout << "Could not find File" << endl;
    }
    out.close();
    return;
}

