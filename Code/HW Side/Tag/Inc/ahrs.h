/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AHRS_H
#define __AHRS_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "math.h"
#include "bmp280.h"
#include "inv_mpu.h"
#include "MadgwickAHRS.h"
#include "motion_fx.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    float roll, pitch, yaw;
} EulerAngles;

typedef struct
{
    float w, x, y, z;
} Quaternion;

/* Exported constants --------------------------------------------------------*/
#define M_PI 3.1415926f
#define DEG_TO_RAD 0.0174532f
#define RAD_TO_DEG 57.295779f

/* Exported variables --------------------------------------------------------*/
extern float gRes, aRes, mRes;
extern const float SampleTime;
extern const float SampleRate;

/* Exported functions prototypes ---------------------------------------------*/
void bmp_init(void);
void imu_init(void);
void ahrs_init(void);
double read_pressure(void);

#endif
