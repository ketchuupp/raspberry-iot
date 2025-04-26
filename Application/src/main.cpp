#include "App/app.h" // Include the App header from include/App/
#include <iostream>
#include <stdexcept>

int main() {
    try {
        // Create the application object which handles initialization
        SensorHub::App::App app; // Fully qualify or use 'using namespace SensorHub::App;'

        // Run the application's main loop
        return app.run();

    } catch (const std::exception& e) {
        std::cerr << "Critical Application Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Caught unknown critical exception. Exiting." << std::endl;
        return 1;
    }
}
