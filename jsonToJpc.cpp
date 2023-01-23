#include "structures.h"

// Goal to convert a json to a jpc format. This will support adding individual blocks.
// Not adding at the moment, just in place changing

// Function Templates
class Buffer
{
    public:
        Buffer() : buffer() {}
        void add_f32(f32 add);
        void add_u32(u32 add);
        void add_i32(i32 add);
        void add_u16(u16 add);
        void add_i16(i16 add);
        void add_u8 (u8  add);
        void add_i8 (i8  add);
        void add_vec3f(vec3f add);
        void add_padding(u32 n);
        void add_color(Color add);
        void add_u8_arr(vector<u8> &add);
        void add_u16_arr(vector<u16> &add);
        void add_u32_arr(vector<u32> &add);
        void add_i8_arr(vector<i8> &add);
        void add_i16_arr(vector<i16> &add);
        void add_i32_arr(vector<i32> &add);
        void add_f32_arr(vector<f32> &add);
        void add_color_arr(vector<Color> &add);
        vector<u8> buffer;
};

vector<u8> build_bem1(JPA_BEM1&);
vector<u8> build_bsp1(JPA_BSP1&);
vector<u8> build_esp1(JPA_ESP1&);
vector<u8> build_ssp1(JPA_SSP1&);
vector<u8> build_etx1(JPA_ETX1&);
vector<u8> build_fld1(JPA_FLD1&);
vector<u8> build_kfa1(JPA_KFA1&);
vector<u8> build_tbd1(vector<u16>&);

vector<u8> build_resource(JPA_Resource&);
vector<u8> build_jpc(JPAC&);


void Buffer::add_f32(f32 add){
    u32 temp = *(u32*)&add;
    add_u32(temp);
}
void Buffer::add_u32(u32 add){
    buffer.push_back((add >> 24) & 0xFF);
    buffer.push_back((add >> 16) & 0xFF);
    buffer.push_back((add >>  8) & 0xFF);
    buffer.push_back((add >>  0) & 0xFF);
}
void Buffer::add_i32(i32 add){
    add_u32((add&0xFFFFFFFF));
}
void Buffer::add_u16(u16 add){
    buffer.push_back((add >>  8) & 0xFF);
    buffer.push_back((add >>  0) & 0xFF);
}
void Buffer::add_i16(i16 add){
    add_u16((add&0xFFFF));
}
void Buffer::add_u8 (u8  add){
    buffer.push_back(add);
}
void Buffer::add_i8 (i8  add){
    buffer.push_back((add&0xFF));
}
void Buffer::add_vec3f(vec3f add){
    for(f32 temp : add)
    {
        add_f32(temp);
    }
}
void Buffer::add_padding(u32 n){
    for (int i = 0; i < n; i++)
    {
        buffer.push_back(0);
    }
}
void Buffer::add_color(Color add){
    add_u8(add.r);
    add_u8(add.g);
    add_u8(add.b);
    add_u8(add.a);
}
void Buffer::add_u8_arr(vector<u8> &add){
    for (u8 temp : add)
    {
        add_u8(temp);
    }
}
void Buffer::add_u16_arr(vector<u16> &add){
    for (u16 temp : add)
    {
        add_u16(temp);
    }
    return;
}
void Buffer::add_u32_arr(vector<u32> &add){
    for (u32 temp : add)
    {
        add_u32(temp);
    }
    return;
}
void Buffer::add_i8_arr(vector<i8> &add){
    for (i8 temp : add)
    {
        add_i8(temp);
    }
    return;
}
void Buffer::add_i16_arr(vector<i16> &add){
    for (i16 temp : add)
    {
        add_i16(temp);
    }
    return;
}
void Buffer::add_i32_arr(vector<i32> &add){
    for (i32 temp : add)
    {
        add_i32(temp);
    }
    return;
}
void Buffer::add_f32_arr(vector<f32> &add){
    for (f32 temp : add)
    {
        add_f32(temp);
    }
    return;
}
void Buffer::add_color_arr(vector<Color> &add){
    for (Color temp : add)
    {
        add_color(temp);
    }
    return;
}

vector<u8> build_bem1(JPA_BEM1 &bem1)
{
    Buffer block;
    block.add_u32((u32)'BEM1');
    block.add_u32(0x7C); // size
    block.add_u32(bem1.emitFlags); // TODO: Build Emit flags with volume type and orig Data
    block.add_u32(bem1.unkFlags);
    block.add_vec3f(bem1.emitterScl);
    block.add_vec3f(bem1.emitterTrs);
    block.add_vec3f(bem1.emitterDir);
    block.add_f32(bem1.initialVelOmni);
    block.add_f32(bem1.initialVelAxis);
    block.add_f32(bem1.initialVelRndm);
    block.add_f32(bem1.initialVelDir);
    block.add_f32(bem1.spread);
    block.add_f32(bem1.initialVelRatio);
    block.add_f32(bem1.rate);
    block.add_f32(bem1.rateRndm);
    block.add_f32(bem1.lifeTimeRndm);
    block.add_f32(bem1.volumeSweep);
    block.add_f32(bem1.volumeMinRad);
    block.add_f32(bem1.airResist);
    block.add_f32(bem1.momentRndm);
    block.add_i16_arr(bem1.emitterRot); // TODO, add conversion for deg to rad
    block.add_i16(bem1.maxFrame);
    block.add_i16(bem1.startFrame);
    block.add_i16(bem1.lifeTime);
    block.add_i16(bem1.volumeSize);
    block.add_i16(bem1.divNumber);
    block.add_u8(bem1.rateStep);
    block.add_padding(3);
    assert(0x7C == block.buffer.size());
    return block.buffer;
}
vector<u8> build_bsp1_JPAC_11(JPA_BSP1& bsp1)
{
    Buffer block;
    block.add_u32((u32)'BSP1');
    u32 size; // This is a doozy xD
    // size = 0x34 + extra
    // extra = (extra floats) + (texAnmId arr ceil(4)) + (color AnmStuff)
    // extra floats  = 0x28 if flag 0x01000000 is set (0 otherwise)
    // texAnmId arr - count bytes, ceiling to multiple of 4  (and pad)
    // Color Prm count (multiple of 6) - need offset and such (run .size() at point)
    // Color Env count (multiple of 6) - need offset and such (run .size() at point)
    // TODO: Restore original flags too
    // Which is 0x00180000 (just add in unk Flags Lmao)
    size = 0x34;
    if (bsp1.isEnableTexScrollAnm) size += 0x28;
    u32 texAnmSize = bsp1.texIdxAnimData.size();
    if (texAnmSize%4 != 0) texAnmSize += (4 - texAnmSize%4);
    size += texAnmSize;
    size += 6*bsp1.colorEnvAnimData.size();
    if (size%4 != 0)
    {
        size += (4 - size%4);
    }
    size += 6*bsp1.colorPrmAnimData.size();
    if (size%4 != 0)
    {
        size += (4 - size%4);
    }
    block.add_u32(size);
    u32 flags = 0;
    flags |= ((bsp1.shapeType              & 0xF) << 0);
    flags |= ((bsp1.dirType                & 0x7) << 4);
    flags |= ((bsp1.rotType                & 0x7) << 7);
    flags |= ((bsp1.planeType              & 0x1) << 10);
    flags |= ((bsp1.tilingS == 2.0       ? 1 : 0) << 0x1B);
    flags |= ((bsp1.tilingT == 2.0       ? 1 : 0) << 0x1C);
    flags |= ((bsp1.origFlags & 0x00180000)); // These are the unk flags
    flags |= ((bsp1.isNoDrawParent       ? 1 : 0) << 29);
    flags |= ((bsp1.isNoDrawChild        ? 1 : 0) << 30);
    flags |= ((bsp1.colorInSelect          & 0x7) << 0xF);
    flags |= ((bsp1.alphaInSelect          & 0x1) << 0x12);
    flags |= ((bsp1.isEnableTexScrollAnm ? 1 : 0) << 26);
    flags |= ((bsp1.isDrawFwdAhead       ? 1 : 0) << 23);
    flags |= ((bsp1.isDrawPrntAhead      ? 1 : 0) << 24);
    flags |= ((bsp1.isEnableProjection   ? 1 : 0) << 22);
    flags |= ((bsp1.isGlblTexAnm         ? 1 : 0) << 14);
    flags |= ((bsp1.isGlblClrAnm         ? 1 : 0) << 12);
    flags |= ((bsp1.origFlags & 0x02000000));
    block.add_u32(flags);
    block.add_u32(0x00); // To be filled later
    block.add_f32_arr(bsp1.baseSize);
    block.add_u16(bsp1.blendModeFlags);
    block.add_u8(bsp1.alphaCompareFlags);
    block.add_u8(bsp1.alphaRef0);
    block.add_u8(bsp1.alphaRef1);
    block.add_u8(bsp1.zModeFlags);
    u8 texFlags = 0; // TODO: Check texFlags
    texFlags |= ((bsp1.texCalcIdxType & 0x7) << 2);
    texFlags |= ((bsp1.texIdxAnimData.size()==0 ? 0 : 1) << 0);
    texFlags |= 0x2;
    block.add_u8(texFlags); // build texFlags
    block.add_u8((u8)bsp1.texIdxAnimData.size());
    block.add_u8(bsp1.texIdx);
    u8 colorFlags = 0;
    colorFlags |= ((bsp1.colorCalcIdxType & 0x7) << 4);
    colorFlags |= ((bsp1.colorPrmAnimData.size() != 0 ? 1 : 0) << 1);
    colorFlags |= ((bsp1.colorEnvAnimData.size() != 0 ? 1 : 0) << 3);
    colorFlags |= bsp1.colorFlags & 0x5;
    block.add_u8(colorFlags);
    block.add_u8(bsp1.colorPrmAnimData.size());
    block.add_u8(bsp1.colorEnvAnimData.size());
    block.add_u16(bsp1.colorAnimMaxFrm);
    block.add_color(bsp1.colorPrm);
    block.add_color(bsp1.colorEnv);
    block.add_u8(bsp1.anmRndm);
    block.add_u8(bsp1.colorLoopOfstMask);
    block.add_u8(bsp1.texIdxLoopOfstMask);
    block.add_padding(3);
    if (bsp1.isEnableTexScrollAnm)
    {
        block.add_f32(bsp1.texInitTransX);
        block.add_f32(bsp1.texIncTransY);
        block.add_f32(bsp1.texInitScaleX);
        block.add_f32(bsp1.texInitScaleY);
        block.add_f32(bsp1.texInitRot);
        block.add_f32(bsp1.texIncTransX);
        block.add_f32(bsp1.texIncTransY);
        block.add_f32(bsp1.texIncScaleX);
        block.add_f32(bsp1.texIncScaleY);
        block.add_f32(bsp1.texIncRot);
    }
    if (bsp1.texIdxAnimData.size() != 0)
    {
        block.add_u8_arr(bsp1.texIdxAnimData);
        if (bsp1.texIdxAnimData.size() % 4 != 0)
            block.add_padding(4 - bsp1.texIdxAnimData.size()%4);
    }
    if(bsp1.colorPrmAnimData.size() != 0)
    {
        block.buffer.at(0xD) = block.buffer.size();
        for(auto& temp : bsp1.colorPrmAnimData)
        {
            block.add_u16(temp.first);
            block.add_color(temp.second);
        }
        if (block.buffer.size()%4 != 0)
        {
            block.add_padding(4 - block.buffer.size()%4);
        }
    }
    if(bsp1.colorEnvAnimData.size() != 0)
    {
        block.buffer.at(0xF) = block.buffer.size();
        for(auto& temp : bsp1.colorEnvAnimData)
        {
            block.add_u16(temp.first);
            block.add_color(temp.second);
        }
        if (block.buffer.size()%4 != 0)
        {
            block.add_padding(4 - block.buffer.size()%4);
        }
    }
    assert(block.buffer.size() == size);
    return block.buffer;
}
vector<u8> build_esp1(JPA_ESP1& esp1)
{
    Buffer block;
    block.add_u32((u32)'ESP1');
    block.add_u32(0x60);
    u32 flags = 0;
    flags |= (esp1.isEnableScale ? 0x01 : 0x00);
    flags |= ((esp1.isDiffXY ? 0x01 : 0x00) << 1);
    flags |= ((esp1.isEnableAlpha ? 0x01 : 0x00) << 16);
    flags |= ((esp1.isEnableSinWave ? 0x01 : 0x00) << 17);
    flags |= ((esp1.isEnableRotate ? 0x01 : 0x00) << 24);
    flags |= ((esp1.scaleAnmTypeX & 0x3) << 0x8);
    flags |= ((esp1.scaleAnmTypeY & 0x3) << 0xA);
    flags |= ((esp1.pivotX & 0x3) << 0xC);
    flags |= ((esp1.pivotY & 0x3) << 0xE);
    block.add_u32(flags);
    block.add_f32(esp1.scaleInTiming);
    block.add_f32(esp1.scaleOutTiming);
    block.add_f32(esp1.scaleInValueX);
    block.add_f32(esp1.scaleOutValueX);
    block.add_f32(esp1.scaleInValueY);
    block.add_f32(esp1.scaleOutValueY);
    block.add_f32(esp1.scaleOutRandom);
    block.add_u16(esp1.scaleAnmMaxFrameX);
    block.add_u16(esp1.scaleAnmMaxFrameY);
    block.add_f32(esp1.alphaInTiming);
    block.add_f32(esp1.alphaOutTiming);
    block.add_f32(esp1.alphaInValue);
    block.add_f32(esp1.alphaBaseValue);
    block.add_f32(esp1.alphaOutValue);
    block.add_f32(esp1.alphaWaveFrequency);
    block.add_f32(esp1.alphaWaveRandom);
    block.add_f32(esp1.alphaWaveAmplitude);
    block.add_f32(esp1.rotateAngle);
    block.add_f32(esp1.rotateAngleRandom);
    block.add_f32(esp1.rotateSpeed);
    block.add_f32(esp1.rotateSpeedRandom);
    block.add_f32(esp1.rotateDirection);

    assert(0x60 == block.buffer.size());
    return block.buffer;
}
vector<u8> build_ssp1(JPA_SSP1& ssp1)
{
    Buffer block;
    block.add_u32((u32)'SSP1');
    block.add_u32(0x48);
    u32 flags = 0;
    flags |= (ssp1.shapeType & 0xF);
    flags |= ((ssp1.dirType & 0x7) << 4);
    flags |= ((ssp1.dirType & 0x7) << 7);
    flags |= ((ssp1.planeType & 0x1) << 10);
    flags |= ((ssp1.isEnableRotate ? 0x1 : 0x0) << 24);
    flags |= ((ssp1.isEnableAlphaOut ? 0x1 : 0x0) << 23);
    flags |= ((ssp1.isEnableScaleOut ? 0x1 : 0x0) << 22);
    flags |= ((ssp1.isEnableField ? 0x1 : 0x0) << 21);
    flags |= ((ssp1.isInheritedRGB ? 0x1 : 0x0) << 18);
    flags |= ((ssp1.isInheritedAlpha ? 0x1 : 0x0) << 17);
    flags |= ((ssp1.isInheritedScale ? 0x1 : 0x0) << 16);
    block.add_u32(flags);
    block.add_f32(ssp1.posRndm);
    block.add_f32(ssp1.baseVel);
    block.add_f32(ssp1.baseVelRndm);
    block.add_f32(ssp1.velInfRate);
    block.add_f32(ssp1.gravity);
    block.add_f32_arr(ssp1.globalScale2D);
    block.add_f32(ssp1.inheritScale);
    block.add_f32(ssp1.inheritAlpha);
    block.add_f32(ssp1.inheritRGB);    
    block.add_color(ssp1.colorPrm);
    block.add_color(ssp1.colorEnv);
    block.add_f32(ssp1.timing);
    block.add_u16(ssp1.life);
    block.add_u16(ssp1.rate);
    block.add_u8(ssp1.step);
    block.add_u8(ssp1.texIdx);
    block.add_u16(ssp1.rotateSpeed);
    // cout << "Buffer: " << block.buffer.size() << endl;
    assert(block.buffer.size() == 0x48);
    return block.buffer;
}
vector<u8> build_etx1_JPAC_10(JPA_ETX1& etx1)
{
    Buffer block;
    block.add_u32((u32)'ETX1');
    block.add_u32(0x28);
    block.add_u16(0);
    block.add_u8(etx1.secondTextureID == -1 ? 0 : 1);
    block.add_u8(etx1.indTextureMode);
    block.add_f32_arr(etx1.floats);
    block.add_u8(etx1.scale);
    block.add_u8(etx1.indTextureID);
    block.add_u8(etx1.secondTextureID == -1 ? 0 : etx1.secondTextureID);
    block.add_padding(1);
    assert(block.buffer.size() == 0x28);
    return block.buffer;
}
vector<u8> build_etx1_JPAC_11(JPA_ETX1& etx1)
{
    // TODO this will change when more gets figured out
    Buffer block;
    block.add_u32((u32)'ETX1');
    block.add_u32(0x50);
    block.add_u16(0);
    block.add_u8(etx1.secondTextureID == 0xFF ? 0 : 1);
    block.add_u8(etx1.indTextureMode);
    block.add_f32_arr(etx1.floats); // TODO: Add verification for this
    block.add_u8(etx1.unk);
    block.add_u8(etx1.scale);
    block.add_u8(etx1.indTextureID);
    block.add_u8(etx1.secondTextureID == 0xFF ? 0 : etx1.secondTextureID);
    assert(block.buffer.size() == 0x50);
    return block.buffer;
}
vector<u8> build_fld1(JPA_FLD1& fld1)
{
    Buffer block;
    block.add_u32((u32)'FLD1');
    block.add_u32(0x44);
    block.add_u16(fld1.sttFlag);
    block.add_u8((fld1.addType & 0x3) | ((fld1.origFlags & 0xFC00)>>8)); // TODO add verification
    block.add_u8((fld1.type & 0xF) | ((fld1.origFlags & 0xF0)>>4));
    block.add_vec3f(fld1.pos);
    block.add_vec3f(fld1.dis);
    block.add_f32(fld1.param1);
    block.add_f32(fld1.param2);
    block.add_f32(fld1.param3);
    block.add_f32(fld1.fadeIn);
    block.add_f32(fld1.fadeOut);
    block.add_f32(fld1.enTime);
    block.add_f32(fld1.disTime);
    block.add_u8(fld1.cycle);
    block.add_padding(3);
    assert(0x44 == block.buffer.size());
    return block.buffer;
}
vector<u8> build_kfa1(JPA_KFA1 &kfa1)
{
    Buffer block;
    block.add_u32((u32)'KFA1');
    u32 size = kfa1.keyValues.size();
    u32 block_size = size*4 + 0xC; // key value array size + header
    block.add_u32(block_size); 
    block.add_u8(kfa1.keyType);
    block.add_u8(size/4); // number of keys is div by 4 due to each key being 4 floats
    block.add_u8(kfa1.unk0xA);
    block.add_u8(kfa1.isLoopEnable ? 0x01 : 0x00);
    block.add_f32_arr(kfa1.keyValues);
    assert(block_size == block.buffer.size());
    return block.buffer;
}
vector<u8> build_tdb1(vector<u16>& tdb1)
{
    // TODO: Add support for name->id matching
    Buffer block;
    block.add_u32((u32)'TDB1');
    u32 len = tdb1.size() * 2;
    if (len % 4 != 0)
        len += (4 - (len%4)); // Buffers it already.
    len+=0x8;
    block.add_u32(len);
    block.add_u16_arr(tdb1);
    if (block.buffer.size() != len)
    {
        block.add_padding(len - block.buffer.size());
    }
    assert(len == block.buffer.size());
    return block.buffer;
}

vector<u8> build_resource(JPA_Resource& resource)
{
    Buffer block;
    block.add_u16(resource.resourceId);
    // u16 block count
    // u8 fieldBlockCount
    // u8 keyBlockCount
    // tdb1 Entry Count
    // Block order: BEM | FLD | KDA | BSP | SSP | ESP | ETX | TDB
    u16 bemNum, fldNum, kfaNum, bspNum, sspNum, espNum, etxNum, tdbNum;
    bemNum = resource.bem1.size(); 
    fldNum = resource.fld1.size(); 
    kfaNum = resource.kfa1.size(); 
    bspNum = resource.bsp1.size(); 
    sspNum = resource.ssp1.size(); 
    espNum = resource.esp1.size(); 
    etxNum = resource.etx1.size(); 
    tdbNum = resource.tdb1.size();
    u16 block_count = bemNum+fldNum+kfaNum+bspNum+sspNum+sspNum+espNum+etxNum+1;
    block.add_u16(block_count);
    block.add_u8(fldNum);
    block.add_u8(kfaNum);
    block.add_u8(tdbNum);
    block.add_u8(0);
    for (auto& blk : resource.bem1){
        vector<u8> temp = build_bem1(blk);
        block.add_u8_arr(temp);
    };
    for (auto& blk : resource.fld1){
        vector<u8> temp = build_fld1(blk);
        block.add_u8_arr(temp);
    };
    for (auto& blk : resource.kfa1){
        vector<u8> temp = build_kfa1(blk);
        block.add_u8_arr(temp);
    };
    for (auto& blk : resource.bsp1){
        vector<u8> temp = build_bsp1_JPAC_11(blk);
        block.add_u8_arr(temp);
    };
    for (auto& blk : resource.esp1){
        vector<u8> temp = build_esp1(blk);
        block.add_u8_arr(temp);
    };
    for (auto& blk : resource.ssp1){
        vector<u8> temp = build_ssp1(blk);
        block.add_u8_arr(temp);
    };
    for (auto& blk : resource.etx1){
        vector<u8> temp = build_etx1_JPAC_11(blk);
        block.add_u8_arr(temp);
    };
    {
        vector<u8> temp = build_tdb1(resource.tdb1);
        block.add_u8_arr(temp);
    }
    return block.buffer;
}
vector<u8> build_jpc(JPAC& jpac)
{
    // first 16 bytes is Verision | resource count | tex offset
    Buffer file;
    file.add_u32('JPAC');
    if (jpac.version.at(7) == '1')
    {
        file.add_u32('2_11');
    }
    else if (jpac.version.at(7) == '0')
    {
        file.add_u32('2_10');
    }
    else 
    {
        assert(!"Non Compatible Verison");
    }
    file.add_u16(jpac.resources.size());
    file.add_u16(jpac.textures.size());
    file.add_u32(0);
    for (auto& resource : jpac.resources){
        vector<u8> resources = build_resource(resource);
        file.add_u8_arr(resources);
    }
    if (file.buffer.size()%16 != 0)
    {
        file.add_padding(16 - file.buffer.size()%16);
    }
    u16 currEnd = file.buffer.size();
    file.buffer.at(0xC) = (currEnd >> 24) & 0xFF;
    file.buffer.at(0xD) = (currEnd >> 16) & 0xFF;
    file.buffer.at(0xE) = (currEnd >> 8) & 0xFF;
    file.buffer.at(0xF) = (currEnd >> 0) & 0xFF;

    for (auto& textures : jpac.textures)
    {
        string name = "./TextureInput/" + textures.name;
        ifstream jpcFile(name, ios::binary | ios::in | ios::ate);
        vector<u8> data;
        if(jpcFile.is_open())
        {
            i32 size = jpcFile.tellg();
            data = vector<u8>(size);
            u8* newdata = new u8[size]; // allocs enough mem for the file
            jpcFile.seekg(0, ios::beg);
            jpcFile.read((char *)newdata, size);
            jpcFile.close(); // at this point, the file should be completely in mem
            for (int i = 0; i < size; i++)
                data.at(i) = (newdata[i]);
            // assert(data.size() == size);
            delete[] newdata;
        } else {
            cout << "Could not locate file: " << textures.name << endl;
            assert(!"Make sure folder \"TextureInput\" exists with the dumped textures");
        }
        file.add_u32('TEX1');
        file.add_u32(data.size()+0x20);
        file.add_u32(0);
        for(char s : textures.name)
            file.add_i8(s);
        file.add_padding(20-textures.name.length());
        file.add_u8_arr(data);
    }
    return file.buffer;
}
int main()
{
    ifstream input("jpc.json");
    json j;
    input >> j;
    JPAC allResources = j;

    vector<u8> file = build_jpc(allResources);
    
    ofstream output("Common.jpc");
    if (output.is_open())
    {
        output.write((char*)&file[0], file.size());
    }
    output.close();
    return 0;
}