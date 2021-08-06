#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

size_t writer(char *data, size_t size, size_t nmemb, void *buffer_in);

struct json {
    char *response;
    size_t size;
};

int main(int argc, char *argv[])
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = 0;
    struct json jsdoc = {0};
    char *url = "https://api.weather.com/v1/location/28400%3A4%3AMX/forecast/hourly/24hour.json?units=m&language=en-US&apiKey=6532d6454b8aa370768e63d6ba5a832e";
    
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&jsdoc);
        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            const cJSON *forecasts;
            const cJSON *forecast;
            cJSON *weather_json = cJSON_Parse(jsdoc.response);
            if (weather_json != 0) {
                forecasts = cJSON_GetObjectItemCaseSensitive(weather_json, "forecasts");
                cJSON_ArrayForEach(forecast, forecasts) 
                {
                    const cJSON *dow = cJSON_GetObjectItemCaseSensitive(forecast, "dow");
                    const cJSON *date = cJSON_GetObjectItemCaseSensitive(forecast, "fcst_valid_local");
                    const cJSON *temp = cJSON_GetObjectItemCaseSensitive(forecast, "temp");
                    const cJSON *current_weather = cJSON_GetObjectItemCaseSensitive(forecast, "phrase_12char");
                    const cJSON *chance = cJSON_GetObjectItemCaseSensitive(forecast, "pop");
                    const cJSON *feels = cJSON_GetObjectItemCaseSensitive(forecast, "feels_like");
                    const cJSON *wind_dir = cJSON_GetObjectItemCaseSensitive(forecast, "wdir_cardinal");
                    const cJSON *wind_speed = cJSON_GetObjectItemCaseSensitive(forecast, "wspd");
                    const cJSON *humidity = cJSON_GetObjectItemCaseSensitive(forecast, "rh");
                    const cJSON *uv_index = cJSON_GetObjectItemCaseSensitive(forecast, "uv_index");
                    
                    printf("%s\n", dow->valuestring);

                    printf("%s\t%d°\t\t%s\t%d%%\n\n", date->valuestring,
                                                  temp->valueint,
                                                  current_weather->valuestring,
                                                  chance->valueint);

                    printf("Feels like\tWind\t\tHumidity\tUV Index\n");
                    printf("%d\t\t%s %d km/h\t%d%%\t\t%d of 10\n\n", feels->valueint,
                                           wind_dir->valuestring,
                                           wind_speed->valueint,
                                           humidity->valueint,
                                           uv_index->valueint);
                    printf("-------------------------------------------------------------------\n");
                }
            }
            cJSON_Delete(weather_json);
        } else {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();

    return 0;    
}

size_t writer(char *data, size_t size, size_t nmemb, void *buffer_in)
{
    size_t realsize = size * nmemb;
    struct json *js = (struct json *)buffer_in;

    char *ptr = realloc(js->response, js->size + realsize + 1);
    if (ptr == 0)
        return 0; //out of memory

    js->response = ptr;
    memcpy(&(js->response[js->size]), data, realsize);
    js->size += realsize;
    js->response[js->size] = 0;

    return realsize;
}