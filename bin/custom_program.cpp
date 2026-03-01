#include <iostream>
#include <string>

int main(int argc, char** argv) {
    // Note: argc includes the program name itself as the first argument.
    // If you pass "Hi there!", argc will be 2.
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <message>" << std::endl;
        return 1;
    }

    // Output the specific text you requested
    std::cout << "I am a custom program. You passed in " << (argc - 1) 
              << " command line argument:" << std::endl;

    // argv[1] is the first actual argument passed ("Hi there!")
    std::cout << argv[1] << std::endl;

    return 0;
}