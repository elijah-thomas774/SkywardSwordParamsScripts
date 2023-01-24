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
typedef vector<f32> vec2f;

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
    return (i8)(data.at(offset) & 0xFF);
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
    f32 y = read_f32(offset+4);
    f32 z = read_f32(offset+8);

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
    vector<f32> vecData;
    if (checkOffset(offset, n*sizeof(f32)))
        return vecData;
    
    for (int i = 0; i < n; i++)
    {
        vecData.push_back(read_f32(i*sizeof(f32) + offset));
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
    u32 unkFlags;
    u8 volumeType;
    vec3f emitterScl;
    vector<i16> emitterRot; // in degrees
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
    f32 momentRndm;
    i16 maxFrame;
    i16 startFrame;
    i16 lifeTime;
    i16 volumeSize;
    i16 divNumber;
    u8 rateStep;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_BEM1, emitFlags, unkFlags, volumeType, emitterScl, emitterRot, emitterTrs, emitterDir, initialVelOmni, initialVelAxis, initialVelRndm, initialVelDir,  spread, initialVelRatio, rate, rateRndm, lifeTimeRndm, volumeSweep, volumeMinRad, airResist, momentRndm, maxFrame, startFrame, lifeTime, volumeSize, divNumber, rateStep);

// https://github.com/magcius/noclip.website/blob/5644a8b71f7a8ca28cbb2e027069c4ea85ca211d/src/Common/JSYSTEM/JPA.ts#L4514
// JPA Base Shape 
// Particle Draw Settings
struct JPA_BSP1 {
    // u32 flags; // Controls some commented out options TODO
    u32 origFlags;
    u8 texFlags;
    u8 colorFlags;
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
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_BSP1, origFlags, texFlags, colorFlags, shapeType, dirType, rotType, planeType, baseSize, tilingS, tilingT, isDrawFwdAhead, isDrawPrntAhead, isNoDrawParent, isNoDrawChild, colorInSelect, alphaInSelect, blendModeFlags, alphaCompareFlags, alphaRef0, alphaRef1, zModeFlags, anmRndm, isGlblTexAnm, texCalcIdxType, texIdx, texIdxAnimData, texIdxLoopOfstMask, isEnableProjection, isEnableTexScrollAnm, texInitTransX, texInitTransY, texInitScaleX, texInitScaleY, texInitRot, texIncTransX, texIncTransY, texIncScaleX, texIncScaleY, texIncRot, isGlblClrAnm, colorCalcIdxType, colorPrm, colorEnv, colorPrmAnimData, colorEnvAnimData, colorAnimMaxFrm, colorLoopOfstMask);

// Extra Shape Block
struct JPA_ESP1 {
    u32 origFlags;
    bool isEnableScale;
    bool isDiffXY;
    u8 scaleAnmTypeX;
    u8 scaleAnmTypeY;
    bool isEnableRotate;
    bool isEnableAlpha;
    bool isEnableSinWave;
    u8 pivotX;
    u8 pivotY;
    f32 scaleInTiming;
    f32 scaleOutTiming;
    f32 scaleInValueX;
    f32 scaleOutValueX;
    f32 scaleInValueY;
    f32 scaleOutValueY;
    f32 scaleOutRandom;
    u16 scaleAnmMaxFrameX;
    u16 scaleAnmMaxFrameY;
    f32 alphaInTiming;
    f32 alphaOutTiming;
    f32 alphaInValue;
    f32 alphaBaseValue;
    f32 alphaOutValue;
    f32 alphaWaveRandom;
    f32 alphaWaveFrequency;
    f32 alphaWaveAmplitude;
    f32 rotateAngle;
    f32 rotateAngleRandom;
    f32 rotateSpeed;
    f32 rotateSpeedRandom;
    f32 rotateDirection;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_ESP1, origFlags, isEnableScale,isDiffXY,scaleAnmTypeX,scaleAnmTypeY,isEnableRotate, isEnableSinWave, isEnableAlpha,pivotX,pivotY,scaleInTiming,scaleOutTiming,scaleInValueX,scaleOutValueX,scaleInValueY,scaleOutValueY,scaleOutRandom,scaleAnmMaxFrameX,scaleAnmMaxFrameY,alphaInTiming,alphaOutTiming,alphaInValue,alphaBaseValue,alphaOutValue,alphaWaveRandom,alphaWaveFrequency,alphaWaveAmplitude,rotateAngle,rotateAngleRandom,rotateSpeed,rotateSpeedRandom,rotateDirection);

// Extra Texture Block
struct JPA_ETX1 {
    u8 indTextureMode;
    vector<f32> floats;
    u8 indTextureID;
    u8 unk;
    i8 scale;
    u8 secondTextureID;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_ETX1, indTextureMode, floats, indTextureID, scale, unk, secondTextureID);
// Child Shape Block
struct JPA_SSP1 {
    u32 origFlags;
    bool isInheritedScale;
    bool isInheritedRGB;
    bool isInheritedAlpha;
    bool isEnableAlphaOut;
    bool isEnableField;
    bool isEnableRotate;
    bool isEnableScaleOut;
    u8 shapeType;
    u8 dirType;
    u8 rotType;
    u8 planeType;
    f32 posRndm;
    f32 baseVel;
    f32 baseVelRndm;
    f32 velInfRate;
    f32 gravity;
    vec2f globalScale2D;
    f32 inheritScale;
    f32 inheritAlpha;
    f32 inheritRGB;
    Color colorPrm;
    Color colorEnv;
    f32 timing;
    u16 life;
    u16 rate;
    u8 step;
    u8 texIdx;
    u16 rotateSpeed;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_SSP1, origFlags, isInheritedScale, isInheritedRGB, isInheritedAlpha, isEnableAlphaOut, isEnableField, isEnableRotate, isEnableScaleOut, shapeType, dirType, rotType, planeType, posRndm, baseVel, baseVelRndm, velInfRate, gravity, globalScale2D, inheritScale, inheritAlpha, inheritRGB, colorPrm, colorEnv, timing, life, rate, step, texIdx, rotateSpeed);
// Field Block
struct JPA_FLD1 {
    u32 origFlags;
    u16 sttFlag;
    u8 type;
    u8 addType;
    vec3f pos;
    vec3f dis;
    f32 param1;
    f32 param2;
    f32 param3;
    f32 fadeIn;
    f32 fadeOut;
    f32 enTime;
    f32 disTime;
    u8 cycle;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_FLD1, origFlags, sttFlag, type, addType, pos, dis, param1, param2, param3, fadeIn, fadeOut, enTime, disTime, cycle);
// Key Block
struct JPA_KFA1 {
    u8 keyType;
    u8 keyCount;
    u8 unk0xA;
    vector<f32> keyValues;
    bool isLoopEnable;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_KFA1, keyType, unk0xA, keyCount, keyValues, isLoopEnable);
// struct JPA_TDB1 { // just a vector of u16 indecies

// };

struct JPA_ResourceRaw {
    u16 resourceId;
    DataBuffer data; 
};

struct JPA_Textures {
    string name;
    DataBuffer data;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_Textures, name);
struct JPA_Resource {
    u16 resourceId;
    u16 blockCount;
    u8 fieldBlockCount;
    u8 keyBlockCount;
    u8 tdb1Count;

    vector<JPA_BEM1> bem1; // Dynamics Block
    vector<JPA_BSP1> bsp1; // Base Shape Block
    vector<JPA_ESP1> esp1; // Extra Shape Block
    vector<JPA_ETX1> etx1; // Extra Texture Block
    vector<JPA_SSP1> ssp1; // Child Shape Block
    vector<JPA_FLD1> fld1; // Field Blocks
    vector<JPA_KFA1> kfa1; // Key Blocks
    vector<u16> tdb1; // Texture Id to Texture Map
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPA_Resource, \
resourceId, blockCount, fieldBlockCount, keyBlockCount, tdb1Count, \
bem1, bsp1, esp1, etx1, ssp1, fld1, kfa1, tdb1);
struct JPAC {
    string version;
    vector<JPA_Resource> resources;
    vector<JPA_Textures> textures;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JPAC, \
version, \
resources, textures);

#endif