// AemoMotion.h - Library for MPU-6050 motion detection and interpretation
// This library provides functions to read MPU-6050 data and detect specific
// motion events such as shake, freefall, tilt, dominant axis, spinning, and jerk.

#ifndef AEMO_MOTION_H
#define AEMO_MOTION_H

#include <Arduino.h>
#include <Wire.h> // Required for I2C communication with MPU-6050

// MPU-6050 I2C address
#define MPU6050_ADDR 0x68 // SDO pin low
// #define MPU6050_ADDR 0x69 // SDO pin high (if you connected SDO to VCC)

// MPU-6050 Register Map
#define MPU6050_SMPLRT_DIV 0x19   // Sample Rate Divider
#define MPU6050_CONFIG 0x1A       // Configuration
#define MPU6050_GYRO_CONFIG 0x1B  // Gyroscope Configuration
#define MPU6050_ACCEL_CONFIG 0x1C // Accelerometer Configuration
#define MPU6050_ACCEL_XOUT_H 0x3B // Accel X-axis High Byte
#define MPU6050_ACCEL_XOUT_L 0x3C // Accel X-axis Low Byte
#define MPU6050_ACCEL_YOUT_H 0x3D // Accel Y-axis High Byte
#define MPU6050_ACCEL_YOUT_L 0x3E // Accel Y-axis Low Byte
#define MPU6050_ACCEL_ZOUT_H 0x3F // Accel Z-axis High Byte
#define MPU6050_ACCEL_ZOUT_L 0x40 // Accel Z-axis Low Byte
#define MPU6050_TEMP_OUT_H 0x41   // Temperature High Byte
#define MPU6050_TEMP_OUT_L 0x42   // Temperature Low Byte
#define MPU6050_GYRO_XOUT_H 0x43  // Gyro X-axis High Byte
#define MPU6050_GYRO_XOUT_L 0x44  // Gyro X-axis Low Byte
#define MPU6050_GYRO_YOUT_H 0x45  // Gyro Y-axis High Byte
#define MPU6050_GYRO_YOUT_L 0x46  // Gyro Y-axis Low Byte
#define MPU6050_GYRO_ZOUT_H 0x47  // Gyro Z-axis High Byte
#define MPU6050_GYRO_ZOUT_L 0x48  // Gyro Z-axis Low Byte
#define MPU6050_PWR_MGMT_1 0x6B   // Power Management 1
#define MPU6050_WHO_AM_I 0x75     // Who Am I (Device ID)

// MPU-6050 Full Scale Ranges (Default to +/- 2g and +/- 250 deg/s)
// These define the sensitivity of the sensor and the raw data scaling.
// Accel FSR: +/-2g = 16384 LSB/g, +/-4g = 8192 LSB/g, +/-8g = 4096 LSB/g, +/-16g = 2048 LSB/g
// Gyro FSR: +/-250 deg/s = 131 LSB/deg/s, +/-500 = 65.5 LSB/deg/s, etc.
#define ACCEL_SCALE_FACTOR 16384.0 // LSB/g for +/- 2g range
#define GYRO_SCALE_FACTOR 131.0    // LSB/deg/s for +/- 250 deg/s range

class AemoMotion {
public:
    // Constructor
    AemoMotion();

    // Initializes the MPU-6050 sensor
    // Returns true on success, false on failure.
    bool begin();

    // Updates sensor data. Call this frequently in your loop().
    void update();

    // --- Raw Sensor Data Accessors ---
    float getAccelX() { return _accelX; }
    float getAccelY() { return _accelY; }
    float getAccelZ() { return _accelZ; }
    float getGyroX() { return _gyroX; }
    float getGyroY() { return _gyroY; }
    float getGyroZ() { return _gyroZ; }
    float getTemperature() { return _temperature; }

    // --- Motion Feature Detection Functions ---

    // detectShake(): Detects a sudden, high-magnitude acceleration.
    // Triggers only once per shake event, followed by a cooldown.
    // Returns true if a shake is detected.
    bool detectShake();

    // getDominantAxis(): Determines the primary axis affected by gravity.
    // Helps identify the robot's orientation (e.g., right-side up, on its back).
    // Returns 'X', 'Y', 'Z', or 'N' if no clear dominant axis (e.g., freefall or rapid motion).
    char getDominantAxis();

    // isFreefalling(): Detects if the robot is in freefall (near 0g acceleration).
    // This requires acceleration magnitude to be below a threshold for a set duration.
    // Returns true if freefall is detected.
    bool isFreefalling();

    // isTilted(): Checks if the robot is tilted beyond a specified angular threshold.
    // Uses roll and pitch angles derived from accelerometer data.
    // thresholdDegrees: The angle in degrees beyond which it's considered tilted.
    // Returns true if the robot is tilted.
    bool isTilted(float thresholdDegrees = 20.0);

    // getRoll(): Calculates the roll angle (rotation around X-axis) in degrees.
    // Based on accelerometer data.
    float getRoll();

    // getPitch(): Calculates the pitch angle (rotation around Y-axis) in degrees.
    // Based on accelerometer data.
    float getPitch();

    // isSpinning(): Detects if the robot is rotating rapidly and continuously.
    // gyroThresholdDPS: Angular velocity threshold in degrees per second (DPS).
    // durationMs: How long the gyroscope readings must exceed the threshold to be considered spinning.
    // Returns true if spinning is detected.
    bool isSpinning(float gyroThresholdDPS = 100.0, unsigned long durationMs = 500);

    // isJerk(): Detects a sudden, sharp change in acceleration (the derivative of acceleration).
    // accelDeltaThreshold: The change in acceleration magnitude (in g) required to trigger a jerk.
    // durationMs: The time window over which the acceleration change is evaluated (e.g., for noise reduction).
    // Returns true if a jerk is detected.
    bool isJerk(float accelDeltaThreshold = 0.5, unsigned long durationMs = 50);

    // --- Configuration Setters for Sensitivity ---

    // setShakeThreshold(): Sets the acceleration magnitude threshold for shake detection.
    // thresholdG: Acceleration in 'g's (e.g., 1.5 for a moderate shake).
    void setShakeThreshold(float thresholdG);

    // setFreefallThreshold(): Sets parameters for freefall detection.
    // accelThresholdG: Acceleration magnitude in 'g's below which freefall is considered.
    // durationMs: Minimum duration in milliseconds for freefall condition to persist.
    void setFreefallThreshold(float accelThresholdG, unsigned long durationMs);

    // setSpinningThreshold(): Sets parameters for spinning detection.
    // gyroThresholdDPS: Angular velocity in deg/s.
    // durationMs: Minimum duration in milliseconds for spinning condition to persist.
    void setSpinningThreshold(float gyroThresholdDPS, unsigned long durationMs);

    // setJerkThreshold(): Sets parameters for jerk detection.
    // accelDeltaThresholdG: Change in acceleration magnitude in 'g's.
    // durationMs: Time window for measuring acceleration change in milliseconds.
    void setJerkThreshold(float accelDeltaThresholdG, unsigned long durationMs);


private:
    // Raw sensor readings (converted to g and deg/s)
    float _accelX, _accelY, _accelZ;
    float _gyroX, _gyroY, _gyroZ;
    float _temperature;

    // Time tracking variables for rate-based detections
    unsigned long _lastUpdateTime;
    float _dt; // Time elapsed since last update in seconds

    // --- Internal State Variables for Motion Detection ---

    // Shake detection variables
    float _shakeThreshold = 1.5; // Default shake threshold in g
    unsigned long _lastShakeTriggerTime = 0;
    unsigned long _shakeCooldownMs = 1000; // Cooldown period after a shake detection

    // Freefall detection variables
    float _freefallAccelThreshold = 0.2; // Default freefall acceleration threshold in g
    unsigned long _freefallDurationThreshold = 100; // Default freefall duration in ms
    unsigned long _freefallStartTime = 0;
    bool _isCurrentlyFreefalling = false; // Internal state flag

    // Spinning detection variables
    float _spinningGyroThresholdDPS = 100.0; // Default spinning angular velocity threshold
    unsigned long _spinningDurationThresholdMs = 500; // Default spinning duration in ms
    unsigned long _spinningStartTime = 0;
    bool _isCurrentlySpinning = false; // Internal state flag

    // Jerk detection variables
    float _jerkAccelDeltaThreshold = 0.5; // Default jerk acceleration change threshold in g
    unsigned long _jerkDurationThreshold = 50; // Default jerk duration (unused for simple jerk, but kept for consistency)
    unsigned long _lastJerkTriggerTime = 0;
    unsigned long _jerkCooldownMs = 500; // Cooldown period after a jerk detection
    float _prevAccelMagnitude = 0.0; // Previous total acceleration magnitude for jerk calculation

    // --- MPU-6050 Communication Helpers ---

    // Reads a single byte from a MPU-6050 register
    uint8_t readMPU6050Register(uint8_t reg);

    // Writes a single byte to a MPU-6050 register
    void writeMPU6050Register(uint8_t reg, uint8_t value);

    // Reads 16-bit signed integer (two bytes) from MPU-6050 registers
    int16_t readMPU6050Word(uint8_t reg);
};

#endif // AEMO_MOTION_H

// --- Implementation of AemoMotion Class ---

AemoMotion::AemoMotion() {
    _lastUpdateTime = micros(); // Initialize last update time
}

// Initializes the MPU-6050 sensor
bool AemoMotion::begin() {
    Wire.begin(); // Join I2C bus

    // Check device ID (WHO_AM_I register)
    uint8_t whoAmI = readMPU6050Register(MPU6050_WHO_AM_I);
    if (whoAmI != MPU6050_ADDR) { // MPU-6050's WHO_AM_I register should return its address
        Serial.print("MPU-6050 not found, WHO_AM_I returned: 0x");
        Serial.println(whoAmI, HEX);
        return false;
    }

    // Wake up MPU-6050 (Power Management 1 register)
    // Clear SLEEP bit (0x00) to wake up sensor.
    writeMPU6050Register(MPU6050_PWR_MGMT_1, 0x00);
    delay(100); // Give sensor time to wake up

    // Configure Accelerometer (ACCEL_CONFIG register)
    // Set to +/- 2g (0x00). Can be 0x08 (+/-4g), 0x10 (+/-8g), 0x18 (+/-16g)
    // The ACCEL_SCALE_FACTOR above must be adjusted if this value changes.
    writeMPU6050Register(MPU6050_ACCEL_CONFIG, 0x00);

    // Configure Gyroscope (GYRO_CONFIG register)
    // Set to +/- 250 deg/s (0x00). Can be 0x08 (+/-500), 0x10 (+/-1000), 0x18 (+/-2000)
    // The GYRO_SCALE_FACTOR above must be adjusted if this value changes.
    writeMPU6050Register(MPU6050_GYRO_CONFIG, 0x00);

    // Set Sample Rate Divider (SMPLRT_DIV register)
    // Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
    // Gyro Output Rate = 8kHz (DLPF disabled) or 1kHz (DLPF enabled)
    // Setting 0x07 (for 125Hz output rate if 1kHz gyro output)
    writeMPU6050Register(MPU6050_SMPLRT_DIV, 0x07);

    // Configure Digital Low Pass Filter (DLPF) - part of CONFIG register
    // Set DLPF_CFG to 0x03 for bandwidth 44Hz (accel) / 42Hz (gyro)
    writeMPU6050Register(MPU6050_CONFIG, 0x03);

    // Perform an initial read to populate values
    update();
    _prevAccelMagnitude = sqrt(pow(_accelX, 2) + pow(_accelY, 2) + pow(_accelZ, 2));

    return true;
}

// Updates sensor data. Call this frequently in your loop().
void AemoMotion::update() {
    unsigned long currentTime = micros();
    _dt = (currentTime - _lastUpdateTime) / 1000000.0; // Convert microseconds to seconds
    _lastUpdateTime = currentTime;

    // Read raw 16-bit sensor data
    int16_t rawAccelX = readMPU6050Word(MPU6050_ACCEL_XOUT_H);
    int16_t rawAccelY = readMPU6050Word(MPU6050_ACCEL_YOUT_H);
    int16_t rawAccelZ = readMPU6050Word(MPU6050_ACCEL_ZOUT_H);

    int16_t rawGyroX = readMPU6050Word(MPU6050_GYRO_XOUT_H);
    int16_t rawGyroY = readMPU6050Word(MPU6050_GYRO_YOUT_H);
    int16_t rawGyroZ = readMPU6050Word(MPU6050_GYRO_ZOUT_H);

    int16_t rawTemp = readMPU6050Word(MPU6050_TEMP_OUT_H);

    // Convert raw data to engineering units (g and deg/s)
    _accelX = (float)rawAccelX / ACCEL_SCALE_FACTOR;
    _accelY = (float)rawAccelY / ACCEL_SCALE_FACTOR;
    _accelZ = (float)rawAccelZ / ACCEL_SCALE_FACTOR;

    _gyroX = (float)rawGyroX / GYRO_SCALE_FACTOR;
    _gyroY = (float)rawGyroY / GYRO_SCALE_FACTOR;
    _gyroZ = (float)rawGyroZ / GYRO_SCALE_FACTOR;

    // Temperature in Celsius = (raw_temp / 340) + 36.53
    _temperature = (float)rawTemp / 340.0 + 36.53;
}

// --- Motion Feature Detection Implementations ---

// detectShake(): Detects a sudden, high-magnitude acceleration.
bool AemoMotion::detectShake() {
    float accelMagnitude = sqrt(pow(_accelX, 2) + pow(_accelY, 2) + pow(_accelZ, 2));
    unsigned long currentMillis = millis();

    // Check if the current acceleration magnitude exceeds the threshold
    // and if enough time has passed since the last shake detection (cooldown)
    if (accelMagnitude > _shakeThreshold && (currentMillis - _lastShakeTriggerTime > _shakeCooldownMs)) {
        _lastShakeTriggerTime = currentMillis; // Update the last trigger time
        return true; // Shake detected!
    }
    return false;
}

// getDominantAxis(): Determines the primary axis affected by gravity.
char AemoMotion::getDominantAxis() {
    // We expect one axis to be close to +/- 1g when stationary
    // and the other two close to 0g.
    float absX = fabs(_accelX);
    float absY = fabs(_accelY);
    float absZ = fabs(_accelZ);

    float tolerance = 0.2; // Tolerance for 'g' values

    // Check if any axis is significantly dominant (close to 1g)
    if (absX > (1.0 - tolerance) && absX < (1.0 + tolerance) && absY < tolerance && absZ < tolerance) {
        return 'X';
    } else if (absY > (1.0 - tolerance) && absY < (1.0 + tolerance) && absX < tolerance && absZ < tolerance) {
        return 'Y';
    } else if (absZ > (1.0 - tolerance) && absZ < (1.0 + tolerance) && absX < tolerance && absY < tolerance) {
        return 'Z';
    }
    // If no single axis is clearly dominant, or if all are near zero (freefall)
    // or all are high (strong acceleration), return 'N' (None)
    return 'N';
}

// isFreefalling(): Detects if the robot is in freefall (near 0g acceleration).
bool AemoMotion::isFreefalling() {
    float accelMagnitude = sqrt(pow(_accelX, 2) + pow(_accelY, 2) + pow(_accelZ, 2));
    unsigned long currentMillis = millis();

    if (accelMagnitude < _freefallAccelThreshold) {
        // If below threshold, check if this is the start of a new freefall event
        if (!_isCurrentlyFreefalling) {
            _freefallStartTime = currentMillis; // Record start time
            _isCurrentlyFreefalling = true;
        } else {
            // If already freefalling, check if duration threshold is met
            if (currentMillis - _freefallStartTime > _freefallDurationThreshold) {
                return true; // Freefall detected for long enough!
            }
        }
    } else {
        // If acceleration is above threshold, reset freefall state
        _isCurrentlyFreefalling = false;
        _freefallStartTime = 0;
    }
    return false;
}

// isTilted(): Checks if the robot is tilted beyond a specified angular threshold.
bool AemoMotion::isTilted(float thresholdDegrees) {
    float roll = getRoll();
    float pitch = getPitch();

    // Check if absolute roll or pitch exceeds the threshold
    if (fabs(roll) > thresholdDegrees || fabs(pitch) > thresholdDegrees) {
        return true;
    }
    return false;
}

// getRoll(): Calculates the roll angle (rotation around X-axis) in degrees.
// Uses accelerometer data. Assumes Z-axis points down when level.
float AemoMotion::getRoll() {
    // atan2(Y_accel, Z_accel) when X is forward/back, Y is left/right, Z is up/down
    // This formula calculates roll around the X-axis.
    return atan2(_accelY, _accelZ) * 180.0 / M_PI;
}

// getPitch(): Calculates the pitch angle (rotation around Y-axis) in degrees.
// Uses accelerometer data. Assumes Z-axis points down when level.
float AemoMotion::getPitch() {
    // atan2(-X_accel, sqrt(Y_accel^2 + Z_accel^2)) for pitch around Y-axis
    // The negative X is typical for IMUs where positive X is forward.
    return atan2(-_accelX, sqrt(pow(_accelY, 2) + pow(_accelZ, 2))) * 180.0 / M_PI;
}

// isSpinning(): Detects if the robot is rotating rapidly and continuously.
bool AemoMotion::isSpinning(float gyroThresholdDPS, unsigned long durationMs) {
    float totalGyroMagnitude = sqrt(pow(_gyroX, 2) + pow(_gyroY, 2) + pow(_gyroZ, 2));
    unsigned long currentMillis = millis();

    if (totalGyroMagnitude > gyroThresholdDPS) {
        // If angular velocity is above threshold, check if this is the start of a new spinning event
        if (!_isCurrentlySpinning) {
            _spinningStartTime = currentMillis; // Record start time
            _isCurrentlySpinning = true;
        } else {
            // If already spinning, check if duration threshold is met
            if (currentMillis - _spinningStartTime > durationMs) {
                return true; // Spinning detected for long enough!
            }
        }
    } else {
        // If angular velocity is below threshold, reset spinning state
        _isCurrentlySpinning = false;
        _spinningStartTime = 0;
    }
    return false;
}

// isJerk(): Detects a sudden, sharp change in acceleration.
bool AemoMotion::isJerk(float accelDeltaThreshold, unsigned long durationMs) {
    float currentAccelMagnitude = sqrt(pow(_accelX, 2) + pow(_accelY, 2) + pow(_accelZ, 2));
    unsigned long currentMillis = millis();

    // Calculate the absolute change in acceleration magnitude
    float deltaAccel = fabs(currentAccelMagnitude - _prevAccelMagnitude);

    // Update previous acceleration magnitude for the next iteration
    _prevAccelMagnitude = currentAccelMagnitude;

    // Check if the change in acceleration exceeds the threshold
    // and if enough time has passed since the last jerk detection (cooldown)
    if (deltaAccel > accelDeltaThreshold && (currentMillis - _lastJerkTriggerTime > _jerkCooldownMs)) {
        _lastJerkTriggerTime = currentMillis; // Update the last trigger time
        return true; // Jerk detected!
    }
    return false;
}

// --- Configuration Setters ---

void AemoMotion::setShakeThreshold(float thresholdG) {
    _shakeThreshold = thresholdG;
}

void AemoMotion::setFreefallThreshold(float accelThresholdG, unsigned long durationMs) {
    _freefallAccelThreshold = accelThresholdG;
    _freefallDurationThreshold = durationMs;
}

void AemoMotion::setSpinningThreshold(float gyroThresholdDPS, unsigned long durationMs) {
    _spinningGyroThresholdDPS = gyroThresholdDPS;
    _spinningDurationThresholdMs = durationMs;
}

void AemoMotion::setJerkThreshold(float accelDeltaThresholdG, unsigned long durationMs) {
    _jerkAccelDeltaThreshold = accelDeltaThresholdG;
    // Note: durationMs is currently not heavily used for jerk as it's often instantaneous.
    // However, it's good practice to keep it for potential future filtering or more complex jerk definitions.
    // For this simple implementation, the cooldown is more impactful.
}

// --- MPU-6050 Communication Private Methods ---

// Reads a single byte from a MPU-6050 register
uint8_t AemoMotion::readMPU6050Register(uint8_t reg) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg); // Register to read from
    Wire.endTransmission(false); // Do not release bus
    Wire.requestFrom(MPU6050_ADDR, 1); // Request 1 byte
    while (!Wire.available()); // Wait for data
    return Wire.read();
}

// Writes a single byte to a MPU-6050 register
void AemoMotion::writeMPU6050Register(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg); // Register to write to
    Wire.write(value); // Value to write
    Wire.endTransmission(true); // Release bus
}

// Reads 16-bit signed integer (two bytes) from MPU-6050 registers
int16_t AemoMotion::readMPU6050Word(uint8_t reg) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg); // Start reading from high byte register
    Wire.endTransmission(false); // Do not release bus
    Wire.requestFrom(MPU6050_ADDR, 2); // Request 2 bytes (high and low)
    while (!Wire.available()); // Wait for data

    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();

    return (int16_t)((highByte << 8) | lowByte); // Combine bytes into a signed 16-bit integer
}
