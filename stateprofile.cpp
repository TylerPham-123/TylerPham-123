
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <cstdlib>
#include <future>
#include <chrono>
#include <jsoncpp/json/json.h>
#include <opencv2/opencv.hpp>

using namespace std;
namespace fs = std::filesystem;

// ---------------------------
// File and Directory Paths
// ---------------------------
const string HEART_RATE_LOG = "/home/m30w/myenv/logs/heart_rate.txt";
const string TEMP_LOG       = "/home/m30w/myenv/logs/temperature.txt";
const string MOTION_LOG     = "/home/m30w/myenv/logs/motion.txt";

const string CORE_WORDS_DIR    = "/home/m30w/myenv/Thresholds/core_words/";
const string CORE_EMOTIONS_DIR = "/home/m30w/myenv/Thresholds/core_emotions/";
const string BODY_LANGUAGE_DIR = "/home/m30w/myenv/Thresholds/body_language/";

// All final files are stored in OUTPUT_DIR.
const string OUTPUT_DIR = "/home/m30w/myenv/Thresholds/output/";

// Log file to be embedded at the bottom in small font.
const string LOG_FILE = "/home/m30w/log.txt";

// Path for USB camera snapshot.
const string CAMERA_IMAGE = OUTPUT_DIR + "camera_snapshot.jpg";

// Final output image name must be exactly "state_profile-0.jpg".
const string FINAL_IMG_NAME = OUTPUT_DIR + "state_profile-0.jpg";

// ---------------------------
// Custom Error Codes
// ---------------------------
const int ERR_NO_CORE_WORDS = 3;
const int ERR_NO_CORE_EMOTIONS = 4;
const int ERR_NO_BODY_LANG = 5;
const int ERR_NO_CAMERA = 2;

// ---------------------------
// Utility Functions
// ---------------------------
vector<string> readLogFile(const string &filePath) {
    vector<string> lines;
    ifstream file(filePath);
    string line;
    while(getline(file, line)) {
        if(!line.empty())
            lines.push_back(line);
    }
    return lines;
}

string readFileContents(const string &filePath) {
    ifstream ifs(filePath);
    stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

string escapeShellArg(const string &s) {
    string res;
    for (char c : s) {
        if (c == '\'')
            res += "'\\''";
        else
            res += c;
    }
    return res;
}

bool fileExists(const string &filePath) {
    return fs::exists(filePath);
}

// ---------------------------
// USB Camera Detection
// ---------------------------
bool isCameraDetected() {
    return fileExists("/dev/video0");
}

// ---------------------------
// Sensor Data Processing
// ---------------------------
// 1. Heart Rate: moving average with Z-score outlier removal.
pair<double, double> computeHeartRateAverages(const vector<string>& lines) {
    vector<double> bpms, spo2s;
    for (const string &line : lines) {
        double bpm = 0.0, spo2 = 0.0;
        if (sscanf(line.c_str(), "BPM: %lf, SpO2: %lf", &bpm, &spo2) == 2) {
            bpms.push_back(bpm);
            spo2s.push_back(spo2);
        }
    }
    if (bpms.empty() || spo2s.empty()) return {NAN, NAN};
    auto average = [&](const vector<double> &v) -> double {
        double sum = 0.0;
        for (double d : v) sum += d;
        return sum / v.size();
    };
    double meanBPM = average(bpms);
    double meanSpO2 = average(spo2s);
    auto stddev = [&](const vector<double> &v, double mean) -> double {
        double sum = 0.0;
        for (double d : v) sum += (d - mean) * (d - mean);
        return sqrt(sum / v.size());
    };
    double stdBPM = stddev(bpms, meanBPM);
    double stdSpO2 = stddev(spo2s, meanSpO2);
    vector<double> filteredBPM, filteredSpO2;
    for (double d : bpms)
        if (fabs(d - meanBPM) <= 2 * stdBPM)
            filteredBPM.push_back(d);
    for (double d : spo2s)
        if (fabs(d - meanSpO2) <= 2 * stdSpO2)
            filteredSpO2.push_back(d);
    double finalBPM = filteredBPM.empty() ? meanBPM : average(filteredBPM);
    double finalSpO2 = filteredSpO2.empty() ? meanSpO2 : average(filteredSpO2);
    return {finalBPM, finalSpO2};
}

// 2. Temperature: median filtering.
pair<double, double> computeTemperatureAverages(const vector<string>& lines) {
    vector<double> ambients, objects;
    for (const string &line : lines) {
        double ambient = 0.0, objectT = 0.0;
        if (sscanf(line.c_str(), "Ambient Temp: %lf C, Object Temp: %lf C", &ambient, &objectT) == 2) {
            ambients.push_back(ambient);
            objects.push_back(objectT);
        }
    }
    if (ambients.empty() || objects.empty()) return {NAN, NAN};
    sort(ambients.begin(), ambients.end());
    sort(objects.begin(), objects.end());
    double medAmbient = ambients[ambients.size() / 2];
    double medObject = objects[objects.size() / 2];
    return {medAmbient, medObject};
}

// 3. Motion: compute RMS of gyroscope values and choose the minimum RMS.
double computeBestMotionValue(const vector<string>& lines) {
    double bestRMS = 1e9;
    for (const string &line : lines) {
        double ax, ay, az, gx, gy, gz;
        if (sscanf(line.c_str(), "accel_x: %lf, accel_y: %lf, accel_z: %lf, gyro_x: %lf, gyro_y: %lf, gyro_z: %lf",
                   &ax, &ay, &az, &gx, &gy, &gz) == 6) {
            double rms = sqrt((gx*gx + gy*gy + gz*gz) / 3.0);
            if (rms < bestRMS) bestRMS = rms;
        }
    }
    return (bestRMS == 1e9) ? NAN : bestRMS;
}

// ---------------------------
// Candidate Selection Functions
// ---------------------------
// Compute confidence percentage using sensor thresholds.
double computeConfidence(double sensorValue, double low, double high) {
    double mid = (low + high) / 2.0;
    double rangeHalf = (high - low) / 2.0;
    double diff = fabs(sensorValue - mid);
    double conf = 100.0 - (diff / (rangeHalf + 1e-6)) * 50.0;
    if (conf < 0) conf = 0;
    if (conf > 100) conf = 100;
    return conf;
}

// For core words and emotions: iterate over each candidate JSON file.
vector<pair<string, double>> selectTopCandidates(const string &dirPath, double sensorValue, int topCount, const string &type) {
    vector<pair<string, double>> candidates;
    for (const auto &entry : fs::directory_iterator(dirPath)) {
        if (entry.path().extension() == ".json") {
            ifstream file(entry.path());
            if (!file.is_open()) continue;
            Json::Value candidate;
            file >> candidate;
            file.close();
            string label = candidate.get((type == "word" ? "word" : "emotion"), "").asString();
            if (label.empty()) continue;
            double conf1 = computeConfidence(sensorValue, candidate["heart_rate_range"][0].asDouble(), candidate["heart_rate_range"][1].asDouble());
            double conf2 = computeConfidence(sensorValue, candidate["object_temp_range"][0].asDouble(), candidate["object_temp_range"][1].asDouble());
            double conf3 = computeConfidence(sensorValue, candidate["ambient_temp_range"][0].asDouble(), candidate["ambient_temp_range"][1].asDouble());
            double conf4 = computeConfidence(98.0, candidate["spo2_range"][0].asDouble(), candidate["spo2_range"][1].asDouble());
            double avgConf = (conf1 + conf2 + conf3 + conf4) / 4.0;
            candidates.push_back({ label, avgConf });
        }
    }
    sort(candidates.begin(), candidates.end(), [](const auto &a, const auto &b) {
        return a.second > b.second;
    });
    if (candidates.size() > (size_t)topCount)
        candidates.resize(topCount);
    return candidates;
}

// For body language: choose the best candidate.
pair<string, double> selectBestBodyLanguage(const string &dirPath, double sensorValue) {
    string bestLabel = "";
    double bestConf = -1.0;
    for (const auto &entry : fs::directory_iterator(dirPath)) {
        if (entry.path().extension() == ".json") {
            ifstream file(entry.path());
            if (!file.is_open()) continue;
            Json::Value candidate;
            file >> candidate;
            file.close();
            string label = candidate.get("position", "").asString();
            if (label.empty()) continue;
            double low = candidate["motion_values"]["acceleration_x"][0].asDouble();
            double high = candidate["motion_values"]["acceleration_x"][1].asDouble();
            double conf = computeConfidence(sensorValue, low, high);
            if (conf > bestConf) {
                bestConf = conf;
                bestLabel = label;
            }
        }
    }
    return { bestLabel, bestConf };
}

// ---------------------------
// USB Camera Snapshot
// ---------------------------
bool captureCameraSnapshot(const string &imagePath) {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Error: USB camera not detected." << endl;
        return false;
    }
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        cerr << "Error: Captured empty frame from camera." << endl;
        return false;
    }
    cv::imwrite(imagePath, frame);
    return true;
}

// ---------------------------
// Document Generation
// ---------------------------
// Create a DOCX file from content via pandoc.
void createDocxDirectly(const string &docxPath, const string &content) {
    string command = "echo \"" + escapeShellArg(content) + "\" | pandoc -f markdown -o " + docxPath + " --resource-path=" + OUTPUT_DIR;
    system(command.c_str());
}

// ---------------------------
// Convert DOCX to JPG
// ---------------------------
// Convert DOCX to PDF via LibreOffice and then to JPG via ImageMagick.
void convertDocxToJpg(const string &docxPath, const string &imgPath) {
    string command = "libreoffice --headless --convert-to pdf " + docxPath + " --outdir " + OUTPUT_DIR;
    system(command.c_str());
    string pdfPath = OUTPUT_DIR + "state_profile.pdf";
    command = "convert -density 150 " + pdfPath + " -quality 90 " + imgPath;
    system(command.c_str());
    if (fs::exists(pdfPath)) fs::remove(pdfPath);
    // The DOCX is preserved.
}

// ---------------------------
// Create Log Screenshot
// ---------------------------
// Generate an image from the full contents of LOG_FILE using ImageMagick's caption.
void createLogScreenshot() {
    string logText = readFileContents(LOG_FILE);
    if (logText.empty()) logText = "No log data available.";
    replace(logText.begin(), logText.end(), '\n', ' ');
    string escapedLog = escapeShellArg(logText);
    string command = "convert -background white -fill black -font Liberation-Sans -pointsize 12 caption:\"" 
                     + escapedLog + "\" " + OUTPUT_DIR + "log_screenshot.jpg";
    system(command.c_str());
}

// ---------------------------
// Main Process
// ---------------------------
int main() {
    auto start = chrono::high_resolution_clock::now();
    
    // Clear OUTPUT_DIR so that only one DOCX and one JPG remain.
    for (const auto &entry : fs::directory_iterator(OUTPUT_DIR))
        fs::remove(entry.path());
    
    // Read sensor logs.
    vector<string> heartRateLines = readLogFile(HEART_RATE_LOG);
    vector<string> motionLines = readLogFile(MOTION_LOG);
    vector<string> tempLines = readLogFile(TEMP_LOG);
    
    // Compute sensor values.
    auto [avgBPM, avgSpO2] = computeHeartRateAverages(heartRateLines);
    double bestMotion = computeBestMotionValue(motionLines);
    auto [avgAmbient, avgObject] = computeTemperatureAverages(tempLines);
    
    // Candidate selection.
    vector<pair<string, double>> topCoreWords = selectTopCandidates(CORE_WORDS_DIR, avgAmbient, 10, "word");
    vector<pair<string, double>> topCoreEmotions = selectTopCandidates(CORE_EMOTIONS_DIR, avgBPM, 10, "emotion");
    pair<string, double> bestBodyLanguage = selectBestBodyLanguage(BODY_LANGUAGE_DIR, bestMotion);
    
    // Error stacking.
    vector<pair<string,int>> errors;
    if (topCoreWords.empty()) errors.push_back({"NO CORE WORDS DETECTED", ERR_NO_CORE_WORDS});
    if (topCoreEmotions.empty()) errors.push_back({"NO CORE EMOTIONS DETECTED", ERR_NO_CORE_EMOTIONS});
    if (bestBodyLanguage.first.empty()) errors.push_back({"NO BODY LANGUAGE DETECTED", ERR_NO_BODY_LANG});
    if (!isCameraDetected()) errors.push_back({"USB CAMERA NOT DETECTED", ERR_NO_CAMERA});
    
    // Build DOCX content in Markdown.
    ostringstream docContent;
    docContent << "# State Profile\n\n";
    docContent << "## Core Words:\n";
    for (const auto &cw : topCoreWords)
        docContent << "- " << cw.first << " [" << fixed << setprecision(3) << cw.second << "%]\n";
    docContent << "\n## Core Emotions:\n";
    for (const auto &ce : topCoreEmotions)
        docContent << "- " << ce.first << " [" << fixed << setprecision(3) << ce.second << "%]\n";
    docContent << "\n## Body Language:\n- " << bestBodyLanguage.first << " [" << fixed << setprecision(3) << bestBodyLanguage.second << "%]\n\n";
    docContent << "## Prompt for Gemini:\n";
    docContent << "Based on the following, formulate a natural sentence for a fuckin cat to say!!! "
                  "It’s critical that the sentence sounds natural and flows seamlessly—avoid being overly long. "
                  "Incorporate context from the attached images and sensor data.\n\n";
    docContent << "Avoid robotic or forced phrasing. The sentence should be concise yet reflective of the cat's current state. "
                  "Use the nouns in the image for context.\n\n";
    docContent << "## Attached:\n- Screenshot of /home/m30w/log.txt\n\n";
    docContent << "## Sensor Summary:\n";
    docContent << "- Average Heart Rate: " << avgBPM << " BPM, Average SpO2: " << avgSpO2 << "\n";
    docContent << "- Median Ambient Temp: " << avgAmbient << " C, Median Object Temp: " << avgObject << " C\n";
    docContent << "- Best Motion (RMS of Gyro): " << bestMotion << "\n";
    // Append entire log.txt in small font at the bottom.
    string logText = readFileContents(LOG_FILE);
    if (logText.empty()) logText = "No log data available.";
    docContent << "\n## Log File Contents (small print):\n<small>" << logText << "</small>\n";
    
    // Define final file paths.
    string finalDocx = OUTPUT_DIR + "state_profile.docx";
    string finalImg = FINAL_IMG_NAME;
    
    // Capture USB camera snapshot.
    if (isCameraDetected()) {
        system(("fswebcam -r 640x480 --jpeg 85 -D 1 " + CAMERA_IMAGE).c_str());
    }
    
    // Create the DOCX file (directly from Markdown content).
    createDocxDirectly(finalDocx, docContent.str());
    
    // Create a log screenshot.
    createLogScreenshot();
    
    // Convert the DOCX file to JPG using our optimized pipeline.
    convertDocxToJpg(finalDocx, finalImg);
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << "State profile generated in " << duration << "ms.\n";
    cout << "DOCX: " << finalDocx << "\nJPG: " << finalImg << "\n";
    
    if (!errors.empty()) {
        cout << "\nErrors Detected:\n";
        for (const auto &err : errors)
            cout << err.first << " [Code: " << err.second << "]\n";
    }
    
    return 0;
}
