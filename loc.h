#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEY_LEN	  128
#define MAX_VALUE_LEN 512

typedef struct
{
	char key[MAX_KEY_LEN];
	char value[MAX_VALUE_LEN];
} Entry;

static void unescape_string(char *str)
{
	char *src = str;
	char *dst = str;
	while(*src)
	{
		if(*src == '\\')
		{
			src++;
			switch(*src)
			{
				case 'n': *dst++ = '\n'; break;
				case 't': *dst++ = '\t'; break;
				case 'r': *dst++ = '\r'; break;
				case '\\': *dst++ = '\\'; break;
				case '"': *dst++ = '"'; break;
				case '0': *dst++ = '\0'; break;
				default:
					/* Unknown escape: preserve backslash and character */
					*dst++ = '\\';
					if(*src)
						*dst++ = *src;
					break;
			}
		}
		else
		{
			*dst++ = *src;
		}
		if(*src)
			src++;
	}
	*dst = '\0';
}

int parse_loc_file(const char *filename, Entry **entries_out)
{
	FILE *fp = fopen(filename, "r");
	if(!fp)
		return -1;

	char line[1024];
	char current_key[MAX_KEY_LEN] = "";
	int capacity = 16;
	int count = 0;

	Entry *entries = malloc(sizeof(Entry) * capacity);
	if(!entries)
	{
		fclose(fp);
		return -1;
	}

	while(fgets(line, sizeof(line), fp))
	{
		char *p = line;
		while(*p == ' ' || *p == '\t')
			p++;

		if(strncmp(p, "REFERENCE", 9) == 0)
		{
			char *key_start = p + 9;
			while(*key_start == ' ' || *key_start == '\t')
				key_start++;
			char *end = key_start + strcspn(key_start, "\r\n");
			*end = '\0';
			strncpy(current_key, key_start, MAX_KEY_LEN - 1);
			current_key[MAX_KEY_LEN - 1] = '\0';
		}
		else if(strncmp(p, "LANG_ENGLISH", 12) == 0)
		{
			char *val_start = strchr(p, '"');
			if(!val_start)
				continue;
			val_start++;
			char *val_end = strrchr(val_start, '"');
			if(!val_end)
				continue;
			*val_end = '\0';

			if(count >= capacity)
			{
				capacity *= 2;
				Entry *tmp = realloc(entries, sizeof(Entry) * capacity);
				if(!tmp)
				{
					free(entries);
					fclose(fp);
					return -1;
				}
				entries = tmp;
			}

			strncpy(entries[count].key, current_key, MAX_KEY_LEN - 1);
			entries[count].key[MAX_KEY_LEN - 1] = '\0';
			strncpy(entries[count].value, val_start, MAX_VALUE_LEN - 1);
			entries[count].value[MAX_VALUE_LEN - 1] = '\0';
			count++;
		}
	}

	fclose(fp);
	*entries_out = entries;
	return count;
}

// int main(void) {
//     Entry *entries = NULL;
//     int n = parse_loc_file("input.txt", &entries);
//     if (n < 0) {
//         fprintf(stderr, "Failed to parse file.\n");
//         return 1;
//     }

//     for (int i = 0; i < n; i++)
//         printf("%s = %s\n", entries[i].key, entries[i].value);

//     free(entries);
//     return 0;
// }
