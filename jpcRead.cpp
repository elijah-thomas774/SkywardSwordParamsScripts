#include <fstream>
#include <iostream>
#include <vector>
#include "structures.h"
using namespace std;

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
typedef vector<u8> data_t;

int mem_size;
vector<pair<string, int>> textures;

string readString(const u8* mem, int start, int n)
{
    string out(n, '\0');
    if (start+n >= mem_size){
        printf("Error: array out of bounds\n");
        return "";    
    }    
    for(int i = 0; i < n; i++)
    {
        out[i] = mem[start++];
    }
    return out;
}
string readString(const u8* mem, int start, int n, bool nullTermination)
{
    string out(n, '\0');
    if (start+n >= mem_size){
        printf("Error: array out of bounds\n");
        return "";    
    }    
    for(int i = 0; i < n; i++)
    {
        out[i] = mem[start++];
        if (out[i] == '\0')
        {
            break;
        }
    }
    return out;
}
u8 readUint8(const u8* mem, int offset)
{
    if (offset >= mem_size)
    {
        printf("Array out of Bounds");
        return 0;
    }
    return mem[offset];
}
u16 readUint16(const u8* mem, int offset)
{   
    if (offset+1 >= mem_size)
    {
        printf("Array out of Bounds\n");
        return 0;
    }
    u16 temp1 = mem[offset];
    u16 temp2 = mem[offset+1];
    return (temp1<<8) + temp2;
}
u32 readUint32(const u8* mem, int offset)
{
    if (offset+3 >= mem_size)
    {
        printf("Array out of Bounds\n");
        return 0;
    }
    u32 t1 = mem[offset];
    u32 t2 = mem[offset+1];
    u32 t3 = mem[offset+2];
    u32 t4 = mem[offset+3];
    return (t1 << 24) | (t2 << 16) | (t3 << 8) | (t4);
}
f32 readFloat(const u8* mem, int offset)
{
    if (offset+3 >= mem_size)
    {
        printf("Array out of Bounds\n");
        return 0;
    }
    u32 t1 = mem[offset];
    u32 t2 = mem[offset+1];
    u32 t3 = mem[offset+2];
    u32 t4 = mem[offset+3];
    u32 combined = ((t1 << 24) | (t2 << 16) | (t3 << 8) | (t4));
    return *(f32*)&combined;
}
string readString(const data_t& mem, int start, int n)
{
    string out(n, '\0');
    if (start+n >= mem.size()){
        printf("Error: array out of bounds\n");
        return "";    
    }    
    for(int i = 0; i < n; i++)
    {
        out[i] = mem[start++];
    }
    return out;
}
string readString(const data_t& mem, int start, int n, bool nullTermination)
{
    string out(n, '\0');
    if (start+n >= mem.size()){
        printf("Error: array out of bounds\n");
        return "";    
    }    
    for(int i = 0; i < n; i++)
    {
        out[i] = mem[start++];
        if (out[i] == '\0')
        {
            break;
        }
    }
    return out;
}
u8 readUint8(const data_t& mem, int offset)
{
    if (offset >= mem.size())
    {
        printf("Array out of Bounds");
        return 0;
    }
    return mem[offset];
}
u16 readUint16(const data_t &mem, int offset)
{   
    if (offset+1 >= mem.size())
    {
        printf("Array out of Bounds\n");
        return 0;
    }
    u16 temp1 = mem[offset];
    u16 temp2 = mem[offset+1];
    return (temp1<<8) + temp2;
}
u32 readUint32(const data_t &mem, int offset)
{
    if (offset+3 >= mem.size())
    {
        printf("Array out of Bounds\n");
        return 0;
    }
    u32 t1 = mem[offset];
    u32 t2 = mem[offset+1];
    u32 t3 = mem[offset+2];
    u32 t4 = mem[offset+3];
    return (t1 << 24) | (t2 << 16) | (t3 << 8) | (t4);
}
f32 readFloat(const data_t& mem, int offset)
{
    if (offset+3 >= mem.size())
    {
        printf("Array out of Bounds\n");
        return 0;
    }
    u32 t1 = mem[offset];
    u32 t2 = mem[offset+1];
    u32 t3 = mem[offset+2];
    u32 t4 = mem[offset+3];
    u32 combined = ((t1 << 24) | (t2 << 16) | (t3 << 8) | (t4));
    return *(f32*)&combined;
}

Color toColor(u32 number) { return {(number>>24)&0xFF, (number>>16)&0xFF, (number>>8)&0xFF, (number)&0xFF}; }

vector<Color> createColorTable(vector<u8>& mem, u32 offset, u32 count, u32 duration)
{ 
    vector<Color> dst(duration+1,{0,0,0,0});
    i32 dstIdx = 0;

    dst[dstIdx++] = toColor(readUint32(mem, offset + 0x02));
    
    u32 time0 = readUint16(mem, offset + 0x00);
    for (int i = 1; i <= std::min(time0, duration);i++)
    {
        dst[dstIdx++] = {dst[0].r, dst[0].g, dst[0].b, dst[0].a};
    }

    u32 time1 = time0;
    for (int i = 1; i < count; i++)
    {
        int entry0 = i-1;
        int entry1 = i;
        time0 = readUint16(mem, entry0*0x06 + 0x00 + offset);
        time1 = readUint16(mem, entry1*0x06 + 0x00 + offset);
    }
    return dst;
}
void readFileStuff(const u8* jpcMem)
{
    string version = readString(jpcMem, 0, 8);
    std::printf("Version: %s\n", version.c_str());

    u16 effect_count = readUint16(jpcMem, 0x8);
    std::printf("effect count: %d(0x%X)\n", effect_count, effect_count);

    u16 texture_count = readUint16(jpcMem, 0xA);
    std::printf("texture count: %d(0x%X)\n", texture_count, texture_count);

    u32 texture_table_offset = readUint32(jpcMem, 0xC);
    std::printf("texture count offset: %d (0x%X)\n", texture_table_offset, texture_table_offset);

    // JPA Resources
    vector<JPAResourceRaw> jpaResources;
    u32 effect_table_index = 0x10;
    for (int i = 0; i < effect_count; i++)
    {
        u32 begin = effect_table_index;
        u16 resourceId = readUint16(jpcMem, effect_table_index + 0x00);
        u16 blockCount = readUint16(jpcMem, effect_table_index + 0x02);
        string name    = readString(jpcMem, effect_table_index + 0x8, 4);
        effect_table_index += 0x08;
        for(int j = 0; j < blockCount; j++)
        {
            u32 block_size = readUint32(jpcMem, effect_table_index + 0x04);
            effect_table_index += block_size;
        }
        data_t data;
        for (int j = begin; j < effect_table_index; j++)
        {
            data.push_back(jpcMem[j]);
        }
        jpaResources.push_back({resourceId, vector<u8>(data), begin, 0});
    }

    // Textures
    u32 texture_table_index = texture_table_offset;
    for (int i = 0; i < texture_count; i++)
    {
        u32 blocksize = readUint32(jpcMem, texture_table_index + 0x04);
        string texture_name = readString(jpcMem, texture_table_index+0xC, 0x14, true);
        char* texture_data = new char[blocksize-0x20];
        for(int j = 0x20; j < blocksize; j++)
        {
            texture_data[j-0x20] = jpcMem[texture_table_index+j];
        }
        // // The following Code Writes the data to an output folder 
        // ofstream tOut;
        // tOut.open("./output/" + texture_name + ".bti", ios::binary | ios::out);
        // tOut.write(texture_data, blocksize-0x20);
        // tOut.close();
        // printf("Texture format: 0x%X %20s (%s)\n", (u8)texture_data[0], texture_name.c_str(), TEX_FORMATS[texture_data[0]].c_str());
        delete[] texture_data;
        textures.push_back({texture_name, texture_table_index});
        texture_table_index += blocksize;
    }
    
    // Resource Parsing

    // Below were used to make sure it read all of them in the file : it does :)
    u32 bem1Counter = 0;
    u32 bsp1Counter = 0;
    u32 esp1Counter = 0;
    u32 ssp1Counter = 0;
    u32 etx1Counter = 0;
    u32 kfa1Counter = 0;
    u32 fld1Counter = 0;
    u32 tdb1Counter = 0;
    u32 unkCounter = 0;
    ofstream tempFile;
    tempFile.open("temp.txt");
    for ( JPAResourceRaw &res : jpaResources)
    {
        vector<u8> dat = res.data;
        u32 blockCount = readUint16(dat, 0x02);
        u32 fieldBlockCount = readUint8(dat, 0x4);
        u32 fieldKeyBlockCount = readUint8(dat, 0x5);
        u32 tdb1Count = readUint8(dat, 0x06);

        JPADynamicsBlock bem1;
        JPABaseShapeBlock bsp1;
        JPAExtraShapeBlock esp1;
        JPAExTexBlock etx1;
        JPAChildShapeBlock ssp1;
        vector<JPAFieldBlock> fld1;
        vector<JPAKeyBlock> kfa1;
        vector<u16> tdb1;

        u32 tableIndex = 0x08;

        for (int j = 0; j < blockCount; j++)
        {
            i32 magic = readUint32(dat, tableIndex+0x00);
            u32 blockSize = readUint32(dat, tableIndex+0x04);
            //printf("0x8%X\n", magic);
            if (magic == 'BEM1')
            {
                //printf("Read BEM1\n");
                bem1Counter++;
            }
            else if (magic == 'BSP1') // This is what i want to focus on
            {
                u32 flags = readUint32(dat, tableIndex+0x08);
                ShapeType shapetype = static_cast<ShapeType>((flags>>0 ) & 0xFF);
                DirType dirtype     = static_cast<DirType  >((flags>>4 ) & 0x03);
                RotType rottype     = static_cast<RotType  >((flags>>7 ) & 0x03);
                PlaneType planetype = static_cast<PlaneType>((flags>>10) & 0x01);
                f32 tilingS = ((flags>>0x19) & 0x01) ? 2.0 : 1.0;
                f32 tilingT = ((flags>>0x1A) & 0x01) ? 2.0 : 1.0;
                bool isNoDrawParent = (flags & 0x08000000);
                bool isNoDrawChild  = (flags & 0x10000000);
                u32 colorInSelect = (flags >> 0x0F) & 0x07;
                u32 alphaInSelect = (flags >> 0x12) & 0x01;
                bool isEnableTexScrollAnm = (flags & 0x01000000);
                bool isDrawFwdAhead       = (flags & 0x00200000);
                bool isDrawPrntAhead      = (flags & 0x00400000);
                bool isEnableProjection   = (flags & 0x00100000);
                bool isGlblTexAnm         = (flags & 0x00004000);
                bool isGlblClrAnm         = (flags & 0x00001000);
                f32 baseSizeX = readFloat(dat, tableIndex + 0x10);
                f32 baseSizeY = readFloat(dat, tableIndex + 0x14);
                f32 baseSize[2] = {baseSizeX, baseSizeY};
                u16 blendModeFlags = readUint16(dat, tableIndex + 0x18);
                u8 alphaCompareFlags = readUint8(dat, tableIndex + 0x1A);
                u8 alphaRef0 = readUint8(dat, tableIndex + 0x1B);
                u8 alphaRef1 = readUint8(dat, tableIndex + 0x1C);
                u8 zModeFlags = readUint8(dat, tableIndex + 0x1D);
                u8 texFlags = readUint8(dat, tableIndex + 0x1E);
                u8 texIdxAnimCount = readUint8(dat, tableIndex + 0x1F);
                u8 texIdx = readUint8(dat, tableIndex + 0x20);
                u8 colorFlags = readUint8(dat, tableIndex + 0x21);

                Color colorPrm = toColor(readUint32(dat, tableIndex + 0x26));
                Color colorEnv = toColor(readUint32(dat, tableIndex + 0x2A));

                CalcIdxType texCalcIdxType = static_cast<CalcIdxType>((texFlags >> 2) & 0x07);

                u8 anmRndm = readUint8(dat, tableIndex + 0x2E);
                u8 colorLoopOfstMask = readUint8(dat, tableIndex + 0x2F);
                u8 texIdxLoopOfstMask = readUint8(dat, tableIndex + 0x30);

                u32 extraDataOffs = tableIndex + 0x34;

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

                if ((flags & 0x01000000)) {
                    texInitTransX = readFloat(dat, extraDataOffs + 0x00);
                    texInitTransY = readFloat(dat, extraDataOffs + 0x04);
                    texInitScaleX = readFloat(dat, extraDataOffs + 0x08);
                    texInitScaleY = readFloat(dat, extraDataOffs + 0x0C);
                    texInitRot = readFloat(dat, extraDataOffs + 0x10);
                    texIncTransX = readFloat(dat, extraDataOffs + 0x14);
                    texIncTransY = readFloat(dat, extraDataOffs + 0x18);
                    texIncScaleX = readFloat(dat, extraDataOffs + 0x1C);
                    texIncScaleY = readFloat(dat, extraDataOffs + 0x20);
                    texIncRot = readFloat(dat, extraDataOffs + 0x24);
                    extraDataOffs += 0x28;
                }

                vector<u8> texIdxAnimData;

                bool isEnableTextureAnm = (texFlags & 0x00000001);
                if (isEnableTextureAnm)
                {
                    for (int a = 0; a < texIdxAnimCount; a++)
                    {
                        texIdxAnimData.push_back(dat[a+extraDataOffs]);
                    }
                }
                if (!texIdxAnimData.empty())
                {
                    std::printf("Res ID: 0x%04X\n", res.resourceId);
                }
                u16 colorAnimMaxFrm = readUint16(dat, tableIndex + 0x24);
                vector<Color> colorPrmAnimData;
                u32 colorPrmAnimDataOffs = 0;
                if ((colorFlags & 0x02)) {
                    colorPrmAnimDataOffs = tableIndex + readUint16(dat, tableIndex + 0x0C);
                    u8 colorPrmAnimDataCount = readUint8(dat, tableIndex + 0x22);
                    colorPrmAnimData = createColorTable(dat, colorPrmAnimDataOffs, colorPrmAnimDataCount, colorAnimMaxFrm);
                }

                vector<Color> colorEnvAnimData;
                u32 colorEnvAnimDataOffs = 0;
                if ((colorFlags & 0x08)) {
                    colorEnvAnimDataOffs = tableIndex + readUint16(dat, tableIndex + 0x0E);
                    u8 colorEnvAnimDataCount = readUint8(dat, tableIndex + 0x23);
                    colorEnvAnimData = createColorTable(dat, colorEnvAnimDataOffs, colorEnvAnimDataCount, colorAnimMaxFrm);
                }
                // char buff[103];
                // std::sprintf(buff, "Resource ID: %4X, Tex: %20s (0x%8X), colorPrm: 0x%8X, colorEnc: 0x%8X\n",res.resourceId, textures.at(texIdx).first.c_str(), textures.at(texIdx).second , colorPrmAnimDataOffs+res.data_offset, colorEnvAnimDataOffs+res.data_offset);
                // tempFile.write(buff, 102);
                CalcIdxType colorCalcIdxType = static_cast<CalcIdxType>((colorFlags >> 4) & 0x07);

                bool isEnableTexture = true;

                
                bsp1Counter++;
            }
            else if (magic == 'ESP1')
            {
                esp1Counter++;
            }
            else if (magic == 'SSP1')
            {
                ssp1Counter++;
            }
            else if (magic == 'ETX1')
            {
                etx1Counter++;
            }
            else if (magic == 'KFA1')
            {
                kfa1Counter++;
            }
            else if (magic == 'FLD1')
            {
                fld1Counter++;
            }
            else if (magic == 'TDB1')
            {
                tdb1Counter++;
            }
            else {
                unkCounter++;
            }
            tableIndex+=blockSize;
        }
    }        
        std::printf("BEM1 Count: %d\n", bem1Counter);
        std::printf("BSP1 Count: %d\n", bsp1Counter);
        std::printf("ESP1 Count: %d\n", esp1Counter);
        std::printf("SSP1 Count: %d\n", ssp1Counter);
        std::printf("ETX1 Count: %d\n", etx1Counter);
        std::printf("KFA1 Count: %d\n", kfa1Counter);
        std::printf("FLD1 Count: %d\n", fld1Counter);
        std::printf("TDB1 Count: %d\n", tdb1Counter);
        std::printf("UNK0 Count: %d\n", unkCounter);
        tempFile.close();
    return;
}

int main()
{
    streampos size;
    u8* jpcMem;
    // I am using a binary read template from cplusplus.com
    ifstream jpcFile("./Common.jpc", ios::binary | ios::in | ios::ate);
    if(jpcFile.is_open())
    {
        size = jpcFile.tellg();
        jpcMem = new u8[size]; // allocs enough mem for the file
        jpcFile.seekg(0, ios::beg);
        jpcFile.read((char *)jpcMem, size);
        jpcFile.close(); // at this point, the file should be completely in mem
        mem_size = size;
        readFileStuff(jpcMem);
        delete[] jpcMem; // deallocs mem
    } else {
        cout << "Could not locate file" << endl;
    }

}