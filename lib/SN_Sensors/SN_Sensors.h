#ifndef SN_SENSORS_H
#define SN_SENSORS_H

#include <Arduino.h>
#include <SN_XR_Board_Types.h>

// Sensor feature flags - can be overridden in platformio.ini build_flags
#ifndef SN_USE_TEMPERATURE_SENSOR
#define SN_USE_TEMPERATURE_SENSOR 1  // Enable DS18B20 temperature sensor by default
#endif

#ifndef SN_USE_IMU
#define SN_USE_IMU 1  // Enable MPU6050 IMU by default
#endif

#ifndef SN_USE_MAGNETOMETER
#define SN_USE_MAGNETOMETER 1  // Enable QMC5883L magnetometer by default
#endif

#ifndef SN_USE_ADC
#define SN_USE_ADC 1  // Enable ADS1115 ADC by default
#endif

#define ADC_CHANN_BUS_CURRENT_MAIN 0
#define ADC_CHANN_BUS_VOLTAGE_MAIN 1
#define ADC_CHANN_BUS_VOLTAGE_5V 2
#define ADC_CHANN_BUS_VOLTAGE_3V3 3

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

bool SN_Sensors_ADCInit();
bool SN_Sensors_DS18B20Init();
int16_t SN_Sensors_ADCReadChannel(uint8_t channel);
float SN_Sensors_ADCGetParameterValue(uint8_t channel);
float SN_Sensors_GetBatteryTemperature();
void SN_Sensors_Init();
void SN_Sensors_MPU_Init();
void SN_Sensors_MAG_Init();




#if SN_USE_IMU == 1
struct s_SN_MPU_Sensor {
    float A_angle = 0;
    float B_angle = 0;
    float C_angle = 0;

    float X_acceleration = 0;
    float Y_acceleration = 0;
    float Z_acceleration = 0;

    float X_gravity = 0;
    float Y_gravity = 0;
    float Z_gravity = 0;

    float W_quaternion = 0;
    float X_quaternion = 0;
    float Y_quaternion = 0;
    float Z_quaternion = 0;

    float temperature = 0;
};

typedef struct s_SN_MPU_Sensor SN_MPU_Sensor;

// External declaration of MPU sensor data
extern SN_MPU_Sensor mpu_sensor;

// Function prototypes
void read_MPU();
void SN_SetMPUOrientation(int orientation);
int SN_GetMPUOrientation();
void SN_ClearMPUCalibrationData();

#endif // SN_USE_IMU

#if SN_USE_MAGNETOMETER == 1
struct s_SN_Magnetometer_Sensor {
    int mag_x = 0;
    int mag_y = 0;
    int mag_z = 0;
    int azimuth = 0;
    float heading_degrees = 0.0;
    char direction[3] = "N";
};

typedef struct s_SN_Magnetometer_Sensor SN_Magnetometer_Sensor;

// External declaration of magnetometer sensor data
extern SN_Magnetometer_Sensor mag_sensor;

// Function prototypes
void read_MAG();
float computeTiltCompensatedHeading(float mag_x, float mag_y, float mag_z, 
                                    float pitch_rad, float roll_rad);
void SN_SetMagnetometerCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max);
void SN_SetMagnetometerSmoothing(uint8_t steps, bool advanced);

#endif // SN_USE_MAGNETOMETER

#endif // SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

#endif // SN_SENSORS_H