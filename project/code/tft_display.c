#include "tft_display.h"

int geted_lg = 0;

void TFT_INIT(void)
{
	tft180_set_dir(TFT180_CROSSWISE_180);        
	tft180_init();
}

void TFT_SHOW_CAR_SPEED(void)
{
    tft180_show_string(8,0,"MOTOR1:");
    tft180_show_float(64,0,cars_mot[0].CAR_MOT_V,5,2);

    tft180_show_string(8,32,"MOTOR2:");
    tft180_show_float(64,32,cars_mot[1].CAR_MOT_V,5,2);

    tft180_show_string(8,64,"MOTOR3:");
    tft180_show_float(64,64,cars_mot[2].CAR_MOT_V,5,2);

    tft180_show_string(8,96,"MOTOR4:");
    tft180_show_float(64,96,cars_mot[3].CAR_MOT_V,5,2);
}

