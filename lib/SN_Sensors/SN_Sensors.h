

#define ADC_CHANN_MAIN_BUS_CURRENT 0
#define ADC_CHANN_MAIN_BUS_VOLTAGE 1
#define ADC_CHANN_BATTERY_TEMPERATURE 2
#define ADC_CHANN_AUXILIARY 3


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