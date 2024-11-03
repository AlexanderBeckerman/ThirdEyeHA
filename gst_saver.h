#pragma once
#include <gst/gst.h>
#include <iostream>
#include <sstream>
#include <string>

class GstSaver {
  public:
    GstSaver(int pattern, int bitrate, int frames_per_file);
    ~GstSaver();

    void Init();
    void Write();
    void Stop();

  private:
    GstElement *pipeline_;
    GstElement *filesink_;
    int pattern_;
    int bitrate_;
    int frames_per_file_;
    int frame_count_;
    int file_index_;
    bool eos_received_;

    void set_output_file();
    static GstPadProbeReturn frame_probe_callback(GstPad *pad,
                                                  GstPadProbeInfo *info,
                                                  gpointer user_data);

    static GstPadProbeReturn
    eos_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    int create_pipeline();
    void rotate_pipeline();
    static gboolean bus_callback(GstBus *bus, GstMessage *msg, gpointer data);
};
