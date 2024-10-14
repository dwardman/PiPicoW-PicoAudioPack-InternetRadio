#include <Arduino.h>
#include <WiFi.h>
#include <time.h>  // Include time library for time functions

#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputPimoroni.h"

// Enter your WiFi setup here:
#ifndef STASSID
#define STASSID "YOUR_SSID"  // Define your SSID here
#define STAPSK  "YOUR_PASSWORD"  // Define your password here
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Randomly picked URLs for streaming
const char *allUrls[] = {
    "http://ice2.somafm.com/live-128-mp3",  // Example stream URL 1
    "http://ice6.somafm.com/live-128-mp3",  // Example stream URL 2
};
#define URLS_LENGTH 2  // Define the total number of available URLs

// Declare audio objects
AudioGeneratorMP3 *mp3;
AudioFileSourceICYStream *file;
AudioFileSourceBuffer *buff;
AudioOutputPimoroni *out;

// Shared variable for communication
volatile bool audioTaskRunning = false;  // Flag to indicate if audio task is running
String currentMetadata = "No Metadata";  // Variable to store current metadata

// Function to determine if DST is in effect
bool isDST() {
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    // Check if the current date is in DST
    if (timeinfo->tm_mon > 2 && timeinfo->tm_mon < 10) { // From April to October
        return true;
    }
    if (timeinfo->tm_mon == 2) { // March
        return (timeinfo->tm_mday - (7 - (timeinfo->tm_wday + 1) % 7) >= 14); // After second Sunday
    }
    if (timeinfo->tm_mon == 10) { // November
        return (timeinfo->tm_mday < (7 - (timeinfo->tm_wday + 1) % 7)); // Before first Sunday
    }
    return false;
}

// Metadata callback
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string) {
    const char *ptr = reinterpret_cast<const char *>(cbData);
    (void)isUnicode; // Ignore the Unicode flag for simplicity
    char s1[32], s2[64];
    strncpy_P(s1, type, sizeof(s1));
    s1[sizeof(s1)-1] = 0;
    strncpy_P(s2, string, sizeof(s2));
    s2[sizeof(s2)-1] = 0;

    // Update the current metadata variable
    currentMetadata = String(s1) + " = " + String(s2);
    Serial.printf("METADATA(%s) '%s'\n", ptr, currentMetadata.c_str());
    Serial.flush();
}

// Status callback for errors and warnings
void StatusCallback(void *cbData, int code, const char *string) {
    const char *ptr = reinterpret_cast<const char *>(cbData);
    char s1[64];
    strncpy_P(s1, string, sizeof(s1));
    s1[sizeof(s1)-1] = 0;
    Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
    Serial.flush();
}

// Task running on Core 1 for MP3 streaming
void audioTask() {
    audioTaskRunning = true;  // Set flag to indicate the task is running
    int urlIndex = rand() % URLS_LENGTH;  // Pick a random URL

    // Set up audio sources and output
    file = new AudioFileSourceICYStream(allUrls[urlIndex]);
    file->RegisterMetadataCB(MDCallback, (void*)"ICY");
    buff = new AudioFileSourceBuffer(file, 2048);
    buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
    out = new AudioOutputPimoroni();
    mp3 = new AudioGeneratorMP3();
    mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
    mp3->begin(buff, out);

    // Streaming loop
    while (true) {
        if (mp3->isRunning()) {
            if (!mp3->loop()) mp3->stop();
        } else {
            Serial.println("MP3 done\n");
            delay(5000);
            audioTaskRunning = false;  // Clear flag before finishing
            return;  // Exit the audio task
        }
        delay(1);  // Yield to other processes
    }
}

// Setup function (runs on Core 0)
void setup() {
    set_sys_clock_khz(250000, true);  // Set the system clock to 250 MHz
    
    Serial.begin(115200);  // Start serial communication at 115200 baud rate
    delay(1000);  // Delay to stabilize the system
    
    Serial.println("Connecting to WiFi");
    WiFi.disconnect();  // Disconnect from any previous WiFi connections
    WiFi.softAPdisconnect(true);  // Disable any access point mode if enabled
    WiFi.mode(WIFI_STA);  // Set WiFi to station mode to connect to an access point
    
    WiFi.begin(ssid, password);  // Start the WiFi connection using the provided SSID and password

    // Loop until the WiFi is connected or until we exceed the retry limit
    int connectionAttempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.printf("...Attempting to connect to WiFi: Attempt %d\n", connectionAttempts + 1);
        Serial.printf("Current RSSI: %d dBm\n", WiFi.RSSI());
        if (connectionAttempts++ > 30) {  // Increase the retry limit as needed
            Serial.println("Connection failed. Please check your credentials.");
            return;  // Exit setup if unable to connect
        }
        delay(1000);  // Wait 1 second between connection attempts
    }
    Serial.printf("Connected to WiFi: %s (RSSI: %d dBm)\n", WiFi.SSID().c_str(), WiFi.RSSI());

    // Set up NTP time synchronization
    configTime(-6 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    
    // Launch audio streaming task on Core 1
    multicore_launch_core1(audioTask);  // Start the audio streaming task on Core 1
}

void loop() {
    // Core 0 can be used for other tasks such as monitoring WiFi, checking user input, etc.
    static int lastPrintTime = 0;  // Variable to store the last time a message was printed
    
    // Get and print the current time first
    time_t now = time(nullptr); // Get the current time
    struct tm *timeinfo = localtime(&now); // Convert time to struct tm
    
    // Determine the UTC offset considering DST
    int utcOffset = isDST() ? -5 : -6; // -5 for CDT and -6 for CST
    
    // Prepare logging information
    String logLine = String() +
        String(timeinfo->tm_year + 1900) + "-" +
        String(timeinfo->tm_mon + 1) + "-" +
        String(timeinfo->tm_mday) + " " +
        String((timeinfo->tm_hour + utcOffset + 24) % 24) + ":" +
        String(timeinfo->tm_min) + ":" +
        String(timeinfo->tm_sec) + " | " +  // Current Time
        String(millis()) + " ms | " +  // Running time in milliseconds
        String(WiFi.RSSI()) + " dBm | " +  // RSSI
        currentMetadata;  // Current Metadata

    // Print the log line every second
    if (millis() - lastPrintTime > 1000) {
        lastPrintTime = millis();  // Update the last printed time
        Serial.println(logLine);  // Print the full log line
        Serial.flush();  // Ensure the message is printed immediately
    }

    delay(1000);  // Simple task running on Core 0 with a 1-second delay
}