#include <chip8cpp_app/app.hpp>

#include <iostream>

int main(int argc, char* argv[])
try
{
    chip8cpp_app::App app;

    if (!app.init(argc, argv))
    {
        return 1;
    }

    app.run();

    return 0;
}
catch (const std::exception& e)
{
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
}
catch (...)
{
    std::cerr << "Unknown exception occurred." << std::endl;
    return 1;
}