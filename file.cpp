#include <iostream>
#include <string.h>
#include <fstream>

class File {
public:
    std::string fileName;
    std::string content;
    std::string filePath;
    std::string originalContent;
    bool isSaved = true;

    File(const std::string& name, const std::string& fileContent, const std::string& path) {
        fileName = name;
        content = fileContent;
        filePath = path;
        originalContent = content;
    }

    std::string getName() {
        return fileName;
    }

    std::string getPath() {
        return filePath;
    }

    void setPath(std::string newPath) {
        filePath = newPath;
    }

    bool isNull() const {
        return fileName.empty();
    }

    std::string getContent() {
        return content;
    }


    void compareContent(std::string newContent) {
        if (content == newContent) {
            isSaved = true;
        }
        else {
            isSaved = false;
        }
    }

    void saveInMemory(std::string newText) {
        isSaved = false;
        content = newText;
    }

    bool saveToFile() {
        if (isNull()) {
            std::cout << "Cannot save a null file." << std::endl;
            return false;
        }

        std::ofstream outputFile(filePath);

        if (outputFile.is_open()) {
            outputFile << content;
            outputFile.close();
            std::cout << "File saved successfully." << std::endl;
            isSaved = true;
            return true;
        }
        else {
            std::cout << "Failed to open the file for saving." << std::endl;
            return false;
        }
    }
};
