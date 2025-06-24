// AemoMotion_Test.ino - Sample sketch to demonstrate AemoMotion library usage.
// This sketch initializes the AemoMotion library, reads MPU-6050 data,
// and logs motion detection events to the Serial Monitor.

#include "gyrosEncode.h" // Include our custom AemoMotion library

// Create an instance of the AemoMotion class
AemoMotion aemo;

void setup() {
    Serial.begin(115200); // Initialize serial communication
    while (!Serial); // Wait for serial port to connect (useful for ESP32)

    Serial.println("\n--- Aemo Motion Library Test ---");
    Serial.println("Initializing MPU-6050...");

    // Initialize the AemoMotion library (and MPU-6050 sensor)
    if (!aemo.begin()) {
        Serial.println("ERROR: MPU-6050 initialization failed! Check wiring.");
        Serial.println("Program will halt.");
        while (true); // Halt if MPU-6050 is not found
    }
    Serial.println("MPU-6050 Initialized Successfully!");
    Serial.println("Move the sensor to see detections.");
    Serial.println("--------------------------------");
}

void loop() {
    // Always call update() to refresh the sensor data before using any detection functions.
    // This should be called as frequently as possible for accurate readings.
    aemo.update();

    // --- Optional: Print Raw Sensor Data for Debugging ---
    // Uncomment these lines if you want to see the raw accelerometer and gyroscope values.
    // Serial.print("Accel (g): X="); Serial.print(aemo.getAccelX(), 2); // 2 decimal places
    // Serial.print(" Y="); Serial.print(aemo.getAccelY(), 2);
    // Serial.print(" Z="); Serial.print(aemo.getAccelZ(), 2);
    // Serial.print(" | Gyro (dps): X="); Serial.print(aemo.getGyroX(), 2);
    // Serial.print(" Y="); Serial.print(aemo.getGyroY(), 2);
    // Serial.print(" Z="); Serial.print(aemo.getGyroZ(), 2);
    // Serial.print(" | Temp: "); Serial.print(aemo.getTemperature(), 2);
    // Serial.println("");


    // --- Motion Detection Feature Checks ---

    // 1. Shake Detection
    if (aemo.detectShake()) {
        Serial.println(">>> Aemo DETECTED SHAKE! <<<");
    }

    // 2. Dominant Axis Detection
    char dominantAxis = aemo.getDominantAxis();
    if (dominantAxis != 'N') { // 'N' means no clear dominant axis or currently in motion
        // Only print if there's a change or a clear axis is identified.
        static char lastDominantAxis = ' '; // Keep track of previous state
        if (dominantAxis != lastDominantAxis) {
            Serial.print("--- Dominant Axis Changed: ");
            Serial.println(dominantAxis);
            lastDominantAxis = dominantAxis;
        }
    } else {
        // Optionally, reset lastDominantAxis if it becomes 'N'
        // static char lastDominantAxis = ' ';
        // if (lastDominantAxis != 'N') {
        //     Serial.println("--- No Clear Dominant Axis ---");
        //     lastDominantAxis = 'N';
        // }
    }

    // 3. Freefall Detection
    if (aemo.isFreefalling()) {
        Serial.println("!!! Aemo DETECTED FREEFALL !!!");
    }

    // 4. Tilt Detection
    // You can adjust the thresholdDegrees as needed. Default is 20.0.
    if (aemo.isTilted(30.0)) { // Example: Check for tilt beyond 30 degrees
        Serial.print("--- Aemo is TILTED! Roll: ");
        Serial.print(aemo.getRoll(), 1); // 1 decimal place
        Serial.print(" deg, Pitch: ");
        Serial.print(aemo.getPitch(), 1);
        Serial.println(" deg ---");
    }

    // 5. Spinning Detection
    // You can adjust gyroThresholdDPS and durationMs. Defaults are 100.0 dps and 500 ms.
    if (aemo.isSpinning(150.0, 750)) { // Example: Spinning fast for longer
        Serial.println(">>> Aemo DETECTED SPINNING! <<<");
    }

    // 6. Jerk Detection
    // You can adjust accelDeltaThreshold and durationMs. Defaults are 0.5 g and 50 ms.
    if (aemo.isJerk(0.7, 50)) { // Example: Detect a sharper jerk
        Serial.println(">>> Aemo DETECTED JERK! <<<");
    }

    // A small delay to prevent overwhelming the serial monitor and allow the ESP32
    // to perform other tasks. Adjust as needed based on your application's requirements.
    delay(50);
}
