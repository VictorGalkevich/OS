#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <fstream>
#include <Windows.h>
#include <time.h>
using namespace std;


struct CodeTreeNode {
    char content;
    int weight;
    CodeTreeNode* left;
    CodeTreeNode* right;

    CodeTreeNode(char content, int weight) {
        this->content = content;
        this->weight = weight;
        this->left = nullptr;
        this->right = nullptr;
    }

    CodeTreeNode(char content, int weight, CodeTreeNode* left, CodeTreeNode* right) {
        this->content = content;
        this->weight = weight;
        this->left = left;
        this->right = right;
    }

    // извлечение кода для символа
    string getCodeForCharacter(char ch, string parentPath) {
        if (content == ch) {
            return parentPath;
        }
        else {
            if (left != nullptr) {
                string path = left->getCodeForCharacter(ch, parentPath + '0');
                if (!path.empty()) {
                    return path;
                }
            }
            if (right != nullptr) {
                string path = right->getCodeForCharacter(ch, parentPath + '1');
                if (!path.empty()) {
                    return path;
                }
            }
        }
        return "";
    }
};

bool comp(CodeTreeNode* a, CodeTreeNode* b) {
    return a->weight - b->weight > 0;
}

map<char, int> countFrequency(const string& text) {
    map<char, int> freqMap;
    for (char c : text) {
        freqMap[c]++;
    }
    return freqMap;
}

CodeTreeNode* huffman(vector<CodeTreeNode*>& codeTreeNodes) {
    while (codeTreeNodes.size() > 1) {
        sort(codeTreeNodes.begin(), codeTreeNodes.end(), comp);

        CodeTreeNode* left = codeTreeNodes.back();
        codeTreeNodes.pop_back();
        CodeTreeNode* right = codeTreeNodes.back();
        codeTreeNodes.pop_back();

        CodeTreeNode* parent = new CodeTreeNode('\0', right->weight + left->weight, left, right);
        codeTreeNodes.push_back(parent);
    }
    return codeTreeNodes[0];
}

string huffmanDecode(const string& encoded, CodeTreeNode* tree) {
    string decoded;
    CodeTreeNode* node = tree;

    for (char ch : encoded) {
        node = (ch == '0') ? node->left : node->right;

        if (node->content != '\0') {
            decoded += node->content;
            node = tree;
        }
    }
    return decoded;
}

class BitArray {
private:
    int size;
    char* bytes;
    char masks[8] = { 0b00000001, 0b00000010, 0b00000100, 0b00001000,
                     0b00010000, 0b00100000, 0b01000000, (char)0b10000000 };

public:
    BitArray(int size) {
        this->size = size;
        int sizeInBytes = size / 8;
        if (size % 8 > 0) {
            sizeInBytes = sizeInBytes + 1;
        }
        bytes = new char[sizeInBytes]();
    }

    BitArray(int size, char* bytes) {
        this->size = size;
        this->bytes = bytes;
    }

    int get(int index) {
        int byteIndex = index / 8;
        int bitIndex = index % 8;
        return (bytes[byteIndex] & masks[bitIndex]) != 0 ? 1 : 0;
    }

    void set(int index, int value) {
        int byteIndex = index / 8;
        int bitIndex = index % 8;
        if (value != 0) {
            bytes[byteIndex] = (char)(bytes[byteIndex] | masks[bitIndex]);
        }
        else {
            bytes[byteIndex] = (char)(bytes[byteIndex] & ~masks[bitIndex]);
        }
    }

    int getSize() {
        return size;
    }

    int getSizeInBytes() {
        int sizeInBytes = size / 8;
        if (size % 8 > 0) {
            sizeInBytes = sizeInBytes + 1;
        }
        return sizeInBytes;
    }

    char* getBytes() {
        return bytes;
    }
};

void saveToFile(const string& output, const map<char, int>& frequencies, const string& bits) {
    try {
        ofstream os(output, ios::binary);
        int frequencyTableSize = frequencies.size();
        os.write((char*)&frequencyTableSize, sizeof(int));
        for (const auto& pair : frequencies) {
            os.write((char*)&pair.first, sizeof(char));
            os.write((char*)&pair.second, sizeof(int));
        }
        int compressedSizeBits = bits.length();
        BitArray bitArray(compressedSizeBits);
        for (int i = 0; i < bits.length(); i++) {
            bitArray.set(i, bits[i] != '0' ? 1 : 0);
        }

        os.write((char*)&compressedSizeBits, sizeof(int));
        os.write(bitArray.getBytes(), bitArray.getSizeInBytes());
        os.close();
    }
    catch (const exception& e) {
        cerr << "Error saving to file: " << e.what() << endl;
    }
}

void loadFromFile(const string& input, map<char, int>& frequencies, string& bits) {
    try {
        ifstream is(input, ios::binary);
        int frequencyTableSize;
        is.read((char*)&frequencyTableSize, sizeof(int));
        for (int i = 0; i < frequencyTableSize; i++) {
            char character;
            int frequency;
            is.read((char*)&character, sizeof(char));
            is.read((char*)&frequency, sizeof(int));
            frequencies[character] = frequency;
        }
        int dataSizeBits;
        is.read((char*)&dataSizeBits, sizeof(int));
        BitArray bitArray(dataSizeBits);
        is.read(bitArray.getBytes(), bitArray.getSizeInBytes());
        is.close();

        for (int i = 0; i < bitArray.getSize(); i++) {
            bits += bitArray.get(i) != 0 ? '1' : '0';
        }
    }
    catch (const exception& e) {
        cerr << "Error loading from file: " << e.what() << endl;
    }
}

void writeToFile(const std::wstring& filename, const std::wstring& content) {
    HANDLE fileHandle = CreateFileW(
        filename.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten = 0;
        WriteFile(
            fileHandle,
            content.c_str(),
            static_cast<DWORD>(content.size() * sizeof(wchar_t)),
            &bytesWritten,
            nullptr
        );

        CloseHandle(fileHandle);
    }
}

std::string readFromFile(const std::wstring& filename) {
    HANDLE fileHandle = CreateFileW(
        filename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    std::string content;

    if (fileHandle != INVALID_HANDLE_VALUE) {
        DWORD fileSize = GetFileSize(fileHandle, nullptr);
        if (fileSize != INVALID_FILE_SIZE) {
            HANDLE fileMappingHandle = CreateFileMappingW(
                fileHandle,
                nullptr,
                PAGE_READONLY,
                0,
                0,
                nullptr
            );

            if (fileMappingHandle != nullptr) {
                LPVOID fileMapView = MapViewOfFile(
                    fileMappingHandle,
                    FILE_MAP_READ,
                    0,
                    0,
                    0
                );

                if (fileMapView != nullptr) {
                    const char* fileContent = static_cast<const char*>(fileMapView);
                    content.assign(fileContent, fileSize);

                    UnmapViewOfFile(fileMapView);
                }

                CloseHandle(fileMappingHandle);
            }
        }

        CloseHandle(fileHandle);
    }

    return content;
}


int main() {
    try {
        string inputFileName;
        cout << "Input file name to compress: ";
        cin >> inputFileName;
        std::wstring inpt(inputFileName.begin(), inputFileName.end());
        clock_t start = clock();

        string content = readFromFile(inpt);

        /*ifstream inputFile(inputFileName, ios::binary);
        if (!inputFile) {
            cerr << "Unable to open file." << endl;
            return 1;
        }

        string content((istreambuf_iterator<char>(inputFile)), (istreambuf_iterator<char>()));
        inputFile.close();*/

        clock_t end = clock();
        double seconds = (double)(end - start) / CLOCKS_PER_SEC;
        cout << seconds << endl;
        std::map<char, int> frequencies = countFrequency(content);

        std::vector<CodeTreeNode*> codeTreeNodes;

        for (auto const& element : frequencies) {
            codeTreeNodes.push_back(new CodeTreeNode(element.first, element.second));
        }

        CodeTreeNode* tree = huffman(codeTreeNodes);

        std::map<char, std::string> codes;

        for (auto const& element : frequencies) {
            codes[element.first] = tree->getCodeForCharacter(element.first, "");
        }

        std::string encoded;

        for (int i = 0; i < content.length(); i++) {
            encoded += codes[content[i]];
        }

        saveToFile("compressed.txt", frequencies, encoded);

        std::map<char, int> frequencies2;
        std::string encoded2;
        codeTreeNodes.clear();

        loadFromFile("compressed.txt", frequencies2, encoded2);

        for (const auto& pair : frequencies2) {
            codeTreeNodes.push_back(new CodeTreeNode(pair.first, pair.second));
        }
        CodeTreeNode* tree2 = huffman(codeTreeNodes);

        std::string decoded = huffmanDecode(encoded2, tree2);

        string outputFileName;
        cout << "Input file name to save decompressed: ";
        cin >> outputFileName;


        std::wstring otpt(outputFileName.begin(), outputFileName.end());
        std::wstring dcdd(decoded.begin(), decoded.end());

        clock_t start2 = clock();

        std::ofstream out;          // поток для записи
        out.open(outputFileName);      // открываем файл для записи
        if (out.is_open())
        {
            out << decoded << std::endl;
        }
        out.close();

        /*writeToFile(otpt, dcdd);*/

        clock_t end2 = clock();
        double seconds2 = (double)(end2 - start2) / CLOCKS_PER_SEC;
        cout << seconds2 << endl;
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}