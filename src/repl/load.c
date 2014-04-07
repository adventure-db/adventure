#include "load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errno.h"

#include <lib/jsmn/jsmn.h>
#include <src/util/fs.h>

int streamJSON(char *json)
{
	int resultCode;
	printf("JSON to parse:\n%s\n", json);

	jsmn_parser p;
	jsmntok_t tokens[256];
	jsmn_init(&p);
	resultCode = jsmn_parse(&p, json, strlen(json), tokens, 256);
	printf("Result code: %d\n", resultCode);
	printf("toksuper: %d\n", p.toksuper);

	int len = p.toksuper < 0? resultCode : p.toksuper;
	for(int i = 0; i < len; i++) {
		jsmntok_t key = tokens[i];
		unsigned int keyLen = key.end - key.start;
		if(key.start < 0 || key.end < 0) continue;
		char keyString[keyLen + 1];    
		memcpy(keyString, &json[key.start], keyLen);
		//printf("(%d, %d, %d)\n", tokens[i].start, tokens[i].end, tokens[i].type);
		keyString[keyLen] = '\0';
		printf("Key: %s\n", keyString);
	}

	return resultCode;
}

int load(const char *filename)
{
	errno = 0;
	struct fs_map *file = fs_map(filename);
	char *buf = file->data;
	if(!buf) goto error;
	
	int code = streamJSON(buf);
	return code;

error:
	printf("Error opening file: %s\n", strerror(errno));
	return 1;
}
