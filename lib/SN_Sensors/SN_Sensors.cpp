#include <SN_Sensors.h>
#include <SN_Logger.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32


#define ONE_WIRE_BUS_PIN 4 // GPIO where the DS18B20 is connected to

#define ADC_MAX_VOLTAGE 3.31

#define MIN_MAIN_BUS_CURRENT 0.0
#define MAX_MAIN_BUS_CURRENT 5.0

#define MIN_MAIN_BUS_VOLTAGE 10.0
#define MAX_MAIN_BUS_VOLTAGE 12.6

#define MIN_BATTERY_TEMPERATURE 0.0
#define MAX_BATTERY_TEMPERATURE 50.0

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MPU6050.h>
#include "MPUFilter/MPUFilter.h"
#include <Preferences.h>

Preferences preferences;

Adafruit_ADS1115 obc_adc;

// Initialize DS18B20 temperature sensor
const int oneWireBus = ONE_WIRE_BUS_PIN ;     // GPIO where the DS18B20 is connected to
OneWire oneWire(oneWireBus);    // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature batt_temp_sensor(&oneWire);   // Pass our oneWire reference to Dallas Temperature sensor 

bool ADC_NotInitialized = true; // Flag to check if ADC is initialized
bool DS18B20_NotInitialized = true; // Flag to check if DS18B20 is initialized


Adafruit_MPU6050 mpu;
MPUFilter mpuFilter;

struct Vector3D
{
    double x;
    double y;
    double z;
};

SN_MPU_Sensor mpu_sensor; // Structure to hold MPU sensor data

// MPU Normalization variables
Vector3D gravity;               // used to store the gravity vector, low-pass filter and normalization
const float gravityAlpha = 0.9; // Complementary filter constant for isolating gravity

// MPU Calibration variables
bool doMPUCalibration = false;
int numberOfSamples = 10000;
int calibrationSamplesCount = 0;
// Accelerometer sums
double axSum = 0.0, aySum = 0.0, azSum = 0.0;
// Gyroscope sums
double gxSum = 0.0, gySum = 0.0, gzSum = 0.0;
// End of MPU Calibration variables

void SN_Sensors_ADCInit()
{   
    uint8_t adc_init_try = 0;

    if (!obc_adc.begin()) {
        logMessage(false, "SN_Sensors_ADCInit()", "Failed to initialize ADC ADS1115.");
        while (adc_init_try < 50)
        {
            delay(50);
            adc_init_try++;
        }
        ADC_NotInitialized = true; // Set flag to true if ADC initialization fails
    }
    ADC_NotInitialized = false; // Set flag to false if ADC initialization succeeds

    // if(!batt_temp_sensor.begin()) {
    //     logMessage(false, "SN_Sensors_ADCInit()", "Failed to initialize DS18B20 temperature sensor.");
    //     while (adc_init_try < 50)
    //     {
    //         delay(50);
    //         adc_init_try++;
    //     }
    //     DS18B20_NotInitialized = true; // Set flag to true if DS18B20 initialization fails
    // }
}

int16_t SN_Sensors_ADCReadChannel(uint8_t channel) {
    int16_t adc_val = 0;

    adc_val = obc_adc.readADC_SingleEnded(channel);
    
    if (adc_val > 17670){ //the ADS1115 in default mode measures 0.0001875 V per count -> 17670 equals 3.31 V; there shouldn't be any value higher than that -> this threshold is used to cap the input
        adc_val = 17670;
    }
    else if(adc_val < 0){ //cap at 0 to prevent issues caused by negative reads (noise)
        adc_val = 0;
    }
    return adc_val;
}

float SN_Sensors_ADCGetParameterValue(uint8_t channel) {

    int16_t adc_raw_val = SN_Sensors_ADCReadChannel(channel);

    float adc_read_volts = obc_adc.computeVolts(adc_raw_val);

    float adc_param_val = 0;

    switch(channel) {
        case ADC_CHANN_MAIN_BUS_CURRENT:

            adc_param_val = map(adc_read_volts, 0, ADC_MAX_VOLTAGE, MIN_MAIN_BUS_CURRENT, MAX_MAIN_BUS_CURRENT);

        break;

        case ADC_CHANN_MAIN_BUS_VOLTAGE:

            adc_param_val = map(adc_read_volts, 0, ADC_MAX_VOLTAGE, MIN_MAIN_BUS_VOLTAGE, MAX_MAIN_BUS_VOLTAGE);

        break;

        case ADC_CHANN_BATTERY_TEMPERATURE:


        break;

        case ADC_CHANN_AUXILIARY:

        break;

        default:
            logMessage(false, "SN_Sensors_ADCGetParameterValue()", "Invalid ADC channel: %d", channel);
            adc_param_val = 0;
    }

    return adc_param_val;
}

float SN_Sensors_GetBatteryTemperature() {
    float battery_temp = 0.0;

    // Request temperature from the DS18B20 sensor
    batt_temp_sensor.requestTemperatures();
    
    // Read the temperature in Celsius
    battery_temp = batt_temp_sensor.getTempCByIndex(0);

    if (battery_temp == DEVICE_DISCONNECTED_C) {
        logMessage(false, "SN_Sensors_GetBatteryTemperature()", "Failed to read temperature from DS18B20 sensor.");
        return -1; // Return an error value
    }

    return battery_temp;
}


void SN_Sensors_Init() {
    SN_Sensors_ADCInit();

    if (!ADC_NotInitialized) {
        logMessage(true, "SN_Sensors_Init", "ADC Initialized Successfully");
    } else {
        logMessage(false, "SN_Sensors_Init", "ADC Initialization Failed");
    }

    // Initialize DS18B20 sensor
    batt_temp_sensor.begin();
    
    if (!DS18B20_NotInitialized) {
        logMessage(true, "SN_Sensors_Init", "DS18B20 Initialized Successfully");
    } else {
        logMessage(false, "SN_Sensors_Init", "DS18B20 Initialization Failed");
    }
}


void SN_Sensors_MPU_Init() {
    if (!mpu.begin()) {
        logMessage(false, "SN_Sensors_MPU_Init", "Failed to initialize MPU6050.");
        return;
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    // Initialize MPUFilter
    mpuFilter.begin(5);

    logMessage(true, "SN_Sensors_MPU_Init", "MPU6050 Initialized Successfully");
}

// This function is called from the read_MPU() function each time
void mpuCalibrationEventHandler(const sensors_event_t &accel, const sensors_event_t &gyro)
{
    if (doMPUCalibration)
    {
        if (calibrationSamplesCount < numberOfSamples)
        {
            axSum += accel.acceleration.x;
            aySum += accel.acceleration.y;
            azSum += accel.acceleration.z;
            gxSum += gyro.gyro.x;
            gySum += gyro.gyro.y;
            gzSum += gyro.gyro.z;
            calibrationSamplesCount++;
        }
        else
        {
            float acc_x = axSum / numberOfSamples;
            float acc_y = aySum / numberOfSamples;
            float acc_z = azSum / numberOfSamples;
            float gyro_x = gxSum / numberOfSamples;
            float gyro_y = gySum / numberOfSamples;
            float gyro_z = gzSum / numberOfSamples;

            float expectedGravity = -9.81;
            int orientation = preferences.getInt("orientation", 0);

            if (orientation == 1)
            {
                acc_x -= expectedGravity;
            }
            else if (orientation == 0)
            {
                acc_z -= expectedGravity;
            }

            preferences.putFloat("acc_x", acc_x);
            preferences.putFloat("acc_y", acc_y);
            preferences.putFloat("acc_z", acc_z);
            preferences.putFloat("gyro_x", gyro_x);
            preferences.putFloat("gyro_y", gyro_y);
            preferences.putFloat("gyro_z", gyro_z);

            doMPUCalibration = false;
            calibrationSamplesCount = 0;
            axSum = 0.0;
            aySum = 0.0;
            azSum = 0.0;
            gxSum = 0.0;
            gySum = 0.0;
            gzSum = 0.0;
        }
    }
}

void read_MPU()
{
    // Get accelerometer, gyroscope, and temperature events
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    // Check if calibration is enabled - Non-blocking calibration mode
    mpuCalibrationEventHandler(accel, gyro);

    // Read with Offsets ===========================================================
    float accelerationX = accel.acceleration.x - preferences.getFloat("acc_x", 0.0f);
    float accelerationY = accel.acceleration.y - preferences.getFloat("acc_y", 0.0f);
    float accelerationZ = accel.acceleration.z - preferences.getFloat("acc_z", 0.0f);
    float gyroX = gyro.gyro.x - preferences.getFloat("gyro_x", 0.0f);
    float gyroY = gyro.gyro.y - preferences.getFloat("gyro_y", 0.0f);
    float gyroZ = gyro.gyro.z - preferences.getFloat("gyro_z", 0.0f);
    // End of Read with offsets ====================================================

    // Apply low-pass filter to isolate gravity from accelerometer data
    gravity.x = gravityAlpha * gravity.x + (1 - gravityAlpha) * accelerationX;
    gravity.y = gravityAlpha * gravity.y + (1 - gravityAlpha) * accelerationY;
    gravity.z = gravityAlpha * gravity.z + (1 - gravityAlpha) * accelerationZ;

    // Subtract gravity from the accelerometer data to get linear acceleration
    float linearAccX = accelerationX - gravity.x;
    float linearAccY = accelerationY - gravity.y;
    float linearAccZ = accelerationZ - gravity.z;

    // Normalize the gravity vector to ensure it has a magnitude of ~9.81
    float magnitude = sqrt(gravity.x * gravity.x + gravity.y * gravity.y + gravity.z * gravity.z);

    // Check if the magnitude deviates significantly from 9.81, and normalize if necessary
    if (abs(magnitude - 9.81) > 0.1)
    { // Allow for a small tolerance
        float correctionFactor = 9.81 / magnitude;
        gravity.x *= correctionFactor;
        gravity.y *= correctionFactor;
        gravity.z *= correctionFactor;
    }

    float gyroScale = 3.800;

    int orientation = preferences.getInt("orientation", 0);

    if (orientation == 1) // vertical
    {
        mpuFilter.updateIMU(-gyroY * gyroScale, -gyroZ * gyroScale, -gyroX * gyroScale, -accelerationY, -accelerationZ, -accelerationX);
    }
    else if (orientation == 0) // horizontal
    {
        mpuFilter.updateIMU(gyroX * gyroScale, gyroY * gyroScale, -gyroZ * gyroScale, accelerationX, accelerationY, -accelerationZ);
    }

    float q0 = mpuFilter.getQ0();
    float q1 = mpuFilter.getQ1();
    float q2 = mpuFilter.getQ2();
    float q3 = mpuFilter.getQ3();

    float roll = mpuFilter.getRoll();
    float pitch = mpuFilter.getPitch();
    float yaw = mpuFilter.getYaw();

    if (orientation == 1)
    {
        // ACCELERATION OUTPUT PD-DATA
        mpu_sensor.X_acceleration = linearAccY;
        mpu_sensor.Y_acceleration = linearAccZ;
        mpu_sensor.Z_acceleration = linearAccX;

        // GRAVITY OUTPUT PD-DATA
        mpu_sensor.X_gravity = gravity.y;
        mpu_sensor.Y_gravity = -gravity.z;
        mpu_sensor.Z_gravity = gravity.x;

        // ANGLE OUTPUT PD-DATA
        mpu_sensor.A_angle = yaw;
        mpu_sensor.B_angle = roll;
        mpu_sensor.C_angle = pitch;
    }
    else if (orientation == 0)
    {
        // ACCELERATION OUTPUT PD-DATA
        mpu_sensor.X_acceleration = linearAccY;
        mpu_sensor.Y_acceleration = linearAccX;
        mpu_sensor.Z_acceleration = linearAccZ;

        // GRAVITY OUTPUT PD-DATA
        mpu_sensor.X_gravity = gravity.y;
        mpu_sensor.Y_gravity = gravity.x;
        mpu_sensor.Z_gravity = gravity.z;

        // ANGLE OUTPUT PD-DATA
        mpu_sensor.A_angle = yaw;
        mpu_sensor.B_angle = -pitch;
        mpu_sensor.C_angle = roll;
    }

    // QUATERNION OUTPUT PD-DATA
    mpu_sensor.W_quaternion = q0;
    mpu_sensor.X_quaternion = q1;
    mpu_sensor.Y_quaternion = q2;
    mpu_sensor.Z_quaternion = q3;

    // TEMPERATURE OUTPUT PD-DATA
    mpu_sensor.temperature = temp.temperature / 340.00 + 36.53;
}

void SN_SetMPUOrientation(int orientation)
{
    if (orientation == 0 || orientation == 1)
    {
        preferences.putInt("orientation", orientation);
        logMessage(true, "SN_SetMPUOrientation", "MPU orientation set to: %d", orientation);
    }
    else
    {
        logMessage(false, "SN_SetMPUOrientation", "Invalid orientation. Use 0 (horizontal) or 1 (vertical).");
    }
}

int SN_GetMPUOrientation()
{
    return preferences.getInt("orientation", 0); // Default to horizontal
}

void SN_ClearMPUCalibrationData()
{
    preferences.remove("acc_x");
    preferences.remove("acc_y");
    preferences.remove("acc_z");
    preferences.remove("gyro_x");
    preferences.remove("gyro_y");
    preferences.remove("gyro_z");
    preferences.remove("orientation");

    logMessage(true, "SN_ClearMPUCalibrationData", "All MPU calibration preferences cleared.");
}

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32






#endif