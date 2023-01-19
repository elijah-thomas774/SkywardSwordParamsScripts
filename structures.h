#ifndef STRUCTURES_H
#define STRUCTURES_H

// #include <cinttypes>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
// most if not all are copies of https://github.com/magcius/noclip.website/blob/d7dd9e7212792ad4454cd00d2bdabc2ce78fea07/src/Common/JSYSTEM/JPA.ts#L4425
// Used with Permission from owner

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef float    f32;
typedef double   f64;
typedef vector<f32> vec3f;

struct Color {
    u8 r,g,b,a;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b, a);

class DataBuffer {
    public:
        DataBuffer(){
            data = vector<u8>(0);
            size = data.size();
        }
        DataBuffer(i32 n) {
            data = vector<u8>(n);
            size = data.size();
            assert(size == n);            
        }
        DataBuffer(const vector<u8> &vec){
            data = vector<u8>(vec);
            size = data.size();
            assert(vec == data);
        }
        DataBuffer(string file_name){
            // I am using a binary read template from cplusplus.com
            ifstream jpcFile(file_name, ios::binary | ios::in | ios::ate);
            if(jpcFile.is_open())
            {
                size = jpcFile.tellg();
                data = vector<u8>(size);
                u8* newdata = new u8[size]; // allocs enough mem for the file
                jpcFile.seekg(0, ios::beg);
                jpcFile.read((char *)newdata, size);
                jpcFile.close(); // at this point, the file should be completely in mem
                for (int i = 0; i < size; i++)
                    this->data.at(i) = (newdata[i]);
                assert(data.size() == size);
                delete[] newdata;

            } else {
                cout << "Could not locate file" << endl;
            }
        }
        ~DataBuffer(){}
        DataBuffer(const DataBuffer& old){
            data = vector<u8>(old.data);
            size = data.size();
            assert(data == old.data);
        }
        bool checkOffset(i32, i32);
        u8 read_u8(i32);
        i8 read_i8(i32);
        u16 read_u16(i32);
        i16 read_i16(i32);
        u32 read_u32(i32);
        i32 read_i32(i32);
        f32 read_f32(i32);
        vec3f read_vec3f(i32);
        Color read_color(i32);
        DataBuffer read_slice(i32, i32);

        string read_string(i32, i32);
        string read_string(i32, i32, bool);
        vector<u8> read_u8_vec(i32, i32);
        vector<u16> read_u16_vec(i32, i32);
        vector<u32> read_u32_vec(i32, i32);
        vector<i8> read_i8_vec(i32, i32);
        vector<i16> read_i16_vec(i32, i32);
        vector<i32> read_i32_vec(i32, i32);
        vector<f32> read_f32_vec(i32, i32);
        
        i32 getSize(){return size;}
    private:
        vector<u8> data;
        i32 size;
};
bool DataBuffer::checkOffset(i32 offset, i32 n = 1){
    bool test = (offset < 0 || offset+n > size);
    if (test)
        if (offset < 0)
            cout << "Cannot Reference negative offsets" << endl;
        else
            cout << "Offset of 0x" << hex << offset << " is out of bounds when trying to read " << hex << n << " bytes" << endl;
    assert(!test);
    return test;
}
u8 DataBuffer::read_u8(i32 offset){
    if (checkOffset(offset))
        return 0;
    return (u8)data.at(offset)&0xFF;
}
i8 DataBuffer::read_i8(i32 offset){
    if (checkOffset(offset))
        return 0;
    return (i8)data.at(offset);
}
u16 DataBuffer::read_u16(i32 offset){
    if (checkOffset(offset,2))
        return 0;
    u16 t1 = ((((u16)data[offset+0])&0xFF)<< 8);
    u16 t2 = ((((u16)data[offset+1])&0xFF)<< 0);
    return (u16)(t1 | t2);
}
i16 DataBuffer::read_i16(i32 offset){
    if (checkOffset(offset,2))
        return 0;
    u16 t1 = ((((u16)data[offset+0])&0xFF)<< 8);
    u16 t2 = ((((u16)data[offset+1])&0xFF)<< 0);
    return (i16)(t1 | t2);
}
u32 DataBuffer::read_u32(i32 offset){
    if (checkOffset(offset, 4))
        return 0;
    u32 t1 = ((((u32)data[offset+0])&0xFF)<< 24);
    u32 t2 = ((((u32)data[offset+1])&0xFF)<< 16);
    u32 t3 = ((((u32)data[offset+2])&0xFF)<<  8);
    u32 t4 = ((((u32)data[offset+3])&0xFF)<<  0);
    return (u32)(t1 | t2 | t3 | t4);
}
i32 DataBuffer::read_i32(i32 offset){
    if (checkOffset(offset, 4))
        return 0;
    u32 t1 = ((((u32)data[offset+0])&0xFF)<< 24);
    u32 t2 = ((((u32)data[offset+1])&0xFF)<< 16);
    u32 t3 = ((((u32)data[offset+2])&0xFF)<<  8);
    u32 t4 = ((((u32)data[offset+3])&0xFF)<<  0);
    return (i32)(t1 | t2 | t3 | t4);
}
f32 DataBuffer::read_f32(i32 offset){
    if (checkOffset(offset, 4))
        return 0.0;
    u32 t = read_u32(offset);
    return *(f32*)&t;
}
vec3f DataBuffer::read_vec3f(i32 offset){
    vec3f vec = {0.0, 0.0, 0.0};
    
    if (checkOffset(offset, 12))
        return vec;

    f32 x = read_f32(offset);
    f32 y = read_f32(offset);
    f32 z = read_f32(offset);

    vec = {x, y, z};

    return vec;
}
Color DataBuffer::read_color(i32 offset){
    vector<u8> colorBytes = read_u8_vec(offset, 4);
    return {colorBytes[0], colorBytes[1], colorBytes[2], colorBytes[3]};
}
DataBuffer DataBuffer::read_slice(i32 offset, i32 offset2){
    if(checkOffset(offset, offset2-offset))
        return DataBuffer(); // nothing will work on this data buffer
    
    DataBuffer newBuff(offset2-offset);
    for (i32 i = 0; i < offset2-offset; i++)
    {
        newBuff.data.at(i) = read_u8(offset+i);
    }
    return newBuff;
}
string DataBuffer::read_string(i32 offset, i32 n){
    if(checkOffset(offset, n))
        return "";
    string str = "";
    for (i32 i = 0; i < n; i++)
    {
        str.append(1, data.at(offset+i));
    }
    return str;
}
string DataBuffer::read_string(i32 offset, i32 n, bool stopOnNull){
    if(checkOffset(offset, n))
        return "";
    string str = "";
    for (i32 i = 0; i < n; i++)
    {   
        str.append(1, data.at(offset+i));
        if (stopOnNull && data.at(offset+i) == '\0')
           break; 
    }
    return str;
}
vector<u8>  DataBuffer::read_u8_vec(i32 offset, i32 n){
    vector<u8> vecData(n);
    if (checkOffset(offset, n*sizeof(u8)))
        return vecData;
    for (int i = 0; i < n; i++)
    {
        vecData.at(i)= read_u8(i+offset);
    }
    return vecData;
}
vector<u16> DataBuffer::read_u16_vec(i32 offset, i32 n){
    vector<u16> vecData(n);
    if (checkOffset(offset, n*sizeof(u16)))
        return vecData;
    
    for (int i = 0; i < n; i++)
    {
        vecData.at(i) = read_u16(i*sizeof(u16) + offset);
    }
    return vecData;
}
vector<u32> DataBuffer::read_u32_vec(i32 offset, i32 n){
    vector<u32> vecData(n);
    if (checkOffset(offset, n*sizeof(u32)))
        return vecData;
    
    for (int i = 0; i < n; i++)
    {
        vecData.at(i) = read_u32(i*sizeof(u32) + offset);
    }
    return vecData;
}
vector<i8>  DataBuffer::read_i8_vec(i32 offset, i32 n){
    vector<i8> vecData(n);
    if (checkOffset(offset, n*sizeof(i8)))
        return vecData;
    
    for (int i = 0; i < n; i++)
    {
        vecData[i] = read_i8(i+offset);
    }
    return vecData;
}
vector<i16> DataBuffer::read_i16_vec(i32 offset, i32 n){
    vector<i16> vecData(n);
    if (checkOffset(offset, n*sizeof(i16)))
        return vecData;
    
    for (int i = 0; i < n; i++)
    {
        vecData[i] = read_i16(i*sizeof(i16) + offset);
    }
    return vecData;
}
vector<i32> DataBuffer::read_i32_vec(i32 offset, i32 n){
    vector<i32> vecData(n);
    if (checkOffset(offset, n*sizeof(i32)))
        return vecData;
    
    for (int i = 0; i < n; i++)
    {
        vecData[i] = read_i32(i*sizeof(i32) + offset);
    }
    return vecData;
}
vector<f32> DataBuffer::read_f32_vec(i32 offset, i32 n){
    vector<f32> vecData(n);
    if (checkOffset(offset, n*sizeof(f32)))
        return vecData;
    
    for (int i = 0; i < n; i++)
    {
        vecData[i] = read_f32(i*sizeof(f32) + offset);
    }
    return vecData;
}

struct BTI_header {
   /* 0x00 */ u8  format;
   /* 0x01 */ u8  enAlpha;
   /* 0x02 */ u16 width;
   /* 0x04 */ u16 height;
   /* 0x06 */ u8  wrapS;
   /* 0x07 */ u8  wrapT;
   /* 0x08 */ u16 palletFormat;
   /* 0x0A */ u16 numPalletEntries;
   /* 0x0C */ u32 palletDataOffset;
   /* 0x10 */ u32 unk1;
   /* 0x14 */ u8  magnificationFilterType;
   /* 0x15 */ u8  minificationFilterType;
   /* 0x16 */ u16 unk2;
   /* 0x18 */ u8  totalNumberOfImages;
   /* 0x19 */ u8  unk3;
   /* 0x1A */ u16 unk4;
};

// https://github.com/magcius/noclip.website/blob/5644a8b71f7a8ca28cbb2e027069c4ea85ca211d/src/Common/JSYSTEM/JPA.ts#L4450
// JPA Dynamics Block
// Emitter settings and details on how it simulates 
// commented out fields are not present in JPAC2, left in, but commented to add support later if i decide to
  
class JPA_BEM1 { 
    public:
    u32 emitFlags;
    u8 volumeType;
    vec3f emitterScl;
    vec3f emitterRot; // in degrees
    vec3f emitterTrs;
    vec3f emitterDir;

    f32 initialVelOmni;
    f32 initialVelAxis;
    f32 initialVelRndm;
    f32 initialVelDir; 

    f32 spread;
    f32 initialVelRatio;
    f32 rate;
    f32 rateRndm;
    f32 lifeTimeRndm;
    f32 volumeSweep;
    f32 volumeMinRad;
    f32 airResist;
    // u32 airResistRndm;
    f32 moment; // will always be 1? (based on comment in Japsers code)
    u32 momentRndm;
    // u32 accel;
    // u32 accelRndm;
    i16 maxFrame;
    i16 startFrame;
    i16 lifeTime;
    i16 volumeSize;
    i16 divNumber;
    u8 rateStep;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_BEM1, emitFlags, volumeType, emitterScl, emitterRot, emitterTrs, emitterDir, initialVelOmni, initialVelAxis, initialVelRndm, initialVelDir,  spread, initialVelRatio, rate, rateRndm, lifeTimeRndm, volumeSweep, volumeMinRad, airResist, moment, momentRndm, maxFrame, startFrame, lifeTime, volumeSize, divNumber, rateStep);

// https://github.com/magcius/noclip.website/blob/5644a8b71f7a8ca28cbb2e027069c4ea85ca211d/src/Common/JSYSTEM/JPA.ts#L4514
// JPA Base Shape 
// Particle Draw Settings
struct JPA_BSP1 {
    // u32 flags; // Controls some commented out options TODO
    u8 shapeType;
    u8 dirType;
    u8 rotType;
    u8 planeType;
    vector<f32> baseSize;
    f32 tilingS;
    f32 tilingT;
    bool isDrawFwdAhead;
    bool isDrawPrntAhead;
    bool isNoDrawParent;
    bool isNoDrawChild;

    // TEV/PE Settings
    u8 colorInSelect;
    u8 alphaInSelect;
    u16 blendModeFlags;
    u8 alphaCompareFlags;
    u8 alphaRef0;
    u8 alphaRef1;
    u8 zModeFlags;

    u8 anmRndm;

    // Texture Palette Animation
    // bool isEnableTexture;
    bool isGlblTexAnm;
    u8 texCalcIdxType;
    u8 texIdx;
    vector<u8> texIdxAnimData;
    u8 texIdxLoopOfstMask;

    // Texture Coordinate Animation
    bool isEnableProjection;
    bool isEnableTexScrollAnm;

    f32 texInitTransX;
    f32 texInitTransY;
    f32 texInitScaleX;
    f32 texInitScaleY;
    f32 texInitRot;
    f32 texIncTransX;
    f32 texIncTransY;
    f32 texIncScaleX;
    f32 texIncScaleY;
    f32 texIncRot;

    // Color Animation Settings
    bool isGlblClrAnm;
    u8 colorCalcIdxType;
    Color colorPrm;
    Color colorEnv;
    vector<pair<u16, Color>> colorPrmAnimData;
    vector<pair<u16, Color>> colorEnvAnimData;
    u16 colorAnimMaxFrm;
    u8 colorLoopOfstMask;

};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_BSP1, shapeType, dirType, rotType, planeType, baseSize, tilingS, tilingT, isDrawFwdAhead, isDrawPrntAhead, isNoDrawParent, isNoDrawChild, colorInSelect, alphaInSelect, blendModeFlags, alphaCompareFlags, alphaRef0, alphaRef1, zModeFlags, anmRndm, isGlblTexAnm, texCalcIdxType, texIdx, texIdxAnimData, texIdxLoopOfstMask, isEnableProjection, isEnableTexScrollAnm, texInitTransX, texInitTransY, texInitScaleX, texInitScaleY, texInitRot, texIncTransX, texIncTransY, texIncScaleX, texIncScaleY, texIncRot, isGlblClrAnm, colorCalcIdxType, colorPrm, colorEnv, colorPrmAnimData, colorEnvAnimData, colorAnimMaxFrm, colorLoopOfstMask);

struct JPA_ESP1 {

};
struct JPA_ETX1 {

};
struct JPA_SSP1 {

};
struct JPA_FLD1 {

};
struct JPA_KFA1 {

};
struct JPA_TDB1 {

};

struct JPA_ResourceRaw {
    u16 resourceId;
    DataBuffer data; 
};

struct JPA_Textures {
    string name;
    DataBuffer data;
};

struct JPA_Resource {
    u16 resourceId;
    u16 blockCount;
    u8 fieldBlockCount;
    u8 keyBlockCount;
    u8 tdb1Count;

    vector<JPA_BEM1> bem1; // Dynamics Block
    vector<JPA_BSP1> bsp1; // Base Shape Block
    vector<u8> esp1; // Extra Shape Block
    vector<u8> etx1; // Extra Texture Block
    vector<u8> ssp1; // Child Shape Block
    vector<vector<u8>> fld1; // Field Blocks
    vector<vector<u8>> kfa1; // Key Blocks
    vector<u16> tdb1; // Texture Id to Texture Map

};
struct JPAC {
    string version;
    vector<JPA_Resource> resources;
    vector<JPA_Textures> textures;
};


#endif