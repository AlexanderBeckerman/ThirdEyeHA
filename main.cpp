#include "gst_saver.h"
#include <csignal>

GMainLoop *main_loop =
    nullptr; // This is bad practice, but I wasn't able to stop the loop and
             // finalize the last file cleanly without the signal handler.

// Signal handler to stop the main loop on SIGINT (Ctrl+C)
void handle_signal(int signal) {
    if (main_loop != nullptr) {
        g_main_loop_quit(main_loop);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <pattern> <bitrate> <frames_per_file>" << std::endl;
        return -1;
    }

    int pattern = std::stoi(argv[1]);
    int bitrate = std::stoi(argv[2]);
    int frames_per_file = std::stoi(argv[3]);

    std::signal(SIGINT, handle_signal);

    GstSaver saver(pattern, bitrate, frames_per_file);
    saver.Init(); // Create the pipeline.

    main_loop = g_main_loop_new(NULL, FALSE);
    std::cout << "Running... Press Ctrl+C to stop." << std::endl;
    g_main_loop_run(main_loop);
    g_main_loop_unref(main_loop);
    main_loop = nullptr;

    return 0;
}
