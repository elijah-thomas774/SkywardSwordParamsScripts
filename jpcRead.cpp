#include <fstream>
#include <iostream>
#include <vector>
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


// const enum BLOCK_MAGIC{
//     BEM1 = 0x42454D3100,
//     BSP1 =,
//     ESP1 =,
//     ETX1 =,
//     SSP1 =,
//     FLD1 =,
//     KFA1 =,
//     TDB1 =,
//     TEX1 =,
// };
struct BTI_header {
    u8 format;
    u8 enAlpha;
    u16 width;
    u16 height;

};

struct JPAResourceRaw {
    u16 resourceId;
    data_t data;
    u16 texIdBase;
};

int mem_size;

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

void readFileStuff(const u8* jpcMem)
{
    string version = readString(jpcMem, 0, 8);
    printf("Versopn: %s\n", version.c_str());

    u16 effect_count = readUint16(jpcMem, 0x8);
    printf("effect count: %d(0x%X)\n", effect_count, effect_count);

    u16 texture_count = readUint16(jpcMem, 0xA);
    printf("texture count: %d(0x%X)\n", texture_count, texture_count);

    u32 texture_table_offset = readUint32(jpcMem, 0xC);
    printf("texture count offset: %d (0x%X)\n", texture_table_offset, texture_table_offset);

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
        jpaResources.push_back({resourceId, data, 0});

    }
    // int idx = 0;
    // for (auto &jpa : jpaResources)
    // {
    //     u8 name[4] = { jpa.data.at(8), jpa.data.at(9), jpa.data.at(10), jpa.data.at(11) };
    //     printf("effect ID: %4d | resource ID: %4d | name: %4s |\n", idx++, jpa.resourceId, name);
    // }


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
        cout << texture_name << endl;
        ofstream tOut;
        tOut.open("./output/" + texture_name + ".bti", ios::binary | ios::out);
        tOut.write(texture_data, blocksize-0x20);
        tOut.close();
        delete[] texture_data;
        texture_table_index += blocksize;
    }
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