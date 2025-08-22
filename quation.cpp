#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <cctype>

using namespace std;

// Structure to hold root data
struct Root {
    int x;
    long long y;
    int base;
    string value;
};

// Function to convert character to integer for base conversion
int charToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return -1;
}

// Function to convert string from given base to decimal
long long baseToDecimal(const string& value, int base) {
    long long result = 0;
    long long power = 1;
    
    for (int i = value.length() - 1; i >= 0; i--) {
        int digit = charToInt(value[i]);
        if (digit == -1 || digit >= base) {
            cout << "Invalid digit in base " << base << ": " << value[i] << endl;
            return -1;
        }
        result += digit * power;
        power *= base;
    }
    return result;
}

// Function to parse JSON-like input (improved parser)
map<string, string> parseJSON(const string& filename) {
    map<string, string> data;
    ifstream file(filename);
    string line;
    
    if (!file.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return data;
    }
    
    while (getline(file, line)) {
        // Remove whitespace
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        if (line.empty()) continue;
        
        // Skip lines that are just brackets or commas
        if (line == "{" || line == "}" || line == ",") continue;
        
        // Remove quotes and brackets
        line.erase(remove_if(line.begin(), line.end(), [](char c) { return c == '"'; }), line.end());
        
        if (line.find(':') == string::npos) continue;
        
        size_t colonPos = line.find(':');
        string key = line.substr(0, colonPos);
        string value = line.substr(colonPos + 1);
        
        // Remove trailing comma if present
        if (!value.empty() && value.back() == ',') {
            value.pop_back();
        }
        
        // Skip nested objects for now, we'll handle them separately
        if (value == "{") continue;
        
        data[key] = value;
    }
    
    file.close();
    return data;
}

// Function to extract nested JSON data
void extractNestedData(const string& filename, int& n, int& k, vector<Root>& roots) {
    ifstream file(filename);
    string line;
    bool inKeys = false;
    bool inRoot = false;
    int currentRoot = -1;
    
    if (!file.is_open()) {
        cout << "Error: Could not open file " << filename << endl;
        return;
    }
    
    while (getline(file, line)) {
        // Remove whitespace
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        if (line.empty()) continue;
        
        // Check if we're entering keys section
        if (line.find("\"keys\":{") != string::npos) {
            inKeys = true;
            continue;
        }
        
        // Check if we're leaving keys section
        if (inKeys && line == "},") {
            inKeys = false;
            continue;
        }
        
        // Parse keys
        if (inKeys) {
            line.erase(remove_if(line.begin(), line.end(), [](char c) { return c == '"'; }), line.end());
            if (line.find("n:") != string::npos) {
                n = stoi(line.substr(line.find("n:") + 2));
            } else if (line.find("k:") != string::npos) {
                k = stoi(line.substr(line.find("k:") + 2));
            }
            continue;
        }
        
        // Check if we're entering a root section
        if (line.find("\"") != string::npos && line.find(":{") != string::npos) {
            string rootNum = line.substr(line.find("\"") + 1, line.find("\"", line.find("\"") + 1) - line.find("\"") - 1);
            currentRoot = stoi(rootNum);
            inRoot = true;
            continue;
        }
        
        // Check if we're leaving a root section
        if (inRoot && line == "},") {
            inRoot = false;
            currentRoot = -1;
            continue;
        }
        
        // Parse root data
        if (inRoot && currentRoot != -1) {
            line.erase(remove_if(line.begin(), line.end(), [](char c) { return c == '"'; }), line.end());
            
            if (line.find("base:") != string::npos) {
                string baseStr = line.substr(line.find("base:") + 5);
                if (baseStr.back() == ',') baseStr.pop_back();
                
                Root root;
                root.x = currentRoot;
                root.base = stoi(baseStr);
                
                // Find the value in the next line
                string nextLine;
                getline(file, nextLine);
                nextLine.erase(remove_if(nextLine.begin(), nextLine.end(), ::isspace), nextLine.end());
                nextLine.erase(remove_if(nextLine.begin(), nextLine.end(), [](char c) { return c == '"'; }), nextLine.end());
                
                if (nextLine.find("value:") != string::npos) {
                    string valueStr = nextLine.substr(nextLine.find("value:") + 6);
                    if (valueStr.back() == ',') valueStr.pop_back();
                    
                    root.value = valueStr;
                    root.y = baseToDecimal(valueStr, root.base);
                    
                    if (root.y != -1) {
                        roots.push_back(root);
                    }
                }
            }
        }
    }
    
    file.close();
}

// Function to calculate Lagrange interpolation
vector<long long> lagrangeInterpolation(const vector<Root>& roots) {
    int n = roots.size();
    vector<long long> coefficients(n, 0);
    
    // For each term in the polynomial
    for (int i = 0; i < n; i++) {
        vector<long long> term(n, 0);
        term[0] = 1; // Start with 1
        
        long long denominator = 1;
        
        // Calculate (x - x_j) / (x_i - x_j) for all j != i
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            
            // Multiply by (x - x_j)
            vector<long long> temp(n, 0);
            for (int k = 0; k < n - 1; k++) {
                temp[k + 1] += term[k];
                temp[k] -= term[k] * roots[j].x;
            }
            term = temp;
            
            // Divide by (x_i - x_j)
            denominator *= (roots[i].x - roots[j].x);
        }
        
        // Multiply by y_i and add to coefficients
        for (int k = 0; k < n; k++) {
            coefficients[k] += (term[k] * roots[i].y) / denominator;
        }
    }
    
    return coefficients;
}

// Function to print polynomial
void printPolynomial(const vector<long long>& coefficients) {
    cout << "Polynomial: ";
    bool first = true;
    
    for (int i = coefficients.size() - 1; i >= 0; i--) {
        if (coefficients[i] == 0) continue;
        
        if (!first && coefficients[i] > 0) cout << " + ";
        if (coefficients[i] < 0) cout << " - ";
        
        if (abs(coefficients[i]) != 1 || i == 0) {
            cout << abs(coefficients[i]);
        }
        
        if (i > 1) cout << "x^" << i;
        else if (i == 1) cout << "x";
        
        first = false;
    }
    cout << endl;
}

int main() {
    cout << "=== Polynomial Interpolation Solver ===" << endl;
    
    // Try to read from testcase.json first, then testcase1.json
    string filename = "testcase.json";
    ifstream testFile(filename);
    if (!testFile.good()) {
        filename = "testcase1.json";
        testFile.close();
        testFile.open(filename);
        if (!testFile.good()) {
            filename = "testcase2.json";
        }
    }
    testFile.close();
    
    cout << "Reading input from " << filename << "..." << endl;
    
    // Extract data from JSON file
    int n, k;
    vector<Root> roots;
    extractNestedData(filename, n, k, roots);
    
    if (roots.empty()) {
        cout << "Error: No valid roots found in JSON file" << endl;
        return 1;
    }
    
    cout << "n = " << n << " (number of roots)" << endl;
    cout << "k = " << k << " (minimum roots needed)" << endl;
    cout << "Polynomial degree = " << (k - 1) << endl << endl;
    
    // Display decoded roots
    for (const Root& root : roots) {
        cout << "Root " << root.x << ": x = " << root.x << ", y = " << root.value 
             << " (base " << root.base << ") = " << root.y << endl;
    }
    
    if (roots.size() < k) {
        cout << "Error: Not enough valid roots. Need " << k << ", got " << roots.size() << endl;
        return 1;
    }
    
    cout << "\nUsing " << k << " roots for interpolation..." << endl;
    
    // Use only the first k roots
    vector<Root> selectedRoots(roots.begin(), roots.begin() + k);
    
    // Calculate polynomial coefficients using Lagrange interpolation
    vector<long long> coefficients = lagrangeInterpolation(selectedRoots);
    
    cout << "\n=== Results ===" << endl;
    printPolynomial(coefficients);
    
    // The constant term (c) is the secret
    cout << "Secret (constant term c) = " << coefficients[0] << endl;
    
    cout << "\n=== Verification ===" << endl;
    for (const Root& root : selectedRoots) {
        long long result = 0;
        long long power = 1;
        
        for (int i = 0; i < coefficients.size(); i++) {
            result += coefficients[i] * power;
            power *= root.x;
        }
        
        cout << "f(" << root.x << ") = " << result << " (expected: " << root.y << ")" << endl;
    }
    
    return 0;
}
