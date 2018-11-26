#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include "library/ini.h"
#include "library/argparse.h"
#include <cjson/cJSON.h>

static const char *const usages[] = {
    "fbcm [options] [[--] args]",
    "fbcm [options]",
    NULL,
};

typedef struct
{
    char *apiserver;
    char *device;
} configuration;

typedef struct
{
    const char* title;
    const char* message;
} notification_text;

typedef struct
{
    const char* to;
    long time_to_live;
    const char* priority;
    notification_text* text;
} notification;

typedef struct
{
    char *multicast_id;
    int success;
    int failure;
    int canonical_ids;
} response;

static int handler(void* user, const char* section, const char* name, const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("general", "apiserver")) {
        pconfig->apiserver = strdup(value);
    } else if (MATCH("general", "device")) {
        pconfig->device = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/*
 * Check if a file exist using stat() function
 * return 1 if the file exist otherwise return 0
 */
int file_exists(const char* filename){
    struct stat buffer;
    int exist = stat(filename,&buffer);
    if(exist == 0)
        return 1;
    else // -1
        return 0;
}

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);

  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int main(int argc, const char **argv)
{
    int debug_mode = 0;
    long ttl = 86400;
	char *priority = "high";
    char *title = NULL;
    char *message = NULL;
    char *device = NULL;
    char *apiserver = NULL;
    char *configuration_file = NULL;
    configuration config;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_STRING('t', "title", &title, "Notification Title", NULL, 0, 0),
        OPT_STRING('m', "message", &message, "Notification Message", NULL, 0, 0),
        OPT_STRING('s', "tokendevice", &device, "Token Device", NULL, 0, 0),
        OPT_STRING('k', "serverkey", &apiserver, "Api Server Key", NULL, 0, 0),
        OPT_INTEGER('d', "debug", &debug_mode, "Debug Mode", NULL, 0, 0),
        OPT_STRING('f', "configuration", &configuration_file, "Configuration File", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
    argc = argparse_parse(&argparse, argc, argv);

    if (title == NULL) {
        printf("Title is empty\n");
        return 1;
    }

    if (message == NULL) {
        printf("Message is empty\n");
        return 1;
    }

    if (configuration_file != NULL) {
        if (!file_exists(configuration_file)) {
            printf("Configuration file '%s' do not exist!\n", configuration_file);
            return 1;
        }

        if (ini_parse(configuration_file, handler, &config) < 0) {
            printf("Can't load '%s'\n",configuration_file);
            return 1;
        }

        device = config.device;
        apiserver = config.apiserver;
    }

    if (device == NULL) {
        printf("Device Token is empty\n");
        return 1;
    }

    if (apiserver == NULL) {
        printf("Api Server Key is empty\n");
        return 1;
    }

    if (debug_mode) {
        printf("API Server: %s\n", apiserver);
        printf("Device Token: %s\n", device);
        printf("Notification Title: %s\n", title);
        printf("Notification Message: %s\n", message);
    }

    if (argc != 0) {
        printf("argc: %d\n", argc);
        int i;
        for (i = 0; i < argc; i++) {
            printf("argv[%d]: %s\n", i, *(argv + i));
        }
    }

    /* Notification Message */
    notification_text notification_text;
    notification_text.title = title;
    notification_text.message = message;

    int l = strlen(message);

    if (l > 4048) {
        printf("Message exceed 4048 bytes'\n");
        return 1;
    }
    
    notification notification;
    notification.to = device;
    notification.time_to_live = ttl;
	notification.priority = priority;
    
    CURL *curl;
    CURLcode res;
    
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk.size = 0;    /* no data at this point */

    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://fcm.googleapis.com/fcm/send");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        /* Header */
        struct curl_slist *headers = NULL;
        char *header_authorization = concat("Authorization:key=",apiserver);
        char *header_contenttype = "Content-Type: application/json";
        headers = curl_slist_append(headers, header_authorization);
        headers = curl_slist_append(headers, header_contenttype);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* Now specify the POST data */ 
        char data[32516];
        sprintf(data,"{\"to\":\"%s\",\"time_to_live\":%d,\"priority\":\"%s\",\"data\":{\"text\":{\"title\":\"%s\",\"message\":\"%s\"}}}\n",notification.to, notification.time_to_live, notification.priority, notification_text.title, notification_text.message);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        /* send all data to this function  */ 
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        /* Debug */   
        if (debug_mode)    
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /* Perform the request, res will get the return code */ 
        res = curl_easy_perform(curl);

        /* Check for errors */ 
        if  (res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            if (debug_mode)  
                printf("%d\n", response_code);
                printf("%s\n", chunk.memory);
    
            if (response_code == 200) {
                cJSON *response_json = cJSON_Parse(chunk.memory);

                if (response_json == NULL) {
                    const char *error_ptr = cJSON_GetErrorPtr();
                    if (error_ptr != NULL) {
                        fprintf(stderr, "Error before: %s\n", error_ptr);    
                    }

                    cJSON_Delete(response_json);
                }        

                int success = cJSON_GetObjectItem(response_json, "success")->valueint;
                printf("%d\n", success);    
                cJSON_Delete(response_json);
            }
        } else {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        /* always cleanup */ 
        curl_easy_cleanup(curl);  
    }

    return 0;
}
