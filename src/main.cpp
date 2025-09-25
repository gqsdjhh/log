#include "log.hpp"

int main() {
    try{
        Logger logger("log.txt");
        logger.log("Welcome to Alliance");
        logger.log("Logging ID: {}", 111);
        logger.log("User {} has logged in from {}", "444", "456");
    }
    catch(const std::exception& error){
        std::cerr << "Error: " << error.what() << std::endl;
    }
    
    return 0;
}