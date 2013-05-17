#include <iterator>
#include <iostream>
#include "Player.h"
#include "SinGenerator.h"

const char * Player::format = "S16LE";

const Player::sample_t Player::max_volume = std::numeric_limits<Player::sample_t>::max();

void Player::build_gst_element(GstElement* &element, const char * kind, const char * name)
{
    element = gst_element_factory_make(kind,name);
    if(element==nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
}

void Player::initialize_gst()
{
    GError *err;
    if(!gst_init_check(nullptr,nullptr,&err))
    {
        std::exit(err->code);
    }
}

Player::Player()
{
    if(!gst_is_initialized())
    {
        initialize_gst();
    }
    if(pipeline = gst_pipeline_new ("pipeline"),pipeline==nullptr)
    {
        std::exit(EXIT_FAILURE);
    };

    build_gst_element(pipeline,"pipeline","pipe");
    build_gst_element(appsrc,"appsrc","source");
    build_gst_element(conv,"audioconvert","conv");
    build_gst_element(audiosink,"autoaudiosink","output");

    g_object_set (G_OBJECT (appsrc), "caps",
            gst_caps_new_simple (
                "audio/x-raw",
                "format", G_TYPE_STRING, format,
                "rate", G_TYPE_INT, sample_rate,
                "channels",G_TYPE_INT, channels,
                "signed", G_TYPE_BOOLEAN, TRUE,
                "layout", G_TYPE_STRING, "interleaved",
                NULL),
            NULL);

    //the gstreamer main loop is the main event loop for audio generation
    loop = g_main_loop_new (NULL, FALSE);

    gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, audiosink, NULL);
    gst_element_link_many (appsrc, conv, audiosink, NULL);

    //need_data_g and enough_data_g are wrappers for member functions in Player
    g_signal_connect (appsrc, "need-data", G_CALLBACK (need_data_g),this);
    g_signal_connect (appsrc, "enough-data", G_CALLBACK (enough_data_g),this);
    
    const double frequency = 440.0;
    add_instrument(new SinGenerator(max_volume/5,2*M_PI*frequency/sample_rate));
    add_instrument(new SinGenerator(max_volume/5,2*M_PI*(5.0/4)*frequency/sample_rate));
    add_instrument(new SinGenerator(max_volume/5,2*M_PI*(3.0/2)*frequency/sample_rate));
    add_instrument(new SinGenerator(max_volume/5,2*M_PI*(15.0/8)*frequency/sample_rate));
    add_instrument(new SinGenerator(max_volume/5,2*M_PI*2*frequency/sample_rate));
}

void Player::add_instrument(Instrument * instrument)
{
    add_instrument(spInstrument(instrument));
}

void Player::add_instrument(spInstrument instrument)
{
    instruments.push_back(instrument);
}

void Player::play()
{
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);
}

void Player::quit()
{
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_main_loop_quit(loop);
}

gboolean Player::push_data()
{
    packet_t data(packet_size);
    for(itInstruments it = instruments.begin();
            it != instruments.end();
            ++it)
    {
        data+=*(*it)->get_samples(packet_size);
    }

    GstBuffer * buffer = gst_buffer_new_allocate (NULL, buffer_length, NULL);
    gst_buffer_fill(buffer,0,&data[0], buffer_length);

    offset+=buffer_length/word_size;

    if(offset > signal_length)
    {
        quit();
        return false;
    }

    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    if (ret != GST_FLOW_OK)
    {
        g_main_loop_quit (loop);
        return false;
    }

    return true;
}

void Player::need_data()
{
    if(sourceid==0)
    {
        sourceid=g_idle_add((GSourceFunc) push_data_g, this);
    } 
}

void Player::enough_data()
{
    if(sourceid!=0)
    {
        g_source_remove(sourceid);
        sourceid=0;
    }
}

Player::~Player()
{
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);
}

gboolean Player::push_data_g(gpointer instance)
{
    Player * this_ = static_cast<Player *>(instance);
    return this_->push_data();
}

void Player::need_data_g(GstElement* element, guint length, gpointer instance)
{
    Player * this_ = static_cast<Player *>(instance);
    this_->need_data();
}

void Player::enough_data_g(GstElement* src, gpointer instance)
{
    Player * this_ = static_cast<Player *>(instance);
    this_->enough_data();
}
