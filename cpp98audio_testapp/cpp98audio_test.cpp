#include <iostream>
#include "../include/cpp_98_audio_envelope.hpp"
using namespace std;

int main() {
    const int nch = 2;
    const int samplerate = 44100;
    const int secs = 30;

    const size_t sz = samplerate * secs * nch;
    const short* const shortbuf = my::cpp98::audio::make_buffer(short(0), 30);
    const float* const floatbuf = my::cpp98::audio::make_buffer(float(0), 30);

    typedef my::cpp98::audio::envelope::history_t hist_t;
    typedef hist_t::const_iterator hist_it;

    my::cpp98::audio::envelope env(44100, 2, 20.0, 500.0);
    hist_t& history = env.history();
    env.envelope_shorts(shortbuf, shortbuf + sz, nch);
    float env_value = env();
    assert(env_value == 0);
    env.envelope_shorts(shortbuf, shortbuf + sz, nch, &history);

    const hist_t& hist = env.history_const();
    hist_it hit = std::max_element(hist.begin(), hist.end());
    assert(*hit == 0 && hit == hist.begin());

    my::cpp98::audio::envelope::floats_to_shorts(
        hist.begin(), hist.end(), (short*)shortbuf, (short*)shortbuf + sz, nch);

    hit = std::max_element(hist.begin(), hist.end());
    assert(*hit == 0 && hit == hist.begin());
    delete[] shortbuf;
    delete[] floatbuf;

    return 0;
}
