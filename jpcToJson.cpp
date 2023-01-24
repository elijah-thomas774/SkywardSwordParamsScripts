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
string version_type;
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
struct blockData {
    string block_offset;
    string resource_id;
    // string flags;
    vector<string> data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(blockData, block_offset, resource_id, data);

u32 gflags = 0xFFFFFFFF;
vector<blockData> bspBlocks;
vector<u32> flagCommon;
u32 allFlag = 0;
JPA_BEM1 parse_bem1(DataBuffer &data);
JPA_BSP1 parse_bsp1(DataBuffer &data);
JPA_ESP1 parse_esp1(DataBuffer &data);
JPA_ETX1 parse_etx1(DataBuffer &data);
JPA_SSP1 parse_ssp1(DataBuffer &data);
JPA_FLD1 parse_fld1(DataBuffer &data);
JPA_KFA1 parse_kfa1(DataBuffer &data);
// JPA_TDB1 parse_tsb1(DataBuffer &data);

JPAC parseJPC(DataBuffer &jpc);
void write_to_json(JPAC &jpc, string file_name);

void dump_textures(JPAC &jpc, string file_name)
{
    vector<JPA_Textures> textures = jpc.textures;
    for (JPA_Textures tex : textures)
    {
        string filePath(file_name);
        filePath.append(tex.name);
        filePath.append(".bti");
        char* imgDat = new char[tex.data.getSize()-0x20];
        for (int i = 0; i < (tex.data.getSize()-0x20); i++)
        {
            imgDat[i] = tex.data.read_u8(i+0x20);
        }
        ofstream img(filePath, ios::binary | ios::out);
        img.write(imgDat, (tex.data.getSize()-0x20));
        img.close();
        delete[] imgDat;
        filePath.clear();
    }
}

int main(int argc, char** argv)
{
    string in_file_name = "";
    string out_file_name = "";
    string texture_dump = "";
    bool dumpTextures = false;
    for (int i = 1; i < argc; i++)
    {   
        string curr_string = argv[i];
        if (curr_string == "-i")
        {
            i++;
            if (i < argc)
                in_file_name = argv[i];
            else
                cout << "No In File Specifed. use -i [filepath]" << endl;
        }
        if (curr_string == "-o")
        {
            i++;
            if (i < argc)
                out_file_name = argv[i];
            else
                cout << "No Out File Specifed. use -o [filepath]" << endl;
        }
        if (curr_string == "-to")
        {
            i++;
            if ( i < argc )
                texture_dump = argv[i];
        }
        if (curr_string == "-d")
        {
            dumpTextures = true;
        }
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
        if (texture_dump != "")
        {
            dump_textures(data, texture_dump);
        }
        else if (dumpTextures)
        {
            dump_textures(data, "./TextureInput/");
        }
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
    // cout << "tblOffset: 0x" << hex << textureTableOffset << endl;
    version_type = version;
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
        // TODO : write data to seperate folder
        currOffset+=textureSize;
        textures.push_back({(string)textureName.c_str(), data});
    }
    cout << "Finished Textures" << endl;

    vector<JPA_Resource> resources;
    int overallOffs = 0x8;
    for (JPA_ResourceRaw& rawRes : resources_raw)
    {
        DataBuffer data(rawRes.data);
        JPA_Resource res;
        // cout << "ResId" << rawRes.resourceId << " | size: 0x" << hex << data.getSize() << endl;

        res.resourceId = rawRes.resourceId;
        // cout << "Res ID: " << hex << res.resourceId << " ";
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
                    res.bem1.push_back(parse_bem1(block)); // stays the same for Both JPAC2_10 and JPAC2_11
                    break;
                case BSP1:
                    // cout << hex << currOffset << endl;
                    // print_block_data(block, res.resourceId, overallOffs+currOffset);
                    res.bsp1.push_back(parse_bsp1(block));
                    break;
                case ESP1:
                    res.esp1.push_back(parse_esp1(block));
                    break;
                case ETX1:
                    res.etx1.push_back(parse_etx1(block));
                    break;
                case SSP1:
                    res.ssp1.push_back(parse_ssp1(block));
                    break;
                case FLD1:
                    res.fld1.push_back(parse_fld1(block));
                    break;
                case KFA1:
                    res.kfa1.push_back(parse_kfa1(block));
                    break;
                case TDB1:
                    res.tdb1 = data.read_u16_vec(currOffset+0x8, res.tdb1Count);
                    break;
            }
            currOffset+=blockSize;
        }
        overallOffs+=data.getSize();
        resources.push_back(res);
    }
    // ofstream o("bspBlocks.json");
    // json j = bspBlocks;
    // o << setw(3) << j;
    // o.close();
    // cout << "Common Flags: " << hex << gflags << endl;
    // cout << "All Flags: " << hex << allFlag << endl;
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
    bem1.unkFlags       = data.read_u32(0x0C);
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
    bem1.emitterRot       = data.read_i16_vec(0x68, 3);
    bem1.maxFrame         = data.read_i16(0x6E);
    bem1.startFrame       = data.read_i16(0x70);
    bem1.lifeTime         = data.read_i16(0x72);
    bem1.volumeSize       = data.read_i16(0x74);
    bem1.divNumber        = data.read_i16(0x76);
    bem1.rateStep         = data.read_u8(0x78);
    return bem1;
}

JPA_BSP1 parse_bsp1(DataBuffer &data)
{   
    // This changes based on Version.
    u32 bsp1_start = data.read_u32(0x0);
    u32 bsp1_length = data.read_u32(0x04);
    assert(bsp1_start == BSP1);
    assert(bsp1_length == data.getSize());
    JPA_BSP1 bsp1;
    
    u32 flags = data.read_u32(0x08);
    bsp1.origFlags = flags;
    bsp1.shapeType = (flags & 0xF);
    bsp1.dirType = (flags >> 4) & 0x7;
    bsp1.rotType = (flags >> 7) & 0x7;
    bsp1.planeType = (flags >> 10) & 0x1;

    vector<f32> baseSize;
    bsp1.baseSize = data.read_f32_vec(0x10, 2);
    if (version_type.at(7) == '0')
    {
        bsp1.isEnableProjection   = !!(flags & 0x00100000);
        bsp1.isEnableTexScrollAnm = !!(flags & 0x01000000);
        bsp1.tilingS = !!((flags>>0x19) & 0x1) ? 2.0 : 1.0;
        bsp1.tilingT = !!((flags>>0x1A) & 0x1) ? 2.0 : 1.0;
        bsp1.isDrawFwdAhead  = !!(flags & 0x00200000);
        bsp1.isDrawPrntAhead = !!(flags & 0x00400000);
        bsp1.isNoDrawParent  = !!(flags & 0x08000000);
        bsp1.isNoDrawChild   = !!(flags & 0x10000000);
        bsp1.colorInSelect = (flags >>  0xF) & 0x07;
        bsp1.alphaInSelect = (flags >> 0x12) & 0x01;
    }
    else if (version_type.at(7) == '1')
    {
        // Texture Coordinate Animation
        // cout << "HERE";
        bsp1.isEnableProjection   = !!(flags & 0x00400000);
        bsp1.isEnableTexScrollAnm = !!(flags & 0x04000000);
        bsp1.tilingS = !!((flags>>0x1B) & 0x1) ? 2.0 : 1.0;
        bsp1.tilingT = !!((flags>>0x1C) & 0x1) ? 2.0 : 1.0;
        bsp1.isDrawFwdAhead  = !!(flags & 0x00800000);
        bsp1.isDrawPrntAhead = !!(flags & 0x01000000);
        bsp1.isNoDrawParent  = !!(flags & 0x20000000);
        bsp1.isNoDrawChild   = !!(flags & 0x40000000);
        bsp1.colorInSelect = (flags >>  0xf) & 0x07;
        bsp1.alphaInSelect = (flags >> 0x12) & 0x01;
    }
    bsp1.blendModeFlags = data.read_u16(0x18);
    bsp1.alphaCompareFlags = data.read_u8(0x1A);
    bsp1.alphaRef0 = data.read_u8(0x1B);
    bsp1.alphaRef1 = data.read_u8(0x1C);
    bsp1.zModeFlags = data.read_u8(0x1D);

    bsp1.anmRndm = data.read_u8(0x2E);

    // Texture Palette Animation
    bsp1.isGlblTexAnm  = !!(flags & 0x00004000);
    u8 texFlags = data.read_u8(0x1E);
    bsp1.texCalcIdxType = (texFlags >> 2) & 0x7;
    bsp1.texIdx = data.read_u8(0x20);
    bsp1.texIdxLoopOfstMask = data.read_u8(0x30);
    
    u8 texIdxAnmCount = data.read_u8(0x1F);


    bsp1.texInitTransX = 0;
    bsp1.texInitTransY = 0;
    bsp1.texInitScaleX = 0;
    bsp1.texInitScaleY = 0;
    bsp1.texInitRot = 0;
    bsp1.texIncTransX = 0;
    bsp1.texIncTransY = 0;
    bsp1.texIncScaleX = 0;
    bsp1.texIncScaleY = 0;
    bsp1.texIncRot = 0;
    i32 extraOff = 0x34;
    if (bsp1.isEnableTexScrollAnm)
    {
        // cout << "Here" << endl;
        bsp1.texInitTransX = data.read_f32(extraOff + 0x00);
        bsp1.texInitTransY = data.read_f32(extraOff + 0x04);
        bsp1.texInitScaleX = data.read_f32(extraOff + 0x08);
        bsp1.texInitScaleY = data.read_f32(extraOff + 0x0C);
        bsp1.texInitRot    = data.read_f32(extraOff + 0x10);
        bsp1.texIncTransX  = data.read_f32(extraOff + 0x14);
        bsp1.texIncTransY  = data.read_f32(extraOff + 0x18);
        bsp1.texIncScaleX  = data.read_f32(extraOff + 0x1C);
        bsp1.texIncScaleY  = data.read_f32(extraOff + 0x20);
        bsp1.texIncRot     = data.read_f32(extraOff + 0x24);
        extraOff += 0x28;
    }
    bsp1.texFlags = texFlags;
    if (!!(texFlags & 0x1))
        bsp1.texIdxAnimData = data.read_u8_vec(extraOff, texIdxAnmCount);

    // Color Animation Settings
    bsp1.isGlblClrAnm  = !!(flags & 0x00001000);
    u8 colorFlags = data.read_u8(0x21);
    bsp1.colorFlags = colorFlags;
    bsp1.colorCalcIdxType = (colorFlags >> 4) & 0x7;
    bsp1.colorLoopOfstMask = data.read_u8(0x2F);
    bsp1.colorPrm = data.read_color(0x26);
    bsp1.colorEnv = data.read_color(0x2A);
    bsp1.colorAnimMaxFrm = data.read_u16(0x24);
    if (!!(colorFlags & 0x02) || data.read_u16(0xC) != 0)
    {
        u16 colorPrmDataOffs = data.read_u16(0xC);
        u8 colorPrmDataCount = data.read_u8(0x22);
        for(int i = 0; i < colorPrmDataCount; i++)
        {
            bsp1.colorPrmAnimData.push_back({
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
            bsp1.colorEnvAnimData.push_back({
                data.read_u16(colorEnvDataOffs + 6*i),
                data.read_color(colorEnvDataOffs + 6*i + 2)
            });
        }
    }
    return bsp1;
}

JPA_ESP1 parse_esp1(DataBuffer &data)
{
    u32 esp_type = data.read_u32(0x0);
    u32 esp_len = data.read_u32(0x4);
    assert(esp_type == ESP1);
    if (!(esp_len == 0x60 && esp_len == data.getSize()))
        cout << esp_len << endl;
    assert(esp_len == 0x60 && esp_len == data.getSize());

    JPA_ESP1 esp1;
    u32 flags = data.read_u32(0x08);
    esp1.origFlags = flags;
    esp1.isEnableScale = !!(flags & 0x1);
    esp1.isDiffXY = !!(flags & 0x2);
    esp1.scaleAnmTypeX = (flags >> 0x08) & 0x03;
    esp1.scaleAnmTypeY = (flags >> 0x0A) & 0x03;
    esp1.isEnableAlpha  = !!(flags & 0x00010000);
    esp1.isEnableSinWave = !!(flags & 0x00020000);
    esp1.isEnableRotate = !!(flags & 0x01000000);
    esp1.pivotX = (flags >> 0x0C) & 0x03;
    esp1.pivotY = (flags >> 0x0E) & 0x03;
    esp1.scaleInTiming = data.read_f32(0xC);
    esp1.scaleOutTiming = data.read_f32(0x10);
    esp1.scaleInValueX = data.read_f32(0x14);
    esp1.scaleOutValueX = data.read_f32(0x18);
    esp1.scaleInValueY = data.read_f32(0x1C);
    esp1.scaleOutValueY = data.read_f32(0x20);
    esp1.scaleOutRandom = data.read_f32(0x24);
    esp1.scaleAnmMaxFrameX = data.read_u16(0x28);
    esp1.scaleAnmMaxFrameY = data.read_u16(0x2A);
    esp1.alphaInTiming = data.read_f32(0x2C);
    esp1.alphaOutTiming = data.read_f32(0x30);
    esp1.alphaInValue = data.read_f32(0x34);
    esp1.alphaBaseValue = data.read_f32(0x38);
    esp1.alphaOutValue = data.read_f32(0x3C);
    esp1.alphaWaveRandom = data.read_f32(0x44);
    esp1.alphaWaveFrequency = data.read_f32(0x40);
    esp1.alphaWaveAmplitude = data.read_f32(0x48);
    esp1.rotateAngle = data.read_f32(0x4C);
    esp1.rotateAngleRandom = data.read_f32(0x50);
    esp1.rotateSpeed = data.read_f32(0x54);
    esp1.rotateSpeedRandom = data.read_f32(0x58);
    esp1.rotateDirection = data.read_f32(0x5C);
    return esp1;
}

JPA_ETX1 parse_etx1(DataBuffer &data)
{
    u32 etx_type = data.read_u32(0x0);
    u32 etx_len = data.read_u32(0x4);
    
    assert(etx_type == ETX1);
    assert((etx_len == 0x28  && etx_len == data.getSize()) || (etx_len == 0x50  && etx_len == data.getSize()));

    JPA_ETX1 etx1;
    u32 flags = data.read_u32(0x8);
    if (version_type.at(7) == '0')
    {   
        etx1.indTextureMode = (flags & 0x1);
        etx1.floats = data.read_f32_vec(0xC, 6);
        etx1.unk = 0;
        etx1.indTextureID = data.read_u8(0x25);
        etx1.scale = data.read_i8(0x24);
        etx1.secondTextureID = !!(flags & 0x100) ? data.read_u8(0x26) : -1;
    }
    else if (version_type.at(7) == '1')
    {
        // cout << "here" << endl;
        etx1.indTextureMode = (flags & 0x1);
        etx1.floats = data.read_f32_vec(0xC, 16);
        etx1.unk = data.read_u8(0x4C);
        etx1.indTextureID = data.read_u8(0x4E);
        etx1.scale = data.read_i8(0x4D);
        etx1.secondTextureID = !!(flags & 0x100) ? data.read_u8(0x4F) : -1;
    }
    return etx1;
}

JPA_SSP1 parse_ssp1(DataBuffer &data)
{

    u32 ssp1_type = data.read_u32(0x0);
    u32 ssp1_len = data.read_u32(0x4);
    
    assert(ssp1_type == SSP1);
    if (ssp1_len != 0x48) {
        cout << ssp1_len << endl;
        assert(ssp1_len == 0x48 == data.getSize());
    }
    JPA_SSP1 ssp1;
    u32 flags = data.read_u32(0x08);
    ssp1.origFlags = flags;
    ssp1.isInheritedScale = !!(flags & 0x00010000);
    ssp1.isInheritedAlpha = !!(flags & 0x00020000);
    ssp1.isInheritedRGB =   !!(flags & 0x00040000);
    ssp1.isEnableField =    !!(flags & 0x00200000);
    ssp1.isEnableScaleOut = !!(flags & 0x00400000);
    ssp1.isEnableAlphaOut = !!(flags & 0x00800000);
    ssp1.isEnableRotate =   !!(flags & 0x01000000);
    ssp1.shapeType = flags & 0xF;
    ssp1.dirType = (flags >> 4) & 0x7;
    ssp1.rotType = (flags >> 7) & 0x7;
    ssp1.planeType = (flags >> 10) & 0x1;
    ssp1.posRndm = data.read_f32(0xC);
    ssp1.baseVel = data.read_f32(0x10);
    ssp1.baseVelRndm = data.read_f32(0x14);
    ssp1.velInfRate = data.read_f32(0x18);
    ssp1.gravity = data.read_f32(0x1C);
    ssp1.globalScale2D = data.read_f32_vec(0x20, 2);
    ssp1.inheritScale = data.read_f32(0x28);
    ssp1.inheritAlpha = data.read_f32(0x2C);
    ssp1.inheritRGB = data.read_f32(0x30);
    ssp1.colorPrm = data.read_color(0x34);
    ssp1.colorEnv = data.read_color(0x38);
    ssp1.timing = data.read_f32(0x3C);
    ssp1.life = data.read_u16(0x40);
    ssp1.rate = data.read_u16(0x42);
    ssp1.step = data.read_u8(0x44);
    ssp1.texIdx = data.read_u8(0x45);
    ssp1.rotateSpeed = data.read_u16(0x46);

    return ssp1;
}

JPA_FLD1 parse_fld1(DataBuffer &data)
{

    u32 fld1_type = data.read_u32(0x0);
    u32 fld1_len = data.read_u32(0x4);
    
    assert(fld1_type == FLD1);
    assert(fld1_len == 0x44 && fld1_len == data.getSize());
    JPA_FLD1 fld1;
    u32 flags = data.read_u32(0x08);
    fld1.origFlags = flags;
    fld1.sttFlag = (flags >> 0x10);
    fld1.type    = flags & 0xF;
    fld1.addType = (flags >> 8) & 0x3;
    fld1.pos     = data.read_f32_vec(0xC, 3);
    fld1.dis     = data.read_f32_vec(0x18, 3);
    fld1.param1  = data.read_f32(0x24);
    fld1.param2  = data.read_f32(0x28);
    fld1.param3  = data.read_f32(0x2C);
    fld1.fadeIn  = data.read_f32(0x30);
    fld1.fadeOut = data.read_f32(0x34);
    fld1.enTime  = data.read_f32(0x38);
    fld1.disTime = data.read_f32(0x3C);
    fld1.cycle   = data.read_u8(0x40);
    // rest is buffer
    return fld1;
}

JPA_KFA1 parse_kfa1(DataBuffer &data)
{
    // cout << "KFA" << endl;
    u32 kfa1_type = data.read_u32(0x0);
    u32 kfa1_len = data.read_u32(0x4);
    
    assert(kfa1_type == KFA1);
    assert(kfa1_len == data.getSize());
    JPA_KFA1 kfa1;
    kfa1.keyType = data.read_u8(0x08);
    kfa1.keyCount = data.read_u8(0x09);
    kfa1.unk0xA = data.read_u8(0xA);
    kfa1.isLoopEnable = !!(data.read_u8(0x0B));
    kfa1.keyValues = data.read_f32_vec(0x0C, kfa1.keyCount*4);
    return kfa1;
}

void write_to_json(JPAC &jpc, string file_name)
{

    ofstream out(file_name);
    if(out.is_open())
    {
        out.clear();
        json j = jpc;
        out << setw(4) << j;
    }
    else
    {
        cout << "Could not find File" << endl;
    }
    out.close();
    return;
}

