/**********************************************************************
  Filename    : Camera and SDcard
  Description : Use the onboard buttons to take photo and save them to an SD card.
  Auther      : www.freenove.com
  Modification: 2022/11/02
**********************************************************************/
#include "esp_camera.h"
#define CAMERA_MODEL_ESP32S3_EYE
#include "camera_pins.h"
#include "ws2812.h"
#include "sd_read_write.h"

#define BUTTON_PIN  0

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.setDebugOutput(false);
  Serial.println();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  ws2812Init();
  sdmmcInit();
  //removeDir(SD_MMC, "/camera");
  createDir(SD_MMC, "/camera");
  listDir(SD_MMC, "/camera", 0);
  if(cameraSetup()==1){
    ws2812SetColor(2);
  }
  else{
    ws2812SetColor(1);
  }
  delay(100);
}

void loop() {
  if(digitalRead(BUTTON_PIN)==LOW){
    delay(20);
    if(digitalRead(BUTTON_PIN)==LOW){
	  	ws2812SetColor(3);
      while(digitalRead(BUTTON_PIN)==LOW);

      camera_fb_t * fb = NULL;
      fb = esp_camera_fb_get();

      printf("sizeof size_t = %d\n", sizeof(size_t));
      printf("fb->buff = %p \n", fb->buf);
      printf("fb->format = %d \n", fb->format);
      printf("fb->height = %d \n", fb->height);
      printf("fb->len = %d \n", fb->len);   // length of buffer.
      printf("fb->time = %d \n", fb->timestamp);

      assert ( fb->height);

      int width = fb->len / fb->height;
      printf("width = %d (fb->len / fb->fb_height)\n", width);

      int one_element = fb->len / (width * fb->height);
      printf("one_element size = %d\n", one_element);
      if (fb != NULL) {
/*
        int photo_index = readFileNum(SD_MMC, "/camera");
        if(photo_index!=-1)
        {
          String path = "/camera/" + String(photo_index) +".jpg";
          writejpg(SD_MMC, path.c_str(), fb->buf, fb->len);
        }
*/
        esp_camera_fb_return(fb);
      }
      else {
        Serial.println("Camera capture failed.");
      }
      ws2812SetColor(2);
    }
  }
}

int cameraSetup(void) {
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

  config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
  //config.frame_size = FRAMESIZE_SXGA;   // 1280x1024
  //config.frame_size = FRAMESIZE_QVGA;   // 320x240

  //config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_GRAYSCALE;    //
  //config.pixel_format = PIXFORMAT_RGB555;    //
  config.pixel_format = PIXFORMAT_RGB565;    //
  //config.pixel_format = PIXFORMAT_RAW;    //
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  // for larger pre-allocated frame buffer.
  if(psramFound()){
    Serial.println ("PSRAM READY --------------");
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return 0;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  s->set_vflip(s, 1); // flip it back
  s->set_brightness(s, 1); // up the brightness just a bit
  s->set_saturation(s, 0); // lower the saturation

  static char crlf = 0;
  for (int reg = 0; reg < 0x200; reg++)
  {
    int value;
    value = s->get_reg(s, reg, 0xFF);

    if (value == 0) continue;

    if (!(crlf++ & 0x3)) Serial.println();
    printf("0x%03X = 0x%02X\n", reg, s->get_reg(s, reg, 0xFF));
  }


  Serial.println("Camera configuration complete!");
  return 1;
}
