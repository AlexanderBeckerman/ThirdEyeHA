#include "gst_saver.h"

GstSaver::GstSaver(int pattern, int bitrate, int frames_per_file)
    : pattern_(pattern), bitrate_(bitrate), frames_per_file_(frames_per_file),
      frame_count_(0), file_index_(0), pipeline_(nullptr),
      eos_received_(false) {}

GstSaver::~GstSaver() {
    // Destory and clean up pipleine on destructing.
    Stop();
}

void GstSaver::Init() {
    gst_init(nullptr, nullptr);
    int ret = create_pipeline(); // Create the pipeline.
    if (ret < 0) {
        g_printerr("Error creating pipeline!\n");
        return;
    }
    ret = gst_element_set_state(pipeline_,
                                GST_STATE_PLAYING); // Start the pipeline.
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Error setting pipeline to PLAYING state!\n");
        g_object_unref(pipeline_);
        return;
    }
}

int GstSaver::create_pipeline() {

    /* Here I used filesink instead of multifilesink because I couldn't open the
     files multifilesink generated for some reason. The files weren't
     corrupted but would just open a blank black screen video that didn't play.
   */

    std::ostringstream pipeline_str;
    pipeline_str
        << "videotestsrc name=videotestsrc0 pattern=" << pattern_
        << " ! video/x-raw,width=1280,height=720,framerate=30/1 "
        << "! x264enc bitrate=" << bitrate_
        << " ! mp4mux name=muxer ! filesink name=filesink location=output_"
        << pattern_ << "_" << file_index_ << ".mp4";

    GError *error = nullptr;
    pipeline_ = gst_parse_launch(pipeline_str.str().c_str(), &error);
    filesink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "filesink");

    if (error) {
        g_printerr("Error creating pipeline!\n");
        return -1;
    }

    // Add a bus callback to watch for EOS when frames_per_file frames reached.
    GstBus *bus = gst_element_get_bus(pipeline_);
    gst_bus_add_watch(bus, (GstBusFunc)bus_callback, this);
    gst_object_unref(bus);

    // Attach a probe to count frames on the source pad.
    GstElement *source =
        gst_bin_get_by_name(GST_BIN(pipeline_), "videotestsrc0");
    if (source) {
        GstPad *pad = gst_element_get_static_pad(source, "src");
        if (pad) {
            gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
                              frame_probe_callback, this, NULL);
            gst_object_unref(pad);
        } else {
            g_printerr("Error: Could not get source pad.\n");
            return -1;
        }
        gst_object_unref(source);
    } else {
        g_printerr("Error: Could not get source element.\n");
        return -1;
    }
    return 0;
}

void GstSaver::Write() {
    frame_count_++;

    // Check if we reached the needed amount of frames.
    if (frame_count_ >= frames_per_file_ && !eos_received_) {
        frame_count_ = 0;
        file_index_++;
        eos_received_ = true;
        std::cout << "Rotating to output_" << file_index_ << ".mp4"
                  << std::endl;
        // Send EOS to finalize the current file.
        gst_element_send_event(pipeline_, gst_event_new_eos());
    }
}

void GstSaver::Stop() {
    if (pipeline_) {
        // Send EOS to finalize the current file.
        gst_element_send_event(pipeline_, gst_event_new_eos());

        // Wait for EOS.
        GstBus *bus = gst_element_get_bus(pipeline_);
        GstMessage *msg = gst_bus_timed_pop_filtered(
            bus, GST_CLOCK_TIME_NONE,
            static_cast<GstMessageType>(GST_MESSAGE_EOS));
        if (msg) {
            gst_message_unref(msg);
        }
        gst_object_unref(bus);

        // Clean up pipeline.
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
}
/* Here changing the location property without changing the filesink element
   didn't work for me, so I had to destory the element, create a new one and
   link it back to the pipeline as shown here, under "dynamically switching an
   element in a PLAYING pipeline":
   https://gstreamer.freedesktop.org/documentation/additional/design/probes.html?gi-language=c
*/
void GstSaver::rotate_pipeline() {

    if (filesink_ && pipeline_) {
        // Pause the entire pipeline to safely change the filesink location.
        gst_element_set_state(pipeline_, GST_STATE_NULL);

        // Get the mp4muxer element from the pipeline.
        GstElement *muxer = gst_bin_get_by_name(GST_BIN(pipeline_), "muxer");
        if (!muxer) {
            g_printerr("Error: Could not find muxer element in pipeline.\n");
            return;
        }

        // Unlink the current filesink from muxer.
        gst_element_unlink(muxer, filesink_);

        // Remove the old filesink.
        gst_element_set_state(filesink_, GST_STATE_NULL);
        gst_bin_remove(GST_BIN(pipeline_), filesink_);

        // Create a new filesink with the updated file location.
        filesink_ = gst_element_factory_make("filesink", "sink");
        set_output_file(); // Set the new location on the new filesink
        gst_bin_add(GST_BIN(pipeline_), filesink_);

        // Link the new filesink to muxer
        if (!gst_element_link(muxer, filesink_)) {
            g_printerr("Error: Could not link muxer to new filesink.\n");
        }

        gst_object_unref(muxer);
        gst_element_set_state(pipeline_, GST_STATE_PLAYING);

        // Reset EOS flag for the next rotation
        std::cout << "Finished rotating to output_" << file_index_ << ".mp4"
                  << std::endl;
        eos_received_ = false;
    } else {
        g_printerr("Error: filesink or pipeline is null in rotate_sink().\n");
    }
}

void GstSaver::set_output_file() {
    std::ostringstream file_path;
    file_path << "output_" << pattern_ << "_" << file_index_ << ".mp4";
    g_object_set(filesink_, "location", file_path.str().c_str(), NULL);
}

// Calls Write() to count frames.
GstPadProbeReturn GstSaver::frame_probe_callback(GstPad *pad,
                                                 GstPadProbeInfo *info,
                                                 gpointer user_data) {
    GstSaver *saver = static_cast<GstSaver *>(user_data);
    saver->Write();
    return GST_PAD_PROBE_OK;
}

// Calls rotate_pipeline() to rotate the output file.
gboolean GstSaver::bus_callback(GstBus *bus, GstMessage *msg, gpointer data) {
    GstSaver *saver = static_cast<GstSaver *>(data);

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS && saver->eos_received_) {
        saver->rotate_pipeline(); // If we got an EOS then rotate to next file.
    } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
        GError *err;
        gst_message_parse_error(msg, &err, nullptr);
        g_printerr("Error: %s\n", err->message);
        g_clear_error(&err);
    }

    return TRUE;
}
