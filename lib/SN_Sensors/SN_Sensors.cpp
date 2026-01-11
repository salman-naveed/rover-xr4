#include <SN_Sensors.h>
#include <SN_Logger.h>

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32


#define ONE_WIRE_BUS_PIN 4 // GPIO where the DS18B20 is connected to
#define MIN_BATTERY_TEMPERATURE 0.0
#define MAX_BATTERY_TEMPERATURE 50.0

#define ADC_MAX_VOLTAGE 3.31
#define MIN_BUS_CURRENT_MAIN 0.0
#define MAX_BUS_CURRENT_MAIN 5.0
#define MIN_BUS_VOLTAGE_MAIN 10.0
#define MAX_BUS_VOLTAGE_MAIN 12.6

// ACS712 Current Sensor Configuration (30A Model)
// The ACS712-30A outputs 2.5V at 0A (zero-point offset)
// Sensitivity: 66 mV/A (0.066 V/A)
// Full range: 0.5V (-30A) to 4.5V (+30A)
// 
// IMPORTANT: ADC limited to 3.31V, so measurable range is:
//   - Min: 0V → (0 - 2.721) / 0.066 = -41.2A (clamped to sensor limit)
//   - Max: 3.31V → (3.31 - 2.721) / 0.066 = +8.92A
//   - Effective range: -30A to +8A (ADC-limited on positive side)
//
// NOTE: Zero point calibrated to 2.721V (measured for this specific ACS712 unit)
//       Nominal spec is 2.5V, but individual units vary ±0.1-0.2V
#define ACS712_ZERO_POINT 2.7212         // Voltage output at 0A (CALIBRATED)
#define ACS712_SENSITIVITY 0.066        // 66 mV/A for 30A model

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_MPU6050.h>
#include "MPUFilter/MPUFilter.h"
#include <QMC5883LCompass.h>
#include <Preferences.h>

Preferences preferences;

// Initialize ADC (ADS1115)
Adafruit_ADS1115 obc_adc;
bool ADC_NotInitialized = true; // Flag to check if ADC is initialized

#if SN_USE_TEMPERATURE_SENSOR == 1
// Initialize DS18B20 temperature sensor
const int oneWireBus = ONE_WIRE_BUS_PIN ;     // GPIO where the DS18B20 is connected to
OneWire oneWire(oneWireBus);    // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature batt_temp_sensor(&oneWire);   // Pass our oneWire reference to Dallas Temperature sensor 
bool DS18B20_NotInitialized = true; // Flag to check if DS18B20 is initialized
#endif // SN_USE_TEMPERATURE_SENSOR

#if SN_USE_IMU == 1
// Initialize MPU6050
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

#endif // SN_USE_IMU

#if SN_USE_MAGNETOMETER == 1
// Initialize QMC5883L Magnetometer
QMC5883LCompass magnetometer;

SN_Magnetometer_Sensor mag_sensor; // Structure to hold magnetometer data

#endif // SN_USE_MAGNETOMETER

bool SN_Sensors_ADCInit()
{   
    if (!obc_adc.begin()) {
        logMessage(false, "SN_Sensors_ADCInit()", "Failed to initialize ADC ADS1115.");
        return true;
    } else {
        return false; // Set flag to false if ADC initialization succeeds
    }
}

#if SN_USE_TEMPERATURE_SENSOR == 1
bool SN_Sensors_DS18B20Init() {

    uint8_t ds18b20_address[8];
    batt_temp_sensor.begin();
    batt_temp_sensor.getAddress(ds18b20_address, 0); // Get address of the first sensor
    if(!batt_temp_sensor.isConnected(ds18b20_address)){
        logMessage(false, "SN_Sensors_DS18B20Init()", "Failed to initialize DS18B20 temperature sensor.");
        return true;
    } else {
        return false;
    }
}
#endif // SN_USE_TEMPERATURE_SENSOR

void SN_Sensors_Init() {
    #if SN_USE_ADC == 1
    if (SN_Sensors_ADCInit()) {
        logMessage(true, "SN_Sensors_Init", "ADC Initialized Successfully");
    } else {
        logMessage(false, "SN_Sensors_Init", "ADC Initialization Failed");
    }
    #endif // SN_USE_ADC

    #if SN_USE_TEMPERATURE_SENSOR == 1
    // Initialize DS18B20 sensor
    if(SN_Sensors_DS18B20Init()) {
        logMessage(false, "SN_Sensors_Init", "DS18B20 Temperature Sensor Initialization Failed");
    } else {
        logMessage(true, "SN_Sensors_Init", "DS18B20 Temperature Sensor Initialized Successfully");
    }
    #endif // SN_USE_TEMPERATURE_SENSOR

    #if SN_USE_IMU == 1
    SN_Sensors_MPU_Init();
    #endif // SN_USE_IMU
    
    #if SN_USE_MAGNETOMETER == 1
    SN_Sensors_MAG_Init();
    #endif // SN_USE_MAGNETOMETER
}

#if SN_USE_IMU == 1
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
#endif // SN_USE_IMU

#if SN_USE_MAGNETOMETER == 1
void SN_Sensors_MAG_Init() {
    magnetometer.init();
    
    // Configure magnetometer
    // Set mode, data rate, scale, and over-sampling
    magnetometer.setMode(0x01, 0x0C, 0x10, 0X00);
    
    logMessage(true, "SN_Sensors_MAG_Init", "QMC5883L Magnetometer Initialized Successfully");
}
#endif // SN_USE_MAGNETOMETER


int16_t SN_Sensors_ADCReadChannel(uint8_t channel) {
#if SN_USE_ADC == 1
    int16_t adc_val = 0;

    adc_val = obc_adc.readADC_SingleEnded(channel);
    
    if (adc_val > 17670){ //the ADS1115 in default mode measures 0.0001875 V per count -> 17670 equals 3.31 V; there shouldn't be any value higher than that -> this threshold is used to cap the input
        adc_val = 17670;
    }
    else if(adc_val < 0){ //cap at 0 to prevent issues caused by negative reads (noise)
        adc_val = 0;
    }
    return adc_val;
#else
    return 0;
#endif // SN_USE_ADC
}

float SN_Sensors_ADCGetParameterValue(uint8_t channel) {
#if SN_USE_ADC == 1
    int16_t adc_raw_val = SN_Sensors_ADCReadChannel(channel);

    float adc_read_volts = obc_adc.computeVolts(adc_raw_val);

    float adc_param_val = 0;

    switch(channel) {
        case ADC_CHANN_BUS_CURRENT_MAIN:
            // ACS712-30A Hall Effect Current Sensor Conversion
            // Formula: Current (A) = (Vout - Vzero) / Sensitivity
            // Vzero = 2.5V (output voltage at 0A)
            // Sensitivity = 66mV/A for 30A model (0.066 V/A)
            // 
            // Example calculations:
            //   Vout = 2.5V → (2.5 - 2.5) / 0.066 = 0A
            //   Vout = 3.0V → (3.0 - 2.5) / 0.066 = 7.58A
            //   Vout = 3.31V → (3.31 - 2.5) / 0.066 = 12.27A (ADC max)
            //   Vout = 2.0V → (2.0 - 2.5) / 0.066 = -7.58A
            //
            // Note: ADC range (0-3.31V) limits measurement to approximately -38A to +12A
            //       Practical range: -30A to +12A (sensor saturates at ±30A)
            
            // Calculate current 
            adc_param_val = ((adc_read_volts - ACS712_ZERO_POINT) / ACS712_SENSITIVITY);
            
            // Clamp to sensor physical limits (ACS712-30A can't measure beyond ±30A)
            if (adc_param_val > 30.0) {
                adc_param_val = 30.0;
            } else if (adc_param_val < -30.0) {
                adc_param_val = -30.0;
            }
            
            // Dead zone to eliminate noise at zero current (±100mA threshold)
            if (abs(adc_param_val) < 0.1) {
                adc_param_val = 0.0;
            }
            break;

        case ADC_CHANN_BUS_VOLTAGE_MAIN:
            // Main bus voltage measurement with voltage divider
            // Voltage divider: R1=30kΩ, R2=7.5kΩ (designed for 0-25V input)
            // Divider ratio: R2/(R1+R2) = 7.5/37.5 = 0.2 (1:5 scaling)
            // Formula: Actual_Voltage = ADC_Voltage / 0.2 = ADC_Voltage × 5
            //
            // Example calculations:
            //   12.0V battery → 2.4V at ADC → displayed as 12.0V
            //   11.1V battery → 2.22V at ADC → displayed as 11.1V
            //   3.31V at ADC (max) → 16.55V actual (within divider range)
            //
            // Max measurable voltage: 3.31V × 5 = 16.55V
            adc_param_val = adc_read_volts * 5.0;
            break;
 
        case ADC_CHANN_BUS_VOLTAGE_5V:
            // 5V rail monitoring with voltage divider (if sensor installed)
            // Voltage divider: R1=30kΩ, R2=7.5kΩ (same as main bus)
            // Divider ratio: R2/(R1+R2) = 7.5/37.5 = 0.2 (1:5 scaling)
            // Formula: Actual_Voltage = ADC_Voltage × 5
            //
            // Example calculations:
            //   5.0V rail → 1.0V at ADC → displayed as 5.0V
            //   4.8V rail → 0.96V at ADC → displayed as 4.8V
            //
            // NOTE: Sensor not currently installed - will read ~0V until connected
            adc_param_val = adc_read_volts * 5.0;
            break;

        case ADC_CHANN_BUS_VOLTAGE_3V3:
            // 3.3V rail monitoring with voltage divider (if sensor installed)
            // Voltage divider: R1=30kΩ, R2=7.5kΩ (same as main bus)
            // Divider ratio: R2/(R1+R2) = 7.5/37.5 = 0.2 (1:5 scaling)
            // Formula: Actual_Voltage = ADC_Voltage × 5
            //
            // Example calculations:
            //   3.3V rail → 0.66V at ADC → displayed as 3.3V
            //   3.2V rail → 0.64V at ADC → displayed as 3.2V
            //
            // NOTE: Sensor not currently installed - will read ~0V until connected
            adc_param_val = adc_read_volts * 5.0;
            break;

        default:
            logMessage(false, "SN_Sensors_ADCGetParameterValue()", "Invalid ADC channel: %d", channel);
            adc_param_val = 0;
    }

    return adc_param_val;
#else
    return 0.0;  // ADC disabled
#endif // SN_USE_ADC
}

float SN_Sensors_GetBatteryTemperature() {
#if SN_USE_TEMPERATURE_SENSOR == 1
    // LATENCY FIX: Non-blocking temperature reading
    // DS18B20 conversion takes 750ms - we request in one call, read in the next
    static float last_temp = 0.0;
    static unsigned long last_request_time = 0;
    static bool conversion_in_progress = false;
    
    unsigned long current_time = millis();
    
    if (!conversion_in_progress) {
        // Start a new conversion (non-blocking)
        batt_temp_sensor.requestTemperatures();
        last_request_time = current_time;
        conversion_in_progress = true;
        return last_temp;  // Return cached value while waiting
    }
    
    // Check if conversion is complete (DS18B20 takes ~750ms)
    if (current_time - last_request_time >= 800) {  // 800ms to be safe
        // Read the temperature in Celsius
        float battery_temp = batt_temp_sensor.getTempCByIndex(0);
        
        if (battery_temp != DEVICE_DISCONNECTED_C) {
            last_temp = battery_temp;  // Cache the new value
        } else {
            logMessage(false, "SN_Sensors_GetBatteryTemperature()", "Failed to read temperature from DS18B20 sensor.");
        }
        
        conversion_in_progress = false;  // Ready for next conversion
        return last_temp;
    }
    
    // Conversion still in progress, return cached value
    return last_temp;
#else
    // Temperature sensor not enabled - return default value
    return 0.0;
#endif // SN_USE_TEMPERATURE_SENSOR
}


#if SN_USE_IMU == 1

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
#endif // SN_USE_IMU

#if SN_USE_MAGNETOMETER == 1
/**
 * Compute tilt-compensated compass heading using magnetometer + IMU
 * This corrects for pitch and roll to give accurate heading even when rover is tilted
 */
float computeTiltCompensatedHeading(float mag_x, float mag_y, float mag_z, 
                                    float pitch_rad, float roll_rad) {
    // AXIS TRANSFORMATION: Magnetometer axes rotated relative to IMU
    // IMU: X=forward, Y=left, Z=up
    // MAG sensor physical orientation needs transformation to match
    // Testing: Negate X only
    float mag_x_transformed = -mag_x;       // MAG -X -> IMU X (forward)
    float mag_y_transformed = mag_y;        // MAG Y -> IMU Y (left)
    float mag_z_transformed = mag_z;        // MAG Z -> IMU Z (up)
    
    // Normalize magnetometer readings
    float mag_norm = sqrt(mag_x_transformed * mag_x_transformed + 
                         mag_y_transformed * mag_y_transformed + 
                         mag_z_transformed * mag_z_transformed);
    if (mag_norm < 0.01) {
        // Invalid magnetometer data
        return 0.0;
    }
    
    float mx = mag_x_transformed / mag_norm;
    float my = mag_y_transformed / mag_norm;
    float mz = mag_z_transformed / mag_norm;
    
    // Tilt compensation formulas
    // Reference: Freescale AN4248 "Implementing a Tilt-Compensated eCompass"
    float cos_pitch = cos(pitch_rad);
    float sin_pitch = sin(pitch_rad);
    float cos_roll = cos(roll_rad);
    float sin_roll = sin(roll_rad);
    
    // Compensate for pitch and roll
    float mag_x_comp = mx * cos_pitch + mz * sin_pitch;
    float mag_y_comp = mx * sin_roll * sin_pitch + my * cos_roll - mz * sin_roll * cos_pitch;
    
    // Calculate heading (atan2 returns -π to +π)
    float heading_rad = atan2(mag_y_comp, mag_x_comp);
    
    // Convert to degrees (0-360)
    float heading_deg = heading_rad * 180.0 / PI;
    if (heading_deg < 0) {
        heading_deg += 360.0;
    }
    
    return heading_deg;
}

void read_MAG()
{
    // Read magnetometer data
    magnetometer.read();
    
    // Get raw magnetic field values
    mag_sensor.mag_x = magnetometer.getX();
    mag_sensor.mag_y = magnetometer.getY();
    mag_sensor.mag_z = magnetometer.getZ();
    
    // Get computed azimuth (0-360 degrees)
    mag_sensor.azimuth = magnetometer.getAzimuth();
    
    // Get heading in degrees (floating point for precision)
    mag_sensor.heading_degrees = (float)mag_sensor.azimuth;
    
    // Get cardinal direction string (N, NE, E, SE, S, SW, W, NW)
    magnetometer.getDirection(mag_sensor.direction, 2);
}


void SN_SetMagnetometerCalibration(int x_min, int x_max, int y_min, int y_max, int z_min, int z_max)
{
    magnetometer.setCalibration(x_min, x_max, y_min, y_max, z_min, z_max);
    logMessage(true, "SN_SetMagnetometerCalibration", 
               "Calibration set - X:[%d,%d] Y:[%d,%d] Z:[%d,%d]",
               x_min, x_max, y_min, y_max, z_min, z_max);
}

void SN_SetMagnetometerSmoothing(uint8_t steps, bool advanced)
{
    magnetometer.setSmoothing(steps, advanced);
    logMessage(true, "SN_SetMagnetometerSmoothing", 
               "Smoothing enabled - Steps: %d, Advanced: %s", 
               steps, advanced ? "true" : "false");
}
#endif // SN_USE_MAGNETOMETER

#elif SN_XR4_BOARD_TYPE == SN_XR4_CTU_ESP32






#endif // End of SN_XR4_BOARD_TYPE check