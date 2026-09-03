#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>

LSM6DS3 imu(I2C_MODE, 0x6A);
Madgwick orientationFilter;

void setup()
{
  Serial.begin(115200);

  if (imu.begin() != 0)
  {
    while (true)
    {
      Serial.println("IMU_INIT_FAILED");
      delay(1000);
    }
  }

  orientationFilter.begin(50.0f);
}

void loop()
{
  const float gx = imu.readFloatGyroX();
  const float gy = imu.readFloatGyroY();
  const float gz = imu.readFloatGyroZ();
  const float ax = imu.readFloatAccelX();
  const float ay = imu.readFloatAccelY();
  const float az = imu.readFloatAccelZ();

  orientationFilter.updateIMU(gx, gy, gz, ax, ay, az);

  // Fused physical orientation: roll, pitch, yaw in degrees.
  Serial.print("ORI,");
  Serial.print(orientationFilter.getRoll(), 2);
  Serial.print(',');
  Serial.print(orientationFilter.getPitch(), 2);
  Serial.print(',');
  Serial.print(orientationFilter.getYaw(), 2);
  Serial.print(',');
  Serial.print(gx, 1);
  Serial.print(',');
  Serial.print(gy, 1);
  Serial.print(',');
  Serial.println(gz, 1);

  delay(20);
}
