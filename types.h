#pragma once
#include <stdint.h>
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

struct LocalizeEntry
{
	const char *value;
	const char *name;
};
struct RawFile
{
	const char *name;
	int len;
	const char *buffer;
};
typedef struct RawFile RawFile;
typedef struct LocalizeEntry LocalizeEntry;
union XAssetHeader
{
	// XModelPieces *xmodelPieces;
	// PhysPreset *physPreset;
	// XAnimParts *parts;
	// XModel *model;
	// Material *material;
	// MaterialPixelShader *pixelShader;
	// MaterialVertexShader *vertexShader;
	// MaterialTechniqueSet *techniqueSet;
	// GfxImage *image;
	// snd_alias_list_t *sound;
	// SndCurve *sndCurve;
	// LoadedSound *loadSnd;
	// clipMap_t *clipMap;
	// ComWorld *comWorld;
	// GameWorldSp *gameWorldSp;
	// GameWorldMp *gameWorldMp;
	// MapEnts *mapEnts;
	// GfxWorld *gfxWorld;
	// GfxLightDef *lightDef;
	// Font_s *font;
	// MenuList *menuList;
	// menuDef_t *menu;
	LocalizeEntry *localize;
	// WeaponDef *weapon;
	// SndDriverGlobals *sndDriverGlobals;
	// FxEffectDef *fx;
	// FxImpactTable *impactFx;
	RawFile *rawfile;
	// StringTable *stringTable;
	void *data;
};
typedef enum XAssetType XAssetType;
typedef union XAssetHeader XAssetHeader;
#ifdef PROTO
enum
{
	XFILE_BLOCK_TEMP = 0x0,
	XFILE_BLOCK_RUNTIME = 0x1,
	XFILE_BLOCK_VIRTUAL = 0x2,
	XFILE_BLOCK_LARGE = 0x3,
	XFILE_BLOCK_PHYSICAL = 0x4,
	MAX_XFILE_COUNT = 0x5,
};

enum XAssetType
{
	ASSET_TYPE_RAWFILE = 0x1B,
	// ...
	ASSET_TYPE_COUNT,
	ASSET_TYPE_STRING,
	ASSET_TYPE_ASSETLIST
};
struct XFile
{
	u32 version; // size?
	u32 size;
	u32 blockSize[MAX_XFILE_COUNT];
};
typedef struct XFile XFile;
enum
{
	sizeof_XFile = sizeof(XFile)
};

#else
	// #define MAX_XFILE_COUNT (8)
	#define MAX_XFILE_COUNT (6)
struct XZoneMemory
{
	u32 size;
	u32 externalsize;
	u32 streams[MAX_XFILE_COUNT]; // RawFile always writes to [3] XFILE_BLOCK_LARGE? buffer align(0) pos = ~alignment & (alignment + pos)
};
typedef struct XZoneMemory XZoneMemory;
typedef struct
{
	char magic[8];
	u32 version;
	u8 allowOnlineUpdate; // flags?
	/*
	0x1 = allowOnlineUpdate
	0xd = russian
	0xb = english
	loc_language 0-14
	*/
	u32 dwHighDateTime;
	u32 dwLowDateTime;

	// Extra
	u32 idk;
	// char idk[21];
	// char idk2[4];
	u8 language;
	u32 numStreamFile; // XAssetStreamFile
	u32 zoneStart;	   // XZoneMemory
	u32 zoneEnd;	   // XZoneMemory
} XFileHeader;
// 37 if PS3/XBox (38 if NX)
// 21 else

struct XAssetStreamFile
{
	u32 fileIndex;
	u32 offset;
	u32 offsetEnd;
};

//NX1
enum XAssetType
{
	ASSET_TYPE_LOCALIZE_ENTRY = 0x1A,
	ASSET_TYPE_RAWFILE = 0x24,

	// ...
	ASSET_TYPE_COUNT,
	ASSET_TYPE_STRING,
	ASSET_TYPE_ASSETLIST
};
#endif
struct XAssetList
{
	u32 scriptStringCount;
	u32 scriptStrings;
	u32 assetCount;
	u32 assets;
	// u32 idk;
	// u32 idk2;
};

struct XAsset
{
	u32 type;
	u32 ptr;
};

typedef struct XAssetList XAssetList;
typedef struct XAsset XAsset;

#define COUNT_OF(X) (sizeof(X) / sizeof((X)[0]))
