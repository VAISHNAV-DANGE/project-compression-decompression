#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cstring>
#include <string>
#include <stdexcept>
#include <sys/stat.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h> // For _mkdir on Windows
#endif

struct Node {
    char c;             // Character (leaf nodes)
    int freq;           // Frequency
    Node* left;         // Left child
    Node* right;        // Right child

    Node(int frequency, char character = '\0', Node* l = nullptr, Node* r = nullptr)
        : c(character), freq(frequency), left(l), right(r) {}
};

// Comparison function for priority queue
struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

class HuffmanDecoder {
private:
    std::priority_queue<Node*, std::vector<Node*>, Compare> pq;
    Node* root = nullptr;

    // Build Huffman Tree
    void buildTree(const std::vector<int>& freq) {
        for (int i = 0; i < 128; ++i) {
            if (freq[i] > 0) {
                pq.push(new Node(freq[i], static_cast<char>(i)));
            }
        }

        while (pq.size() > 1) {
            Node* left = pq.top();
            pq.pop();
            Node* right = pq.top();
            pq.pop();
            Node* parent = new Node(left->freq + right->freq, '\0', left, right);
            pq.push(parent);
        }

        root = pq.top();
    }

    // Decode the Huffman encoded file
    void decodeFile(std::istream& in, std::ostream& out, int length) {
        Node* current = root;
        char byte;
        int bitCount = 0;

        for (int i = 0; i < length; ++i) {
            if (bitCount == 0) {
                in.get(byte);
                if (in.eof()) break;
                bitCount = 8;
            }

            if (byte & (1 << (--bitCount))) {
                current = current->right;
            } else {
                current = current->left;
            }

            if (current->c) { // Reached a leaf node
                out.put(current->c);
                current = root;
            }
        }
    }

public:
    // Import frequency table and build Huffman tree
    void importTable(std::istream& tableFile, std::vector<int>& freq) {
        char c;
        int i = 0;
        while (tableFile.get(c)) {
            freq[i++] = static_cast<unsigned char>(c);
        }

        buildTree(freq);
    }

    // Main decoding function
    void decode(const std::string& inputFileName) {
        std::ifstream inFile(inputFileName, std::ios::binary);
        if (!inFile.is_open()) {
            throw std::runtime_error("ERROR: Unable to open input file.");
        }

        // Read the length of the encoded data
        int length;
        inFile >> length;
        inFile.ignore(); // Skip newline character

        std::string outputFileName = inputFileName + ".decoded";
        std::ofstream outFile(outputFileName, std::ios::binary);
        if (!outFile.is_open()) {
            throw std::runtime_error("ERROR: Unable to open output file.");
        }

        decodeFile(inFile, outFile, length);

        inFile.close();
        outFile.close();

        std::cout << "Decoded file saved as: " << outputFileName << std::endl;
    }

    ~HuffmanDecoder() {
        // Cleanup memory
        std::queue<Node*> nodes;
        if (root) nodes.push(root);

        while (!nodes.empty()) {
            Node* n = nodes.front();
            nodes.pop();

            if (n->left) nodes.push(n->left);
            if (n->right) nodes.push(n->right);
            delete n;
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            throw std::invalid_argument("ERROR: Please provide the input file name.");
        }

        std::string fileName(argv[1]);
        std::ifstream tableFile(fileName + ".table", std::ios::binary);
        if (!tableFile.is_open()) {
            throw std::runtime_error("ERROR: Frequency table cannot be found.");
        }

        HuffmanDecoder decoder;
        std::vector<int> freq(128, 0);

        decoder.importTable(tableFile, freq);
        tableFile.close();

        decoder.decode(fileName);

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
