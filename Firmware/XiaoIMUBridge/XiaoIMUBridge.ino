#include <Arduino.h>
#include <LSM6DS3.h>
#include <Wire.h>

// The upstream Arduino Madgwick library stores the fused quaternion but only
// exposes Euler getters. Limit this compatibility shim to this one header so
// we can transmit the original quaternion without editing an installed library.
// MadgwickAHRS.h includes Arduino.h, which is already guarded above.
#define private public
#include <MadgwickAHRS.h>
#undef private

LSM6DS3 imu(I2C_MODE, 0x6A);
Madgwick orientationFilter;

constexpr float SampleFrequencyHz = 50.0f;
constexpr uint32_t SamplePeriodMicros = 20000;
constexpr int GyroCalibrationSamples = 200;

float gyroBiasX = 0.0f;
float gyroBiasY = 0.0f;
float gyroBiasZ = 0.0f;
uint32_t nextSampleMicros = 0;

void calibrateAndInitializeOrientation()
{
  float gyroSumX = 0.0f;
  float gyroSumY = 0.0f;
  float gyroSumZ = 0.0f;
  float accelSumX = 0.0f;
  float accelSumY = 0.0f;
  float accelSumZ = 0.0f;

  // Keep the doll still in its forward pose during this two-second startup.
  for (int index = 0; index < GyroCalibrationSamples; ++index)
  {
    gyroSumX += imu.readFloatGyroX();
    gyroSumY += imu.readFloatGyroY();
    gyroSumZ += imu.readFloatGyroZ();
    accelSumX += imu.readFloatAccelX();
    accelSumY += imu.readFloatAccelY();
    accelSumZ += imu.readFloatAccelZ();
    delay(10);
  }

  const float inverseSampleCount = 1.0f / GyroCalibrationSamples;
  gyroBiasX = gyroSumX * inverseSampleCount;
  gyroBiasY = gyroSumY * inverseSampleCount;
  gyroBiasZ = gyroSumZ * inverseSampleCount;

  const float ax = accelSumX * inverseSampleCount;
  const float ay = accelSumY * inverseSampleCount;
  const float az = accelSumZ * inverseSampleCount;

  // Seed roll and pitch from gravity instead of waiting several seconds for
  // the filter to converge from the identity quaternion. Yaw starts at zero.
  const float roll = atan2f(ay, az);
  const float pitch = atan2f(-ax, sqrtf(ay * ay + az * az));
  const float halfRoll = 0.5f * roll;
  const float halfPitch = 0.5f * pitch;
  const float cosRoll = cosf(halfRoll);
  const float sinRoll = sinf(halfRoll);
  const float cosPitch = cosf(halfPitch);
  const float sinPitch = sinf(halfPitch);

  orientationFilter.q0 = cosRoll * cosPitch;
  orientationFilter.q1 = sinRoll * cosPitch;
  orientationFilter.q2 = cosRoll * sinPitch;
  orientationFilter.q3 = -sinRoll * sinPitch;
}

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

  orientationFilter.begin(SampleFrequencyHz);
  delay(100);
  calibrateAndInitializeOrientation();
  nextSampleMicros = micros() + SamplePeriodMicros;
}

void loop()
{
  const uint32_t currentMicros = micros();
  if (static_cast<int32_t>(currentMicros - nextSampleMicros) < 0)
  {
    return;
  }

  nextSampleMicros += SamplePeriodMicros;
  if (static_cast<int32_t>(currentMicros - nextSampleMicros) >
      static_cast<int32_t>(SamplePeriodMicros))
  {
    nextSampleMicros = currentMicros + SamplePeriodMicros;
  }

  const float gx = imu.readFloatGyroX() - gyroBiasX;
  const float gy = imu.readFloatGyroY() - gyroBiasY;
  const float gz = imu.readFloatGyroZ() - gyroBiasZ;
  const float ax = imu.readFloatAccelX();
  const float ay = imu.readFloatAccelY();
  const float az = imu.readFloatAccelZ();

  orientationFilter.updateIMU(gx, gy, gz, ax, ay, az);

  // Raw Madgwick quaternion packet: W, X, Y, Z, Gyro X, Gyro Y, Gyro Z.
  // Sending the fused quaternion avoids the Euler singularity near +/-90 deg.
  const float qw = orientationFilter.q0;
  const float qx = orientationFilter.q1;
  const float qy = orientationFilter.q2;
  const float qz = orientationFilter.q3;
  const float normSquared = qw * qw + qx * qx + qy * qy + qz * qz;

  if (isfinite(normSquared) && normSquared > 0.0f)
  {
    const float inverseNorm = 1.0f / sqrtf(normSquared);
    Serial.print("QUAT,");
    Serial.print(qw * inverseNorm, 6);
    Serial.print(',');
    Serial.print(qx * inverseNorm, 6);
    Serial.print(',');
    Serial.print(qy * inverseNorm, 6);
    Serial.print(',');
    Serial.print(qz * inverseNorm, 6);
    Serial.print(',');
    Serial.print(gx, 1);
    Serial.print(',');
    Serial.print(gy, 1);
    Serial.print(',');
    Serial.println(gz, 1);
  }
}
