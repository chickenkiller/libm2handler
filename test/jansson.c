// Author: Xavier Lange
// Date: 9/28/2011

#include <assert.h>
#include <jansson.h>
#include <string.h>

int main(int argc, char** args){
	const char* test = "{\"guid\": \"1234ASDF6789ASDF\", \"a_num\":1234, \"double\": 3.14}";

	json_error_t jerr;
	json_t *jroot = json_loads(test,0,&jerr);
	assert(jroot != NULL);

	json_t *jguid = json_object_get(jroot,"guid");
	assert(jguid != NULL);
	assert(json_typeof(jguid) == JSON_STRING);
	const char* guid_temp = json_string_value(jguid);
	assert(strcmp(guid_temp,"1234ASDF6789ASDF") == 0);

	json_t *jnum = json_object_get(jroot,"a_num");
	assert(jnum != NULL);
	assert(json_typeof(jnum) == JSON_INTEGER);
	int num_temp = json_integer_value(jnum);
	assert(num_temp == 1234);

	json_t *jdub = json_object_get(jroot,"double");
	assert(jdub != NULL);
	assert(json_typeof(jdub) == JSON_REAL);
	double dub_temp = json_real_value(jdub);
	assert(dub_temp < 3.14+0.1 && dub_temp > 3.14-0.1);

	return 0;
}

