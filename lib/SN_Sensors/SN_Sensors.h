

#define ADC_CHANN_MAIN_BUS_CURRENT 0
#define ADC_CHANN_MAIN_BUS_VOLTAGE 1
#define ADC_CHANN_BATTERY_TEMPERATURE 2
#define ADC_CHANN_AUXILIARY 3

#if SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32

bool SN_Sensors_ADCInit();
bool SN_Sensors_DS18B20Init();
int16_t SN_Sensors_ADCReadChannel(uint8_t channel);
float SN_Sensors_ADCGetParameterValue(uint8_t channel);
float SN_Sensors_GetBatteryTemperature();
void SN_Sensors_Init();




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

#endif // SN_USE_IMU

#endif // SN_XR4_BOARD_TYPE == SN_XR4_OBC_ESP32