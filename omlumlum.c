#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

void http_request(const char* req)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, req);
        res = curl_easy_perform(curl);
        /* always cleanup */ 
        curl_easy_cleanup(curl);
    }
}


void omlumlum_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t gnome) {
    const int size = 8192;
    char req[8192] = "http://mpd/artnet.php?data[]=0&data[]=0&data[]=0&";

    char colors[128]; 
    snprintf(colors, 128, "data[]=%d&data[]=%d&data[]=%d&", r, g, b);
    
    char gnomecolors[16];
    snprintf(gnomecolors, 16, "data[]=%d&", gnome);

    int i, j;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 5; j++) {
	      strncat(req, colors, size);
  	  }
  	  
      strncat(req, gnomecolors, size);
  	  
    }

    http_request("http://mpd.lan/rainbow.php?enable=0");
    http_request(req);
}
