/* Private includes ----------------------------------------------------------*/
#include "ahrs.h"

/* Private variables ---------------------------------------------------------*/
/* Barometer Control */
struct bmp280_dev bmp;
struct bmp280_config conf;

/* IMU Scale Factors */
float gRes, aRes, mRes;

/* System Loop Time */
const float SampleTime = 0.01f;
const float SampleRate = 100.f;

/* Private functions ---------------------------------------------------------*/
EulerAngles ToEulerAngles(Quaternion q)
{
    EulerAngles angles;

    // roll (x-axis rotation)
    float sinr_cosp = +2.0f * (q.w * q.x + q.y * q.z);
    float cosr_cosp = +1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    angles.roll = atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    float sinp = +2.0f * (q.w * q.y - q.z * q.x);
    if (fabs(sinp) >= 1)
        angles.pitch = copysign(M_PI / 2.0f, sinp); // use 90 degrees if out of range
    else
        angles.pitch = asin(sinp);

    // yaw (z-axis rotation)
    float siny_cosp = +2.0f * (q.w * q.z + q.x * q.y);
    float cosy_cosp = +1.0f - 2.0f * (q.y * q.y + q.z * q.z);  
    angles.yaw = atan2(siny_cosp, cosy_cosp);

    return angles;
}

void AHRS_ALG_Execute(short acc[3], short gyr[3])
{
  float gx = gyr[0] * gRes * DEG_TO_RAD;
  float gy = gyr[1] * gRes * DEG_TO_RAD;
  float gz = gyr[2] * gRes * DEG_TO_RAD;
  MadgwickAHRSupdateIMU(gx, gy, gz, acc[0], acc[1], acc[2]);
  Quaternion quat; EulerAngles euler;
  quat.w = q0; quat.x = q1; quat.y = q2; quat.z = q3;
  euler = ToEulerAngles(quat);
  printf("%f %f %f\r\n", euler.roll * RAD_TO_DEG, euler.pitch * RAD_TO_DEG, euler.yaw * RAD_TO_DEG);
}

/* MotionFX Port ############################################################ */
char MotionFX_LoadMagCalFromNVM(unsigned short int datasize, unsigned int *data)
{
  return 1; /* FAILURE: Read from NVM not implemented. */
}

char MotionFX_SaveMagCalInNVM(unsigned short int datasize, unsigned int *data)
{
  return 1; /* FAILURE: Write to NVM not implemented. */
}

void ahrs_init(void)
{
  char lib_version[35], len;
  MotionFX_initialize();
  len = MotionFX_GetLibVersion(lib_version);
  printf("%.*s\r\n", len, lib_version);
  MFX_knobs_t iKnobs;
  MotionFX_getKnobs(&iKnobs);
	iKnobs.LMode = 0;
	iKnobs.modx = 1;
  MotionFX_setKnobs(&iKnobs);
  MotionFX_MagCal_init(SampleTime * 1000.0f, 1);
  MotionFX_enable_9X(MFX_ENGINE_ENABLE);
}

/* Bosch-Sensortec Port ##################################################### */
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
	HAL_I2C_Mem_Write(&hi2c3, i2c_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, length, 100);
	return 0;
}

int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length)
{
	HAL_I2C_Mem_Read(&hi2c3, i2c_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, length, 100);
	return 0;
}

void print_rslt(const char api_name[], int8_t rslt)
{
    if (rslt != BMP280_OK)
    {
        printf("%s\t", api_name);
        if (rslt == BMP280_E_NULL_PTR)
        {
            printf("Error [%d] : Null pointer error\r\n", rslt);
        }
        else if (rslt == BMP280_E_COMM_FAIL)
        {
            printf("Error [%d] : Bus communication failed\r\n", rslt);
        }
        else if (rslt == BMP280_E_IMPLAUS_TEMP)
        {
            printf("Error [%d] : Invalid Temperature\r\n", rslt);
        }
        else if (rslt == BMP280_E_DEV_NOT_FOUND)
        {
            printf("Error [%d] : Device not found\r\n", rslt);
        }
        else
        {
            /* For more error codes refer "*_defs.h" */
            printf("Error [%d] : Unknown error code\r\n", rslt);
        }
    }
}

void bmp_init(void)
{
	int8_t rslt;

	/* Map the delay function pointer with the function responsible for implementing the delay */
	bmp.delay_ms = HAL_Delay;

	/* Assign device I2C address based on the status of SDO pin (GND for PRIMARY(0x76) & VDD for SECONDARY(0x77)) */
	bmp.dev_id = BMP280_I2C_ADDR_PRIM;

	/* Select the interface mode as I2C */
	bmp.intf = BMP280_I2C_INTF;

	/* Map the I2C read & write function pointer with the functions responsible for I2C bus transfer */
	bmp.read = i2c_reg_read;
	bmp.write = i2c_reg_write;
	
	rslt = bmp280_init(&bmp);
	print_rslt(" bmp280_init status", rslt);

	/* Always read the current settings before writing, especially when
	 * all the configuration is not modified
	 */
	rslt = bmp280_get_config(&conf, &bmp);
	print_rslt(" bmp280_get_config status", rslt);

	/* configuring the temperature oversampling, filter coefficient and output data rate */
	/* Overwrite the desired settings */
	conf.filter = BMP280_FILTER_COEFF_2;

	/* Pressure oversampling set at 4x */
	conf.os_pres = BMP280_OS_4X;

	/* Setting the output data rate as 1HZ(1000ms) */
	conf.odr = BMP280_ODR_62_5_MS;
	rslt = bmp280_set_config(&conf, &bmp);
	print_rslt(" bmp280_set_config status", rslt);

	/* Always set the power mode after setting the configuration */
	rslt = bmp280_set_power_mode(BMP280_NORMAL_MODE, &bmp);
	print_rslt(" bmp280_set_power_mode status", rslt);
}

double read_pressure(void)
{
	int8_t rslt;
	struct bmp280_uncomp_data ucomp_data;
	double pres;
	
	/* Reading the raw data from sensor */
	rslt = bmp280_get_uncomp_data(&ucomp_data, &bmp);

	/* Getting the compensated pressure as floating point value */
	rslt = bmp280_get_comp_pres_double(&pres, ucomp_data.uncomp_press, &bmp);
	
	UNUSED(rslt);
	return pres;
}

/* Invensense Port ########################################################## */
static inline int run_self_test(void)
{
	int result;
	long gyro[3], accel[3];
#if defined (MPU6500) || defined (MPU9250)
	result = mpu_run_6500_self_test(gyro, accel, 0);
#elif defined (MPU6050) || defined (MPU9150)
	result = mpu_run_self_test(gyro, accel);
#endif
	if (result == 0x7)
	{
		for(int i = 0; i<3; i++)
		{
			gyro[i] = (long)(gyro[i] * 32.8f); //convert to +-1000dps
			accel[i] *= 2048.f; //convert to +-16G
			accel[i] = accel[i] >> 16;
			gyro[i] = (long)(gyro[i] >> 16);
		}
		mpu_set_gyro_bias_reg(gyro);
#if defined (MPU6500) || defined (MPU9250)
		mpu_set_accel_bias_6500_reg(accel);
#elif defined (MPU6050) || defined (MPU9150)
		mpu_set_accel_bias_6050_reg(accel);
#endif
		return 0;
	}
	else
	{
		return 1;
	}
}

void imu_init(void)
{
	mpu_init();
	mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
	mpu_set_gyro_fsr(2000);
	mpu_set_accel_fsr(16);
	mpu_set_sample_rate(SampleRate);
	mpu_set_compass_sample_rate(SampleRate);
	mpu_set_lpf(188);
	mpu_set_int_level(0);
	mpu_set_int_latched(1);
	run_self_test();	
	
	// sensor scale factors
	float temp1;
	unsigned short temp2;
	mpu_get_gyro_sens(&temp1);
	gRes = 1.0f / temp1;
	mpu_get_accel_sens(&temp2);
	aRes = 1.0f / (float)temp2;
	mpu_get_compass_fsr(&temp2);
	mRes = (float)temp2 / (float)INT16_MAX;
}
