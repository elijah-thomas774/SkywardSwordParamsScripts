#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <cinttypes>
#include <string>
#include <vector>

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
typedef std::vector<u8> data_t;
struct Color{
    u32 r;
    u32 g;
    u32 b;
    u32 a;
};
std::string TEX_FORMATS[] {
    "I4",
    "I8",
    "IA4",
    "IA8",
    "RGB565",
    "RGB5A3",
    "RGBA8",
    "77",
    "C4",
    "C8",
    "C14X2",
    "bb",
    "cc",
    "dd",
    "CMPR",
    "ff",
};

struct JPAResourceRaw {
    u16 resourceId;
    data_t data;
    u32 data_offset;
    u16 texIdBase;
};

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
   /* 0x1C */ u32 imageDataOffset;            
};
typedef enum VolumeType{
    Cube,
    Sphere,
    Cylinder,
    Torus,
    Point,
    Circle,
    Line,
}VolumeType_t;
typedef enum EmitFlags{
    FixedDensity         = 0x01,
    FixedInterval        = 0x02,
    InheritScale         = 0x04,
    FollowEmitter        = 0x08,
    FollowEmitterCholder = 0x10,
}EmitFlag_t;
struct JPADynamicsBlock {
    EmitFlags emitFlags;
    VolumeType volumeType;
    f32 emmiterSc1[3];
    f32 emitterRot[3];
    f32 emitterTrs[3];
    f32 emitterDir[3];
    u32 initialVelOmni;
    u32 initialVelAxis;
    u32 initialVelRndm;
    u32 initialVelDir;
    u32 spread;
    u32 initialVelRatio;
    u32 rate;
    u32 rateRndm;
    u32 lifeTimeRndm;
    u32 volumeSweep;
    u32 volumMinRad;
    u32 airResist;
    u32 airResistRndm;
    u32 moment;
    u32 momentRndm;
    u32 accel;
    u32 accelRndm;
    u32 maxFrame;
    u32 startFrame;
    u32 lifeTime;
    u32 volumeSize;
    u32 divNumber;
    u32 rateStep;
};
typedef enum ShapeType{
    Point_Shape,
    Line_Shape,
    Billboard,
    Direction,
    DirectionCross,
    Stripe,
    StripeCross,
    Rotation,
    RotationCross,
    DirBillbaord,
    YBillboard
}ShapeType_t;
typedef enum DirType{
    Vel,
    Pos,
    PosInv,
    EmtrDir,
    PrevPct1,
}DirType_t;
typedef enum RotType{
    Y,
    X,
    Z,
    XYZ,
    YJiggle
}RotType_t;
typedef enum PlaneType{
    XY,
    XZ,
    X_Plane
}PlaneType_t;
typedef enum CalcIdxType{
    Normal,
    Repeat,
    Reverse,
    Merge,
    Random,
}CalcIdxType_t;
typedef enum CalcScaleAnmType{
    Normal_Scale,
    Repeat_Scale,
    Reverse_Scale,
}CalcScaleAnmType_t;
typedef enum CalcAlphaWaveType{
    None = -1,
    NrmSin = 0,
    AddSin,
    MultSin,
}CalcAlphaWaveType_t;
typedef enum IndTextureMode{
    Off,
    Normal_Ind,
    Sub,
}IndTextureMode_t;
typedef enum FieldType{
    Gravity,
    Air,
    Magnet,
    Newton,
    Vortex,
    Random_Field,
    Drag,
    Convection,
    Spin,
}FieldType_t;
typedef enum FieldAddType{
    FieldAccel,
    BaseVelocity,
    FieldVelocity
}FieldAddType_t;
typedef enum FieldStatusFlag{
    NoInheritRotate = 0x02,
    AirDrag = 0x04,
    FadeUseEnTime = 0x08,
    FadeUseDisTime = 0x10,
    FadeUseFadeIn = 0x20,
    FadeUseFadeOut = 0x40,
    FadeFlagMask = (0x08 | 0x10 | 0x20 | 0x40),
    UseMaxDist = 0x80,
}FieldStatusFlag_t;
typedef enum JPAKeyType{
    Rate,
    VolumeSize,
    VolumeSweep,
    VolumeMinRad,
    LifeTime,
    Moment,
    InitialVelOmni,
    InitialVelAxis,
    InitialVelDir,
    Spread,
    Scale,
}JPAKeyType_t;
struct CommonShapeTypeFields {
    ShapeType shape_type;
    DirType dir_type;
    RotType rot_type;
    PlaneType plane_type;
};
struct JPABaseShapeBlock {
    ShapeType shape_type;
    DirType_t dir_type;
    RotType_t rot_type;
    PlaneType_t plane_type;
    f32 basesize[2];
    u32 tilingS;
    u32 tilingT;
    bool isDrawFwdAhead;
    bool isDrawPrntAhead;
    bool isNoDrawParent;
    bool isNoDrawChild;

    // TEV/PE Settings
    u32 colorInSelect;
    u32 alphaInSelect;
    u32 blendModeFlags;
    u32 alphaCompareFlags;
    u32 alphaRef0;
    u32 alphaRef1;
    u32 zModeFlags;

    u32 anmRndm;

    // Texture Palette Animation
    bool isEnableTexture;
    bool isGlblTexAnm;
    CalcIdxType_t texCalcIdxType;
    u32 texIdx;
    std::vector<u8> Uint8Array;
    u32 texIdxLoopOfstMask;

    // Texture Coordinate Animation
    bool isEnableProjection;
    bool isEnableTexScrollAnm;
    u32 texInitTransX;
    u32 texInitTransY;
    u32 texInitScaleX;
    u32 texInitScaleY;
    u32 texInitRot;
    u32 texIncTransX;
    u32 texIncTransY;
    u32 texIncScaleX;
    u32 texIncScaleY;
    u32 texIncRot;

    // Color Animation Settings
    bool isGlblClrAnm;
    CalcIdxType_t colorCalcIdxType;
    Color colorPrm;
    Color colorEnv;
    std::vector<Color> colorPrmAnimData;
    std::vector<Color> colorEnvAnimData;
    u32 colorAnimMaxFrm;
    u32 colorLoopOfstMask;
};

struct JPAExtraShapeBlock{
    bool isEnableScale;
    bool isDiffXY;
    bool isEnableScaleBySpeedX;
    bool isEnableScaleBySpeedY;
    CalcScaleAnmType_t scaleAnmTypeX;
    CalcScaleAnmType_t scaleAnmTypeY;
    bool isEnableRotate;
    bool isEnableAlpha;
    CalcAlphaWaveType_t alphaWaveType;
    u32 pivotX;
    u32 pivotY;
    u32 scaleInTiming;
    u32 scaleOutTiming;
    u32 scaleInValueX;
    u32 scaleOutValueX;
    u32 scaleInValueY;
    u32 scaleOutValueY;
    u32 scaleOutRandom;
    u32 scaleAnmMaxFrameX;
    u32 scaleAnmMaxFrameY;
    u32 scaleIncreaseRateX;
    u32 scaleIncreaseRateY;
    u32 scaleDecreaseRateX;
    u32 scaleDecreaseRateY;
    u32 alphaInTiming;
    u32 alphaOutTiming;
    u32 alphaInValue;
    u32 alphaBaseValue;
    u32 alphaOutValue;
    u32 alphaIncreaseRate;
    u32 alphaDecreaseRate;
    u32 alphaWaveParam1;
    u32 alphaWaveParam2;
    u32 alphaWaveParam3;
    u32 alphaWaveRandom;
    u32 rotateAngle;
    u32 rotateAngleRandom;
    u32 rotateSpeed;
    u32 rotateSpeedRandom;
    u32 rotateDirection;
};
struct JPAExTexBlock {
    IndTextureMode_t indTextureMode;
    std::vector<f32> indTextureMtx;
    u32 indTextureID;
    u32 subTextureID;
    u32 secondTextureIndex;
};

struct JPAChildShapeBlock {
    bool isInheritedScale;
    bool isInheritedRGB;
    bool isInheritedAlpha;
    bool isEnableAlphaOut;
    bool isEnableField;
    bool isEnableRotate;
    bool isEnableScaleOut;
    ShapeType shapeType;
    DirType_t dirType;
    RotType_t rotType;
    PlaneType_t planeType;
    u32 posRndm;
    u32 baseVel;
    u32 baseVelRndm;
    u32 velInfRate;
    u32 gravity;
    f32 globalScale2D[2];
    u32 inheritScale;
    u32 inheritAlpha;
    u32 inheritRGB;
    Color colorPrm;
    Color colorEnv;
    u32 timing;
    u32 life;
    u32 rate;
    u32 step;
    u32 texIdx;
    u32 rotateSpeed;
};

struct JPAFieldBlock {
    FieldStatusFlag_t sttFlag;
    FieldType_t type;
    FieldAddType_t addType;
    // Used by JPA1 and JEFFjpa1
    u32 maxDistSq;
    f32 pos[3];
    f32 dir[3];
    u32 fadeIn;
    u32 fadeOut;
    u32 disTime;
    u32 enTime;
    u32 cycle;
    u32 fadeInRate;
    u32 fadeOutRate;

    // Used by Gravity, Air, Magnet, Newton, Vortex, Random, Drag, Convection, Spin
    u32 mag;
    // Used by Drag
    u32 magRndm;
    // Used by Newton, Air and Convection
    u32 refDistance;
    // Used by Vortex and Spin
    u32 innerSpeed;
    // Used by Vortex
    u32 outerSpeed;
};
struct JPAKeyBlock {
    JPAKeyType_t keyType;
    std::vector<f32> keyValues;
    bool isLoopEnable;
};

struct JPAResource {
    JPADynamicsBlock           bem1;
    JPABaseShapeBlock          bsp1;
    JPAExtraShapeBlock         esp1;
    JPAExTexBlock              etx1;
    JPAChildShapeBlock         ssp1;
    std::vector<JPAFieldBlock> fld1;
    std::vector<JPAKeyBlock>   kfa1;
    std::vector<u16>           tdb1;
};
#endif