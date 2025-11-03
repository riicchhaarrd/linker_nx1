#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include "types.h"
// #pragma comment(lib, "zlib.lib")
// #include <zlib.h>
#include "miniz.h"
#include "loc.h"
#include "util.h"

#define APP_VERSION "0.1.0"

// Convert a 32-bit integer to big-endian
uint32_t to_big_endian_32(uint32_t val)
{
	return ((val & 0x000000FF) << 24) | ((val & 0x0000FF00) << 8) | ((val & 0x00FF0000) >> 8)
		   | ((val & 0xFF000000) >> 24);
}

// Convert a 16-bit integer to big-endian
uint16_t to_big_endian_16(uint16_t val)
{
	return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}

// Helper to write a 32-bit integer in big-endian
void write_be32(FILE *file, uint32_t val)
{
	uint32_t be_val = to_big_endian_32(val);
	fwrite(&be_val, sizeof(be_val), 1, file);
}

// Helper to write a 16-bit integer in big-endian
void write_be16(FILE *file, uint16_t val)
{
	uint16_t be_val = to_big_endian_16(val);
	fwrite(&be_val, sizeof(be_val), 1, file);
}

void write_string(FILE *file, const char *str)
{
	fwrite(str, sizeof(char), strlen(str), file); // write characters
	char null_byte = '\0';
	fwrite(&null_byte, sizeof(char), 1, file); // write null terminator
}

// void *myalloc(void *q, unsigned n, unsigned m)
// {
// 	(void)q;
// 	return calloc(n, m);
// }

// void myfree(void *q, void *p)
// {
// 	(void)q;
// 	free(p);
// }

// static alloc_func zalloc = myalloc;
// static free_func zfree = myfree;

// void compress_memory(const unsigned char *data, size_t dataLen, unsigned char **compressed, size_t *compressedLen)
// {
// 	int err;
// 	z_stream c_stream; /* compression stream */

// 	c_stream.zalloc = zalloc;
// 	c_stream.zfree = zfree;
// 	c_stream.opaque = (voidpf)0;
// 	c_stream.next_in = (Bytef *)data;
// 	c_stream.avail_in = (uInt)dataLen;

// 	err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
// 	/* CHECK_ERR(err, "deflateInit"); */

// 	size_t capacity = 4096;
// 	size_t size = 0;
// 	unsigned char *output = (unsigned char *)malloc(capacity);
// 	unsigned char buf[4096];

// 	for(;;)
// 	{
// 		c_stream.next_out = buf;
// 		c_stream.avail_out = sizeof(buf);
// 		err = deflate(&c_stream, Z_FINISH);

// 		size_t have = sizeof(buf) - c_stream.avail_out;

// 		/* Resize output buffer if needed */
// 		if(size + have > capacity)
// 		{
// 			capacity *= 2;
// 			output = (unsigned char *)realloc(output, capacity);
// 		}

// 		/* Copy compressed data to output buffer */
// 		memcpy(output + size, buf, have);
// 		size += have;

// 		if(err == Z_STREAM_END)
// 			break;
// 		/* CHECK_ERR(err, "large deflate"); */
// 	}

// 	err = deflateEnd(&c_stream);

// 	*compressed = output;
// 	*compressedLen = size;
// }

void *myalloc(void *opaque, size_t items, size_t size)
{
    (void)opaque;
    return calloc(items, size);
}

void myfree(void *opaque, void *address)
{
    (void)opaque;
    free(address);
}

static mz_alloc_func zalloc = myalloc;
static mz_free_func zfree = myfree;

void compress_memory(const unsigned char *data, size_t dataLen, unsigned char **compressed, size_t *compressedLen)
{
    int err;
    mz_stream c_stream; /* compression stream */
    
    memset(&c_stream, 0, sizeof(c_stream));
    
    c_stream.zalloc = zalloc;
    c_stream.zfree = zfree;
    c_stream.opaque = NULL;
    c_stream.next_in = data;
    c_stream.avail_in = (unsigned int)dataLen;
    
    err = mz_deflateInit(&c_stream, MZ_DEFAULT_COMPRESSION);
    /* CHECK_ERR(err, "deflateInit"); */
    
    size_t capacity = 4096;
    size_t size = 0;
    unsigned char *output = (unsigned char *)malloc(capacity);
    unsigned char buf[4096];
    
    for(;;)
    {
        c_stream.next_out = buf;
        c_stream.avail_out = sizeof(buf);
        
        err = mz_deflate(&c_stream, MZ_FINISH);
        
        size_t have = sizeof(buf) - c_stream.avail_out;
        
        /* Resize output buffer if needed */
        if(size + have > capacity)
        {
            capacity *= 2;
            output = (unsigned char *)realloc(output, capacity);
        }
        
        /* Copy compressed data to output buffer */
        memcpy(output + size, buf, have);
        size += have;
        
        if(err == MZ_STREAM_END)
            break;
        /* CHECK_ERR(err, "large deflate"); */
    }
    
    err = mz_deflateEnd(&c_stream);
    
    *compressed = output;
    *compressedLen = size;
}

char *read_file_to_memory(const char *filename, size_t *out_size)
{
	FILE *file = fopen(filename, "rb");
	if(!file)
		return NULL;

	// Determine file size
	fseek(file, 0, SEEK_END);
	long filesize = ftell(file);
	rewind(file);

	if(filesize < 0)
	{
		fclose(file);
		return NULL;
	}

	char *buffer = malloc(filesize + 1); // +1 for null terminator
	if(!buffer)
	{
		fclose(file);
		return NULL;
	}

	size_t read_size = fread(buffer, 1, filesize, file);
	fclose(file);
	// These won't match on Windows with text files \r\n
	//if(read_size != filesize)
	//{
	//	free(buffer);
	//	return NULL;
	//}

	buffer[filesize] = '\0'; // Null-terminate
	if(out_size)
		*out_size = filesize;
	return buffer;
}

static const char *scriptStrings[] = { };
// static const char *scriptStrings[] = { "yoyo is gay" };

// static const char *scriptStrings[] = { "tag_origin",
// 									   "j_mainroot",
// 									   "j_coatfront_le",
// 									   "j_coatfront_ri",
// 									   "j_coatrear_le",
// 									   "j_coatrear_ri",
// 									   "j_hip_le",
// 									   "j_hip_ri",
// 									   "j_spinelower",
// 									   "j_hiptwist_le",
// 									   "j_hiptwist_ri",
// 									   "j_knee_le",
// 									   "j_knee_ri",
// 									   "j_shorts_le",
// 									   "j_shorts_lift_le",
// 									   "j_shorts_lift_ri",
// 									   "j_shorts_ri",
// 									   "j_spineupper",
// 									   "j_ankle_le",
// 									   "j_ankle_ri",
// 									   "j_knee_bulge_le",
// 									   "j_knee_bulge_ri",
// 									   "j_spine4",
// 									   "j_ball_le",
// 									   "j_ball_ri",
// 									   "j_clavicle_le",
// 									   "j_clavicle_ri",
// 									   "j_neck",
// 									   "j_shoulderraise_le",
// 									   "j_shoulderraise_ri",
// 									   "joint1",
// 									   "joint2",
// 									   "tag_weapon_chest",
// 									   "j_head",
// 									   "j_shoulder_le",
// 									   "j_shoulder_ri",
// 									   "j_brow_le",
// 									   "j_brow_ri",
// 									   "j_cheek_le",
// 									   "j_cheek_ri",
// 									   "j_elbow_bulge_le",
// 									   "j_elbow_bulge_ri",
// 									   "j_elbow_le",
// 									   "j_elbow_ri",
// 									   "j_eye_lid_bot_le",
// 									   "j_eye_lid_bot_ri",
// 									   "j_eye_lid_top_le",
// 									   "j_eye_lid_top_ri",
// 									   "j_eyeball_le",
// 									   "j_eyeball_ri",
// 									   "j_head_end",
// 									   "j_jaw",
// 									   "j_levator_le",
// 									   "j_levator_ri",
// 									   "j_lip_top_le",
// 									   "j_lip_top_ri",
// 									   "j_mouth_le",
// 									   "j_mouth_ri",
// 									   "j_shouldertwist_le",
// 									   "j_shouldertwist_ri",
// 									   "tag_eye",
// 									   "j_chin_skinroll",
// 									   "j_helmet",
// 									   "j_lip_bot_le",
// 									   "j_lip_bot_ri",
// 									   "j_wrist_le",
// 									   "j_wrist_ri",
// 									   "j_wristtwist_le",
// 									   "j_wristtwist_ri",
// 									   "j_gun",
// 									   "j_index_le_1",
// 									   "j_index_ri_1",
// 									   "j_mid_le_1",
// 									   "j_mid_ri_1",
// 									   "j_pinky_le_1",
// 									   "j_pinky_ri_1",
// 									   "j_ring_le_1",
// 									   "j_ring_ri_1",
// 									   "j_thumb_le_1",
// 									   "j_thumb_ri_1",
// 									   "tag_weapon_left",
// 									   "tag_weapon_right",
// 									   "j_index_le_2",
// 									   "j_index_ri_2",
// 									   "j_mid_le_2",
// 									   "j_mid_ri_2",
// 									   "j_pinky_le_2",
// 									   "j_pinky_ri_2",
// 									   "j_ring_le_2",
// 									   "j_ring_ri_2",
// 									   "j_thumb_le_2",
// 									   "j_thumb_ri_2",
// 									   "j_index_le_3",
// 									   "j_index_ri_3",
// 									   "j_mid_le_3",
// 									   "j_mid_ri_3",
// 									   "j_pinky_le_3",
// 									   "j_pinky_ri_3",
// 									   "j_ring_le_3",
// 									   "j_ring_ri_3",
// 									   "j_thumb_le_3",
// 									   "j_thumb_ri_3",
// 									   "end",
// 									   "trim_char_f_1_1",
// 									   "trim_char_x_1_1" };
// static const XAsset assets[] = {
// 	// { 0x1b, 0xffffffff }, { 0x6, 0xffffffff },	{ 0x6, 0xffffffff },  { 0x6, 0xffffffff },	{ 0x6, 0xffffffff },
// 	// { 0x6, 0xffffffff },  { 0x6, 0xffffffff },	{ 0x6, 0xffffffff },  { 0x6, 0xffffffff },	{ 0x6, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x8, 0xffffffff },	{ 0x14, 0xffffffff }, { 0x7, 0xffffffff },	{ 0x7, 0xffffffff },
// 	// { 0x7, 0xffffffff },  { 0x7, 0xffffffff },	{ 0x7, 0xffffffff },  { 0x7, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0xf, 0xffffffff },  { 0xf, 0xffffffff },	{ 0xf, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x15, 0xffffffff },
// 	// { 0x16, 0xffffffff }, { 0xd, 0xffffffff },	{ 0xd, 0xffffffff },  { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff },
// 	// { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x12, 0xffffffff }, { 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x5, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0xf, 0xffffffff },	{ 0xf, 0xffffffff },  { 0xf, 0xffffffff },	{ 0xf, 0xffffffff },
// 	// { 0xf, 0xffffffff },  { 0xf, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x5, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x10, 0xffffffff }, { 0x10, 0xffffffff }, { 0x13, 0xffffffff }, { 0x2, 0xffffffff },	{ 0x3, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x4, 0xffffffff },  { 0x4, 0xffffffff },	{ 0x4, 0xffffffff },  { 0x1, 0xffffffff },	{ 0x4, 0xffffffff },
// 	// { 0x1b, 0xffffffff }, { 0x1b, 0xffffffff }
// 	{ ASSET_TYPE_RAWFILE, -1 }
// };
typedef struct Asset
{
	char *filename;
	char *name;
	XAssetType type;
	XAssetHeader *header;
	struct Asset *next;
} Asset;
Asset *assets;

XAssetType asset_type_for_string(const char *type)
{
	#ifndef PROTO
	if(!strcmp(type, "localize"))
		return ASSET_TYPE_LOCALIZE_ENTRY;
	#endif
	if(!strcmp(type, "rawfile"))
		return ASSET_TYPE_RAWFILE;
	return -1;
}

Asset *new_xasset(XAssetType type, char *name, char *filename)
{
	Asset *f = calloc(1, sizeof(Asset));
	f->type = type;
	f->filename = filename;
	f->name = name ? strdup(name) : NULL;
	f->header = calloc(1, sizeof(XAssetHeader));

	f->filename = strdup(filename);
	// f->next = assets;
	// assets = f;
	f->next = NULL;
	if (!assets) {
        assets = f;  // first element
    } else {
        Asset *current = assets;
        while (current->next) {
            current = current->next;  // traverse to end
        }
        current->next = f;  // append
    }
	return f;
}

int serialize_localize_entry(FILE *fp, XAssetHeader *asset)
{
	u32 ptr = -1;
	write_be32(fp, ptr);
	u32 ptr2 = -1;
	write_be32(fp, ptr2);
	LocalizeEntry *loc = asset->localize;
	write_string(fp, loc->value);
	write_string(fp, loc->name);
	return 0;
}

int load_localize_entry(XAssetType type, const char *basename, const char *path)
{
	char tmp[512];
	snprintf(tmp, sizeof(tmp), "%s/english/localizedstrings/%s.str", basename, path);
	char prefix[512];
	get_basename(tmp, prefix);
	Entry *entries = NULL;
	strtoupper(prefix);
	printf("prefix: %s\n", prefix);
	// int n = parse_loc_file("patch/english/localizedstrings/dlc.str", &entries);
	int n = parse_loc_file(tmp, &entries);
	if(n < 0)
	{
		fprintf(stderr, "Failed to parse file.\n");
		return 1;
	}

	char key[512];
	for(int i = 0; i < n; i++)
	{
		// Entry *entry = &entries[n - i - 1];
		Entry *entry = &entries[i];
		unescape_string(entry->value);
		snprintf(key, sizeof(key), "%s_%s", prefix, entry->key);
		printf("%s = %s\n", key, entry->value);

		Asset *a = new_xasset(type, NULL, (char*)path);
		LocalizeEntry *loc = calloc(1, sizeof(LocalizeEntry));
		a->header->localize = loc;
		loc->name = strdup(key);
		loc->value = strdup(entry->value);
	}
	free(entries);
	return 0;
}

int load_raw_file(XAssetType type, const char *basename, const char* path)
{
	Asset *a = new_xasset(type, NULL, (char*)path);
	RawFile *rf = calloc(1, sizeof(RawFile));
	a->header->rawfile = rf;
	char tmp[512];
	snprintf(tmp, sizeof(tmp), "%s/%s", basename, path);
	size_t outsz = 0;
	rf->buffer = read_file_to_memory(tmp, &outsz);
	rf->len = outsz;
	rf->name = a->filename;
	return 0;
}

int serialize_raw_file(FILE *fp, XAssetHeader *asset)
{
	RawFile *rf = asset->rawfile;
	#ifdef PROTO
	u32 ptr = -1;
	write_be32(fp, ptr);
	write_be32(fp, strlen(rf->buffer)); // rf->len
	u32 ptr2 = -1;
	write_be32(fp, ptr2);
	write_string(fp, rf->name);
	write_string(fp, rf->buffer);
	#else
	u32 compressedLen = 0; // Don't compress lol

	u32 ptr = -1;
	write_be32(fp, ptr);
	write_be32(fp, compressedLen);
	int n = strlen(rf->buffer);
	write_be32(fp, n); // rf->len
	write_be32(fp, ptr);
	write_string(fp, rf->name);
	write_string(fp, rf->buffer);
	#endif
	return 0;
}

typedef struct
{
	int (*load)(XAssetType type, const char *basename, const char* path);
	int (*serialize)(FILE *fp, XAssetHeader *asset);
} AssetHandler;

AssetHandler asset_handlers[ASSET_TYPE_ASSETLIST] = {
	#ifndef PROTO
	[ASSET_TYPE_LOCALIZE_ENTRY] = { load_localize_entry, serialize_localize_entry },
	#endif
	[ASSET_TYPE_RAWFILE] = { load_raw_file, serialize_raw_file },
};

    #ifndef PROTO
static void write_zone_memory_header(FILE *fp, XZoneMemory *mem)
{
	write_be32(fp, mem->size);
	write_be32(fp, mem->externalsize);
	for(int i = 0; i < MAX_XFILE_COUNT; i++)
		write_be32(fp, mem->streams[i]);
}
#endif
static void write_xassetlist(FILE *fp, XAssetList *list)
{
    #ifndef PROTO
	// write 32 bytes (maybe blockSizes?)
	// from pre_code_gfx.ff
	// const uint8_t idk[32] = {
	// 	0x00, 0x00, 0x08, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
	// 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// };
	// XZoneMemory
	// XZoneMemory zone_memory = { .size = 487, .externalsize = 0, .streams[0] = 32, .streams[3] = 407 };
	const size_t static_size = 5000000; // 5MB
	XZoneMemory zone_memory = { .size = static_size, .externalsize = 0, .streams[0] = static_size, .streams[3] = static_size };
	write_zone_memory_header(fp, &zone_memory);
	// const uint8_t idk[32] = {
	// 	0x00, 0x52, 0x1F, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x8C, 0x00, 0x41, 0x30, 0x00,
	// 	0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0xD1, 0x67, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x1C, 0xE4,
	// };
	// memset(idk, 0, sizeof(idk));
	// fwrite(idk, sizeof(idk), 1, fp);
	#endif
	write_be32(fp, list->scriptStringCount);
	write_be32(fp, list->scriptStrings);
	write_be32(fp, list->assetCount);
	write_be32(fp, list->assets);
	// write_be32(fp, list->idk);
	// write_be32(fp, list->idk2);
	for(int i = 0; i < list->scriptStringCount; ++i)
	{
		write_be32(fp, -1);
	}
	for(int i = 0; i < list->scriptStringCount; ++i)
	{
		const char *str = scriptStrings[i];
		write_string(fp, str);
	}

	for(Asset *it = assets; it; it = it->next)
	{
		write_be32(fp, it->type);
		write_be32(fp, -1);
	}
	// for(int i = 0; i < list->assetCount; i++)
	// {
	// 	const Asset *asset = &assets[i];
	// 	write_be32(fp, asset->type);
	// 	write_be32(fp, asset->ptr);
	// }
}

// static void write_raw_file(FILE *fp, const char *filename, const char *source)
// {
// 	u32 ptr = -1;
// 	write_be32(fp, ptr);
// 	write_be32(fp, strlen(source));
// 	u32 ptr2 = -1;
// 	write_be32(fp, ptr2);
// 	write_string(fp, filename);
// 	write_string(fp, source);
// }

static int write_fastfile(const char *input_filename, const char *output_filename)
{
	FILE *fin = fopen(input_filename, "rb");
	if(!fin)
	{
		perror("fopen input");
		return 1;
	}

	fseek(fin, 0, SEEK_END);
	long filesize = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	if(filesize < 0)
	{
		perror("ftell");
		fclose(fin);
		return 1;
	}

	unsigned char *input_data = malloc(filesize);
	if(!input_data)
	{
		perror("malloc");
		fclose(fin);
		return 1;
	}

	if(fread(input_data, 1, filesize, fin) != (size_t)filesize)
	{
		perror("fread");
		free(input_data);
		fclose(fin);
		return 1;
	}
	fclose(fin);

	uLongf compressed_size = compressBound(filesize);
	unsigned char *compressed_data = malloc(compressed_size);
	if(!compressed_data)
	{
		perror("malloc compressed");
		free(input_data);
		return 1;
	}

	if(compress(compressed_data, &compressed_size, input_data, filesize) != Z_OK)
	{
		fprintf(stderr, "Compression failed\n");
		free(input_data);
		free(compressed_data);
		return 1;
	}
	free(input_data);
	FILE *fout = fopen(output_filename, "wb");
	if(!fout)
	{
		perror("fopen output");
		free(compressed_data);
		return 1;
	}


#ifndef PROTO
	XFileHeader header = { 0 };
	memcpy(&header.magic, "NXffu100", sizeof(header.magic));
	header.version = 357;
	// header.allowOnlineUpdate = 0;
	header.allowOnlineUpdate = 1;
	header.dwHighDateTime = 0x0001CCBC;
	header.dwLowDateTime = 0x918F279E;
	// header.dwHighDateTime = 0x0001CCBC;
	// header.dwLowDateTime = 0x918BEF57;

	fwrite(header.magic, sizeof(header.magic), 1, fout);
	write_be32(fout, header.version);
	fwrite(&header.allowOnlineUpdate, 1, 1, fout);
	write_be32(fout, header.dwHighDateTime);
	write_be32(fout, header.dwLowDateTime);
    // from code_pre_gfx.ff
	// const uint8_t idk[15] = {
	// 	0x60, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xC6, 0x00, 0x00,
	// };
	// memset(idk, 0, sizeof(idk));
	// fwrite(idk, sizeof(idk), 1, fout);

	// header.idk = 0x20000000;
	header.idk = 0xC0000000;
	write_be32(fout, header.idk);
	header.language = 1;
	fwrite(&header.language, 1, 1, fout);
	header.numStreamFile = 0;
	write_be32(fout, header.numStreamFile);
	// header.zoneStart = 1534886; // 1.5MB?
	// header.zoneEnd = 1534886;

	// header.zoneStart = 324;
	// header.zoneEnd = 324;
	header.zoneStart = filesize;
	header.zoneEnd = filesize;
	write_be32(fout, header.zoneStart);
	write_be32(fout, header.zoneEnd);

#else
	XFile header;
	header.blockSize[0] = 0x770;
	header.blockSize[1] = 0;
	header.blockSize[2] = 0x6e74a;
	header.blockSize[3] = 0x35f000;
	header.blockSize[4] = 0xe8a8;
	header.version = 302;
	// header.size=4064019;
	header.size = filesize;

	write_be32(fout, header.version);
	write_be32(fout, header.size);
	for(int k = 0; k < MAX_XFILE_COUNT; k++)
	{
		write_be32(fout, header.blockSize[k]);
	}
#endif

	// if (fwrite(&header, sizeof(header), 1, fout) != 1) {
	//     perror("fwrite header");
	//     free(compressed_data);
	//     fclose(fout);
	//     return 1;
	// }

	if(fwrite(compressed_data, 1, compressed_size, fout) != compressed_size)
	{
		perror("fwrite data");
		free(compressed_data);
		fclose(fout);
		return 1;
	}

	free(compressed_data);
	fclose(fout);

	return 0;
}

#define MAX_LINE 1024

int parse_csv(const char *basename, const char *csv)
{
	FILE *fp = fopen(csv, "r");
	if(!fp)
	{
		perror("fopen");
		return 1;
	}

	char line[MAX_LINE];
	while(fgets(line, sizeof(line), fp))
	{
		// Remove trailing newline
		line[strcspn(line, "\r\n")] = '\0';

		// Tokenize by comma
		char *token = strtok(line, ",");
		if(token == NULL)
			continue;

		char *asset_type_str = token;
		char *asset_path = strtok(NULL, ",");

		if(asset_path != NULL)
		{
			printf("%s, %s\n", asset_type_str, asset_path);
			// printf("Field1: %s, Field2: %s\n", field1, field2);
			XAssetType type = asset_type_for_string(asset_type_str);
			if(type == -1)
			{
				fprintf(fp, "Unsupported asset type '%s'\n", asset_type_str);
				exit(1);
			}
			if(asset_handlers[type].load(type, basename, asset_path) > 0)
			{
				fprintf(stderr, "Error loading asset '%s'\n", asset_path);
				exit(1);
			}
		}
		else
		{
			printf("Field1: %s, Field2: (missing)\n", asset_type_str);
		}
	}

	fclose(fp);
	return 0;
}
int asset_count()
{
	int n = 0;
	for(Asset *it = assets; it; it = it->next)
	{
		n++;
	}
	return n;
}

#ifndef PROTO
// typedef struct
// {
// 	u32 dwLowDateTime;
// 	u32 dwHighDateTime;
// } FILETIME_LINUX;

// // Returns current time as FILETIME_LINUX
// void GetFileTime(FILETIME_LINUX *ft)
// {
// 	struct timespec ts;
// 	clock_gettime(CLOCK_REALTIME, &ts);

// 	// Convert Unix epoch (1970-01-01) to FILETIME epoch (1601-01-01)
// 	uint64_t filetime = ((uint64_t)(ts.tv_sec + 11644473600ULL) * 10000000ULL) + ((uint64_t)ts.tv_nsec / 100);

// 	ft->dwLowDateTime = (u32)(filetime & 0xFFFFFFFF);
// 	ft->dwHighDateTime = (u32)(filetime >> 32);
// }
#endif
int write_localize_entry(FILE *fp, const char *filename)
{
	char basename[512];
	get_basename(filename, basename);
	Entry *entries = NULL;
	strtoupper(basename);
	printf("basename: %s\n", basename);
	// int n = parse_loc_file("patch/english/localizedstrings/dlc.str", &entries);
	int n = parse_loc_file(filename, &entries);
	if(n < 0)
	{
		fprintf(stderr, "Failed to parse file.\n");
		return 1;
	}

	char key[512];
	for(int i = 0; i < n; i++)
	{
		unescape_string(entries[i].value);
		snprintf(key,sizeof(key),"%s_%s",basename,entries[i].key);
		printf("%s = %s\n", key, entries[i].value);

		u32 ptr = -1;
		write_be32(fp, ptr);
		u32 ptr2 = -1;
		write_be32(fp, ptr2);
		write_string(fp, entries[i].value);
		write_string(fp, key);
	}

	free(entries);

	return 0;
}

static void print_banner()
{
#ifndef PROTO
	const char *banner = "fastfile - compiler / linker v" APP_VERSION " for NX";
#else
	const char *banner = "fastfile - compiler / linker v" APP_VERSION " for WaW Prototype";
#endif
	// 	const char *banner =
	// "    ▄▄▄▄                                    ▄▄▄▄      ██     ▄▄▄▄               \n"
	// "   ██▀▀▀                         ██        ██▀▀▀      ▀▀     ▀▀██               \n"
	// " ███████    ▄█████▄  ▄▄█████▄  ███████   ███████    ████       ██       ▄████▄  \n"
	// "   ██       ▀ ▄▄▄██  ██▄▄▄▄ ▀    ██        ██         ██       ██      ██▄▄▄▄██ \n"
	// "   ██      ▄██▀▀▀██   ▀▀▀▀██▄    ██        ██         ██       ██      ██▀▀▀▀▀▀ \n"
	// "   ██      ██▄▄▄███  █▄▄▄▄▄██    ██▄▄▄     ██      ▄▄▄██▄▄▄    ██▄▄▄   ▀██▄▄▄▄█ \n"
	// "   ▀▀       ▀▀▀▀ ▀▀   ▀▀▀▀▀▀      ▀▀▀▀     ▀▀      ▀▀▀▀▀▀▀▀     ▀▀▀▀     ▀▀▀▀▀  \n"
	// "                                                                                \n"
	// "                           fastfile v"APP_VERSION" — compiler / linker for NX                  \n";

	printf("%s\n", banner);
}

int main(int argc, char **argv)
{
	print_banner();
	if(argc < 2)
	{
		printf("Usage: %s <mod.csv>\n", argv[0]);
		return 1;
	}
	const char *csv = argv[1];
	char basename[512];
	snprintf(basename, sizeof(basename), "%s", csv);
	for(char *p = basename; *p; p++)
	{
		if(*p == '.')
		{
			*p = 0;
			break;
		}
	}
	// printf("basename: %s\n", basename);
	if(parse_csv(basename, csv) > 0)
    {
        printf("Failed to read csv '%s'\n", csv);
		return 1;
    }
	char ffraw[512];
	snprintf(ffraw, sizeof(ffraw), "%s.ffraw", basename);
	FILE *fp = fopen(ffraw, "wb");
	if(!fp)
	{
		perror("Failed to open file");
		return 1;
	}
	// Create rawfile with ff name at end
	{
		Asset *a = new_xasset(ASSET_TYPE_RAWFILE, NULL, basename);
		RawFile *rf = calloc(1, sizeof(RawFile));
		a->header->rawfile = rf;
		rf->buffer = "";
		rf->len = 0;
		rf->name = basename;
	}
	int numassets = asset_count();
	XAssetList list = { 0 };
	list.scriptStringCount = COUNT_OF(scriptStrings);
	list.scriptStrings = COUNT_OF(scriptStrings) > 0 ? -1 : 0;
	// list.assetCount = COUNT_OF(assets);
	list.assetCount = numassets;
	list.assets = -1;
	// list.idk2 = -1;
	// list.idk = -1;
	write_xassetlist(fp, &list);
	// for(int i = 0; i < 100; i++)
	// 	write_be32(fp, -1);
	for(Asset *it = assets; it; it = it->next)
	{
		asset_handlers[it->type].serialize(fp, it->header);
	}
	// write_raw_file(fp, basename, "");
	fclose(fp);
	// assert(sizeof(XFile) == 28);
	char tmp[512];
	#ifndef PROTO
	snprintf(tmp, sizeof(tmp), "%s.ffm", basename);
	#else
	snprintf(tmp, sizeof(tmp), "%s.ff", basename);
	#endif
	write_fastfile(ffraw, tmp);
    printf("Written to '%s'\n", tmp);
	return 0;
}
