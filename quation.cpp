#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
using namespace std;
struct Root {
    int x;
    long long y;
    int base;
    string value;
};
// convert char to int for base conversion
int charToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return -1;
}
// convert string from base to decimal
long long baseToDecimal(string value, int base) {
    long long result = 0;
    long long power = 1;
    for (int i = value.length() - 1; i >= 0; i--) {
        int digit = charToInt(value[i]);
        if (digit == -1 || digit >= base) {
            cout << "Invalid digit: " << value[i] << " in base " << base << endl;
            return -1;
        }
        result += digit * power;
        power *= base;
    }
    return result;
}
// parse the json file
void parseJSON(string filename, int& n, int& k, vector<Root>& roots) {
    ifstream file(filename);
    string line;
    bool inKeys = false;
    int currentRoot = -1;
    if (!file.is_open()) {
        cout << "Cant open file " << filename << endl;
        return;
    }
    while (getline(file, line)) {
        // remove spaces
        line.erase(remove(line.begin(), line.end(), ' '), line.end());
        line.erase(remove(line.begin(), line.end(), '\t'), line.end());
        if (line.empty()) continue;
        // check for keys section
        if (line.find("\"keys\":{") != string::npos) {
            inKeys = true;
            continue;
        }
        if (inKeys && line == "},") {
            inKeys = false;
            continue;
        }
        // get n and k
        if (inKeys) {
            line.erase(remove(line.begin(), line.end(), '"'), line.end());
            if (line.find("n:") != string::npos) {
                n = stoi(line.substr(line.find("n:") + 2));
            } else if (line.find("k:") != string::npos) {
                k = stoi(line.substr(line.find("k:") + 2));
            }
            continue;
        }
        // check for root entry
        if (line.find("\"") != string::npos && line.find(":{") != string::npos) {
            string rootNum = line.substr(line.find("\"") + 1, line.find("\"", line.find("\"") + 1) - line.find("\"") - 1);
            currentRoot = stoi(rootNum);
            continue;
        }
        // parse root data
        if (currentRoot != -1) {
            line.erase(remove(line.begin(), line.end(), '"'), line.end());
            if (line.find("base:") != string::npos) {
                string baseStr = line.substr(line.find("base:") + 5);
                if (baseStr.back() == ',') baseStr.pop_back();
                Root root;
                root.x = currentRoot;
                root.base = stoi(baseStr);
                // get value from next line
                string nextLine;
                getline(file, nextLine);
                nextLine.erase(remove(nextLine.begin(), nextLine.end(), ' '), nextLine.end());
                nextLine.erase(remove(nextLine.begin(), nextLine.end(), '"'), nextLine.end());
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
// lagrange interpolation
vector<long long> lagrange(vector<Root>& roots) {
    int n = roots.size();
    vector<long long> coeff(n, 0);
    for (int i = 0; i < n; i++) {
        vector<long long> term(n, 0);
        term[0] = 1;
        long long denom = 1;
        for (int j = 0; j < n; j++) {
            if (i == j) continue;     
            // multiply by (x - x_j)
            vector<long long> temp(n, 0);
            for (int k = 0; k < n - 1; k++) {
                temp[k + 1] += term[k];
                temp[k] -= term[k] * roots[j].x;
            }
            term = temp;
            denom *= (roots[i].x - roots[j].x);
        }
        // add to coefficients
        for (int k = 0; k < n; k++) {
            coeff[k] += (term[k] * roots[i].y) / denom;
        }
    }
    return coeff;
}
// print the polynomial
void printPoly(vector<long long>& coeff) {
    cout << "Polynomial: ";
    bool first = true;
    for (int i = coeff.size() - 1; i >= 0; i--) {
        if (coeff[i] == 0) continue;
        if (!first && coeff[i] > 0) cout << " + ";
        if (coeff[i] < 0) cout << " - ";
        if (abs(coeff[i]) != 1 || i == 0) {
            cout << abs(coeff[i]);
        }
        if (i > 1) cout << "x^" << i;
        else if (i == 1) cout << "x";
        first = false;
    }
    cout << endl;
}
int main() {
    cout << "Polynomial Interpolation Solver" << endl;
    // try different filenames
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
    cout << "Reading from " << filename << endl;
    int n, k;
    vector<Root> roots;
    parseJSON(filename, n, k, roots);
    if (roots.empty()) {
        cout << "No roots found!" << endl;
        return 1;
    }
    cout << "n = " << n << ", k = " << k << endl;
    cout << "Degree = " << (k - 1) << endl << endl;
    // show roots
    for (auto& root : roots) {
        cout << "Root " << root.x << ": (" << root.x << ", " << root.value 
             << " base " << root.base << " = " << root.y << ")" << endl;
    }  
    if (roots.size() < k) {
        cout << "Not enough roots! Need " << k << ", got " << roots.size() << endl;
        return 1;
    }
    cout << "\nUsing " << k << " roots..." << endl;
    // use first k roots
    vector<Root> selected(roots.begin(), roots.begin() + k);
    // calculate coefficients
    vector<long long> coeff = lagrange(selected);
    cout << "\nResults:" << endl;
    printPoly(coeff);
    // secret is constant term
    cout << "Secret (c) = " << coeff[0] << endl; 
    // verify
    cout << "\nVerification:" << endl;
    for (auto& root : selected) {
        long long result = 0;
        long long power = 1;
        for (int i = 0; i < coeff.size(); i++) {
            result += coeff[i] * power;
            power *= root.x;
        }
        cout << "f(" << root.x << ") = " << result << " (expected: " << root.y << ")" << endl;
    }
    return 0;
}
