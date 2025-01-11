#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <map>
#include <queue>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <numeric>


// Node structure to define a Huffman tree node
struct Node {
    Node* left;
    Node* right;
    int freq;
    char c;

    Node(int f, char ch, Node* l = nullptr, Node* r = nullptr)
        : left(l), right(r), freq(f), c(ch) {}
};

// Comparator for the priority queue
struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

// Global variables
std::map<char, std::string> codeMap;
std::vector<char> buffer;
int inputData = 0, outputData = 0;

// Function to build the Huffman tree
Node* buildHuffmanTree(const std::map<char, int>& freqMap) {
    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;

    for (const auto& entry : freqMap) {
        pq.push(new Node(entry.second, entry.first));
    }

    while (pq.size() > 1) {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();
        Node* combined = new Node(left->freq + right->freq, '\0', left, right);
        pq.push(combined);
    }

    return pq.top();
}

// Function to build the Huffman codes
void buildCode(Node* root, const std::string& str) {
    if (!root) return;

    if (root->c) {
        codeMap[root->c] = str;
        return;
    }

    buildCode(root->left, str + "0");
    buildCode(root->right, str + "1");
}

// Function to print the Huffman codes
void printCodeTable(const std::map<char, int>& freqMap) {
    std::cout << "\n---------CODE TABLE---------\n"
              << "----------------------------\n"
              << "CHAR  FREQ  CODE\n"
              << "----------------------------\n";
    for (const auto& entry : codeMap) {
        char c = entry.first;
        if (std::isprint(c)) {
            std::cout << std::setw(4) << c << "  " << std::setw(4) << freqMap.at(c)
                      << "  " << std::setw(16) << entry.second << "\n";
        } else {
            std::cout << std::setw(4) << std::hex << static_cast<int>(c) << std::dec
                      << "  " << std::setw(4) << freqMap.at(c)
                      << "  " << std::setw(16) << entry.second << "\n";
        }
    }
    std::cout << "----------------------------\n";
}

// Function to encode the input file
void encode(std::ifstream& inFile, std::ofstream& outFile, const std::map<char, int>& freqMap) {
    inFile.clear();
    inFile.seekg(0);

    std::string encodedString;
    char c;
    while (inFile.get(c)) {
        encodedString += codeMap[c];
    }

    outFile << encodedString.size() << '\n';
    std::cout << "\nEncoded:\n" << encodedString << "\n";

    char tempChar = 0;
    int bitCount = 0;
    for (char bit : encodedString) {
        tempChar = (tempChar << 1) | (bit - '0');
        bitCount++;
        if (bitCount == 8) {
            outFile.put(tempChar);
            tempChar = 0;
            bitCount = 0;
        }
    }

    if (bitCount > 0) {
        tempChar <<= (8 - bitCount);
        outFile.put(tempChar);
    }

    outputData = (encodedString.size() + 7) / 8;
}

// Main function
int main(int argc, char* argv[]) {
    std::string fileName;
    std::map<char, int> freqMap;

    std::cout << "**********************************************************************\n"
              << "                      COMMUNICATION ENGINEERING\n"
              << "                                 SHU M.Eng\n"
              << "                             -HUFFMAN ENCODER-\n"
              << "**********************************************************************\n\n";

    if (argc == 2) {
        fileName = argv[1];
    } else {
        std::cout << "Please enter the file to be compressed: ";
        std::cin >> fileName;
    }

    if (fileName.length() >= 50) {
        std::cerr << "ERROR: Enter a file name less than 50 characters.\n";
        return 1;
    }

    std::ifstream inFile(fileName, std::ios::binary);
    if (!inFile) {
        std::cerr << "\nERROR: No such file.\n";
        return 1;
    }

    char c;
    while (inFile.get(c)) {
        freqMap[c]++;
    }

    Node* root = buildHuffmanTree(freqMap);
    buildCode(root, "");
    printCodeTable(freqMap);

    std::ofstream outFile(fileName + ".huffman", std::ios::binary);
    encode(inFile, outFile, freqMap);

    std::ofstream tableFile(fileName + ".table");
    for (const auto& entry : freqMap) {
        tableFile.put(entry.second);
    }

    inputData = std::accumulate(freqMap.begin(), freqMap.end(), 0,
                            [](int sum, const std::pair<const char, int>& entry) {
                                return sum + entry.second;
                            });

    std::cout << "\nInput bytes: " << inputData << "\n";
    std::cout << "Output bytes: " << outputData << "\n";
    std::cout << "Compression ratio: " << std::fixed << std::setprecision(2)
              << ((static_cast<double>(inputData - outputData) / inputData) * 100) << "%\n\n";

    return 0;
}
