#include "esp_camera.h"
#include <WiFi.h>
 
const char* ssid = "Verizon-SM-G930V-A5BE";
const char* password = "mtpg344#";
 
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
 
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
 
WiFiServer server(80);
bool connected = false;
WiFiClient live_client;
 
 
String index_html = "<meta charset=\"utf-8\"/>\n" \
                    "<style>\n" \
                    "#content {\n" \
                    "display: flex;\n" \
                    "flex-direction: column;\n" \
                    "justify-content: center;\n" \
                    "align-items: center;\n" \
                    "text-align: center;\n" \
                    "min-height: 100vh;}\n" \
                    "</style>\n" \
                    "<body bgcolor=\"#2986cc\"><div id=\"content\"><h2 style=\"color:#000000\">Autoquarium LIVE</h2><img src=\"video\"></div></body>";
 
void configCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
  config.pixel_format = PIXFORMAT_JPEG; //YUV422,GRAYSCALE,RGB565,JPEG
 
  config.frame_size = FRAMESIZE_QVGA; //QQVGA-QXGA Do not use sizes above QVGA when not JPEG
  config.jpeg_quality = 40; //0-63 lower number means higher quality
  config.fb_count = 1; //if more than one, i2s runs in continuous mode. Use only with JPEG
 
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
 
//continue sending camera frame
void liveCam(WiFiClient &client){
  size_t _jpg_buf_len;
  uint8_t * _jpg_buf;
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Frame buffer could not be acquired");
      return;
  }
  if(fb->format != PIXFORMAT_JPEG){
      bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
      if(!jpeg_converted){
          ESP_LOGE(TAG, "JPEG compression failed");
          esp_camera_fb_return(fb);
          return;
      }
  } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
  }
  client.print("--frame\n");
  client.print("Content-Type: image/jpeg\n\n");
  client.flush();
  client.write(_jpg_buf, _jpg_buf_len);
  client.flush();
  client.print("\n");
  esp_camera_fb_return(fb);
}
 
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  String IP = WiFi.localIP().toString();
  Serial.println("IP address: " + IP);
  index_html.replace("server_ip", IP);
  server.begin();
  configCamera();
}
    
void http_resp(){
  WiFiClient client = server.available();                           
  if (client.connected()) {     
      String req = "";
      while(client.available()){
        req += (char)client.read();
      }
      Serial.println("request " + req);
      int addr_start = req.indexOf("GET") + strlen("GET");
      int addr_end = req.indexOf("HTTP", addr_start);
      if (addr_start == -1 || addr_end == -1) {
          Serial.println("Invalid request " + req);
          return;
      }
      req = req.substring(addr_start, addr_end);
      req.trim();
      Serial.println("Request: " + req);
      client.flush();
  
      String s;
      if (req == "/")
      {
          s = "HTTP/1.1 200 OK\n";
          s += "Content-Type: text/html\n\n";
          s += index_html;
          s += "\n";
          client.print(s);
          client.stop();
      }
      else if (req == "/video")
      {
          live_client = client;
          live_client.print("HTTP/1.1 200 OK\n");
          live_client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\n\n");
          live_client.flush();
          connected = true;
      }
      else
      {
          s = "HTTP/1.1 404 Not Found\n\n";
          client.print(s);
          client.stop();
      }
    }       
}
 
void loop() {
  http_resp();
  if(connected == true){
    liveCam(live_client);
  }
}
