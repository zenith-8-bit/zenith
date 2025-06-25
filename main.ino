// ESP32 Emo-Like Robot Base Code - Multiprocessing with FreeRTOS
// This code demonstrates setting up multiple tasks (like separate programs running concurrently)
// on the ESP32's dual cores (Core 0 and Core 1) for an emo-like robot.

// -----------------------------------------------------------------------------
// 1. Include Libraries
// -----------------------------------------------------------------------------
#include <WiFi.h>          // Standard library for Wi-Fi connectivity
#include <freertos/task.h> // FreeRTOS library for task management (multitasking)
#include <freertos/queue.h>// FreeRTOS library for inter-task communication (Queues)
#include <freertos/semphr.h>// FreeRTOS library for task synchronization (Semaphores)

// -----------------------------------------------------------------------------
// 2. Global Constants and Definitions
// -----------------------------------------------------------------------------

// Wi-Fi Credentials
const char* ssid = "YOUR_WIFI_SSID";     // Replace with your Wi-Fi network name
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your Wi-Fi password

// Server Connection Details (Placeholder)
const char* serverAddress = "your_server_ip_or_domain.com"; // Replace with your server address
const int serverPort = 80;                                // Replace with your server port (e.g., 80 for HTTP, 443 for HTTPS, or a custom port)

// Pin Definitions for Motors (Placeholders - adjust as per your motor driver/shield)
// Example: Assuming 2 DC motors, each needing 2 control pins (e.g., for L298N driver)
const int MOTOR1_PIN1 = 25;
const int MOTOR1_PIN2 = 26;
const int MOTOR2_PIN1 = 27;
const int MOTOR2_PIN2 = 14;

// Pin Definitions for Sensors (Placeholders - adjust as per your sensor types)
// Example: Assuming an Ultrasonic sensor (HC-SR04) and a Photoresistor
const int ULTRASONIC_TRIG_PIN = 32;
const int ULTRASONIC_ECHO_PIN = 33;
const int PHOTORESISTOR_PIN = 34; // Analog pin

// -----------------------------------------------------------------------------
// 3. Global Variables for FreeRTOS and Communication
// -----------------------------------------------------------------------------

// Task Handles: Used to refer to tasks after they are created.
// You might use these for monitoring, suspending, or resuming tasks.
TaskHandle_t TaskMotorControl = NULL;
TaskHandle_t TaskSensorRead = NULL;
TaskHandle_t TaskServerComm = NULL;
TaskHandle_t TaskMainRobotLogic = NULL;

// Queues: For sending data safely between different tasks.
// E.g., Sensor data can be put into a queue by the sensor task
// and read by the main robot logic task.
QueueHandle_t sensorDataQueue; // Queue to send sensor readings
QueueHandle_t motorCommandQueue; // Queue to send motor commands

// Semaphores: For protecting shared resources (e.g., a global variable)
// or for signaling between tasks.
SemaphoreHandle_t wifiConnectedSemaphore; // Binary semaphore to signal Wi-Fi connection status

// -----------------------------------------------------------------------------
// 4. FreeRTOS Task Functions
//    Each of these functions will run as a separate "thread" or "process"
//    on one of the ESP32's cores.
// -----------------------------------------------------------------------------

// Task 1: Motor Control Task
// This task will handle sending commands to the motors.
// It will typically read commands from a queue.
void motorControlTask(void *pvParameters) {
  // Initialize motor pins (set them as OUTPUTs)
  pinMode(MOTOR1_PIN1, OUTPUT);
  pinMode(MOTOR1_PIN2, OUTPUT);
  pinMode(MOTOR2_PIN1, OUTPUT);
  pinMode(MOTOR2_PIN2, OUTPUT);

  // Initial state: Motors off
  digitalWrite(MOTOR1_PIN1, LOW);
  digitalWrite(MOTOR1_PIN2, LOW);
  digitalWrite(MOTOR2_PIN1, LOW);
  digitalWrite(MOTOR2_PIN2, LOW);

  Serial.println("Motor Control Task running on Core " + String(xPortGetCoreID()));

  // Ensure Wi-Fi is connected before attempting to use motors if they depend on server commands
  // This is an example of inter-task synchronization using a semaphore
  xSemaphoreTake(wifiConnectedSemaphore, portMAX_DELAY); // Wait indefinitely until Wi-Fi is connected

  for (;;) { // Infinite loop for the task
    // Placeholder for receiving motor commands from a queue
    // In a real application, you'd read from `motorCommandQueue` here.
    // Example: int command; if (xQueueReceive(motorCommandQueue, &command, portMAX_DELAY)) { ... execute command ... }

    // --- Motor Control Logic (Placeholder) ---
    // Example: Move forward for 2 seconds, stop for 1 second
    Serial.println("Motors: Moving Forward...");
    digitalWrite(MOTOR1_PIN1, HIGH);
    digitalWrite(MOTOR1_PIN2, LOW);
    digitalWrite(MOTOR2_PIN1, HIGH);
    digitalWrite(MOTOR2_PIN2, LOW);
    vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2 seconds (non-blocking for other tasks)

    Serial.println("Motors: Stopping...");
    digitalWrite(MOTOR1_PIN1, LOW);
    digitalWrite(MOTOR1_PIN2, LOW);
    digitalWrite(MOTOR2_PIN1, LOW);
    digitalWrite(MOTOR2_PIN2, LOW);
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second

    // --- End Motor Control Logic ---

    // Yield control back to the scheduler if this task has nothing else to do
    // This allows other tasks of the same priority to run.
    vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to prevent task from hogging CPU if queue is empty
  }
}

// Task 2: Sensor Reading Task
// This task will periodically read data from various sensors and
// send the readings to another task (e.g., Main Robot Logic) via a queue.
void sensorReadTask(void *pvParameters) {
  // Initialize sensor pins (set them as INPUTs or specific sensor init)
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(PHOTORESISTOR_PIN, INPUT);

  Serial.println("Sensor Reading Task running on Core " + String(xPortGetCoreID()));

  for (;;) { // Infinite loop for the task
    // --- Sensor Reading Logic (Placeholder) ---
    // 1. Read Ultrasonic Sensor (Distance)
    long duration, distanceCm;
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(2));
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(10));
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
    distanceCm = duration * 0.034 / 2; // Speed of sound in cm/microsecond

    // 2. Read Photoresistor (Light Level)
    int lightLevel = analogRead(PHOTORESISTOR_PIN);

    Serial.print("Sensors: Distance = ");
    Serial.print(distanceCm);
    Serial.print(" cm, Light = ");
    Serial.println(lightLevel);

    // --- End Sensor Reading Logic ---

    // Send sensor data to the main robot logic task via the queue
    // In a real application, you'd define a struct to hold all sensor data.
    // Example: struct SensorData { int distance; int light; };
    //          SensorData currentReadings = {distanceCm, lightLevel};
    //          xQueueSend(sensorDataQueue, &currentReadings, portMAX_DELAY);

    vTaskDelay(pdMS_TO_TICKS(500)); // Read sensors every 500 milliseconds
  }
}

// Task 3: Server Communication Task
// This task handles sending data to and receiving commands from a remote server.
// It relies on Wi-Fi being connected.
void serverCommunicationTask(void *pvParameters) {
  Serial.println("Server Communication Task running on Core " + String(xPortGetCoreID()));

  // Wait until Wi-Fi is connected before attempting server communication
  xSemaphoreTake(wifiConnectedSemaphore, portMAX_DELAY); // Wait indefinitely

  // Create a WiFiClient object to connect to the server
  WiFiClient client;

  for (;;) { // Infinite loop for the task
    if (WiFi.isConnected()) {
      if (!client.connected()) {
        Serial.print("Server: Connecting to ");
        Serial.print(serverAddress);
        Serial.print(":");
        Serial.println(serverPort);
        if (client.connect(serverAddress, serverPort)) {
          Serial.println("Server: Connected!");
          // Send initial data or handshake
          client.println("Hello Server from ESP32!");
        } else {
          Serial.println("Server: Connection failed!");
        }
      }

      if (client.connected()) {
        // --- Server Interaction Logic (Placeholder) ---
        // 1. Send data to server (e.g., sensor readings from a queue)
        // Example: client.println("SensorData: " + String(distanceCm));

        // 2. Receive commands from server
        while (client.available()) {
          String line = client.readStringUntil('\n');
          Serial.print("Server: Received -> ");
          Serial.println(line);
          // Parse received command and potentially put it into motorCommandQueue
          // or another queue for the main robot logic task.
        }

        // --- End Server Interaction Logic ---
      } else {
        Serial.println("Server: Client disconnected, attempting to reconnect...");
      }
    } else {
      Serial.println("Server: Wi-Fi lost, waiting for reconnection...");
    }

    vTaskDelay(pdMS_TO_TICKS(5000)); // Attempt communication/reconnect every 5 seconds
  }
}

// Task 4: Main Robot Logic Task
// This task acts as the "brain" of the robot. It processes sensor data,
// makes decisions, and sends commands to other tasks (e.g., motor task).
void mainRobotLogicTask(void *pvParameters) {
  Serial.println("Main Robot Logic Task running on Core " + String(xPortGetCoreID()));

  // Wait until Wi-Fi is connected if logic depends on it (e.g., for server data)
  xSemaphoreTake(wifiConnectedSemaphore, portMAX_DELAY); // Wait indefinitely

  for (;;) { // Infinite loop for the task
    // --- Robot Decision-Making Logic (Placeholder) ---
    // Example: Read sensor data from queue and make a decision
    // int receivedDistance, receivedLight;
    // if (xQueueReceive(sensorDataQueue, &receivedSensorData, 0)) { // 0 means no wait
    //   Serial.print("Logic: Processed Distance="); Serial.print(receivedSensorData.distance);
    //   Serial.print(", Light="); Serial.println(receivedSensorData.light);
    //   // If distance too close, send stop command to motor task
    //   // int command = STOP;
    //   // xQueueSend(motorCommandQueue, &command, 0);
    // }

    // Simulate some decision based on time
    static int counter = 0;
    if (counter % 10 == 0) { // Every 10 seconds (approx)
      Serial.println("Main Logic: Making a decision...");
      // Here you'd implement AI/emotional responses, state changes, etc.
    }
    counter++;

    // --- End Robot Decision-Making Logic ---

    vTaskDelay(pdMS_TO_TICKS(1000)); // Run logic every 1 second
  }
}

// -----------------------------------------------------------------------------
// 5. Arduino Setup Function (Runs once on startup)
// -----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  while (!Serial) { delay(100); } // Wait for serial port to connect

  Serial.println("\n--- ESP32 Robot Booting Up ---");

  // Create Semaphores
  // Binary semaphore for Wi-Fi connection status. Initial state is not taken.
  wifiConnectedSemaphore = xSemaphoreCreateBinary();
  if (wifiConnectedSemaphore == NULL) {
    Serial.println("Error creating Wi-Fi Connected Semaphore!");
    // Handle error, e.g., halt system
  }

  // Create Queues (adjust queue length based on expected data volume)
  // Example: Queue for integers, length 10
  sensorDataQueue = xQueueCreate(10, sizeof(int)); // Placeholder: If sending structs, use sizeof(MyStruct)
  motorCommandQueue = xQueueCreate(5, sizeof(int)); // Placeholder: If sending structs, use sizeof(MyStruct)

  if (sensorDataQueue == NULL || motorCommandQueue == NULL) {
    Serial.println("Error creating one or more Queues!");
    // Handle error
  }

  // Connect to Wi-Fi (this happens on Core 0 by default, which is good)
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait for Wi-Fi connection with a timeout
  unsigned long connectionStartTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - connectionStartTime > 30000) { // 30-second timeout
      Serial.println("\nWiFi connection timed out!");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    // Give the semaphore to signal that Wi-Fi is connected
    xSemaphoreGive(wifiConnectedSemaphore);
  } else {
    Serial.println("\nFailed to connect to WiFi. Tasks relying on WiFi might not function.");
  }

  // -------------------------------------------------------------------------
  // Create FreeRTOS Tasks
  // Syntax: xTaskCreatePinnedToCore( TaskFunction, Name, StackDepth, Parameters, Priority, TaskHandle, CoreID );
  //
  // CoreID:
  //   0 (PRO_CPU): Handles Wi-Fi, Bluetooth, and standard Arduino loop.
  //   1 (APP_CPU): Generally free for user applications, good for heavy tasks.
  //
  // StackDepth: Amount of memory (in bytes) allocated for the task's stack.
  //             Adjust based on task complexity. 10000-20000 is a safe starting point for complex tasks.
  // Priority: 0 (lowest) to configMAX_PRIORITIES-1 (highest). Higher number = higher priority.
  // -------------------------------------------------------------------------

  // Create Motor Control Task on Core 1 (APP_CPU) to offload it from Core 0
  xTaskCreatePinnedToCore(
      motorControlTask,   // Task function
      "MotorControl",     // Name of task
      10000,              // Stack size in words (approx. 40KB for 10000 words on ESP32)
      NULL,               // Parameter to pass to the
