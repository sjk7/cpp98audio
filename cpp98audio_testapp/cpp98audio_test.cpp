#include <iostream>
#include "../include/cpp_98_audio_envelope.hpp"
using namespace std;

int main() {
    const int nch = 2;
    const int samplerate = 44100;
    const int secs = 30;

    assert(!my::float_equal(0.1f, 0));
    const size_t sz = samplerate * secs * nch;
    size_t actual_sz = 0;

    short* const shortbuf = my::cpp98::audio::make_buffer(
        short(0), actual_sz, 30);
    assert(actual_sz == sz);

    float* const floatbuf = my::cpp98::audio::make_buffer(
        float(0), actual_sz, 30);

    typedef my::cpp98::audio::envelope::history_t hist_t;
    typedef hist_t::const_iterator hist_it;

    my::cpp98::audio::envelope env(44100, 2, 20.0, 500.0);
    hist_t& history = env.history();
    history.resize(sz);
    env.envelope_shorts(shortbuf, shortbuf + sz);
    float env_value = env();
    assert(my::float_equal(env_value, 0));
    env.envelope_shorts(shortbuf, shortbuf + sz);

    const hist_t& hist = env.history_const();
    hist_it hit
        = std::max_element(hist.begin(), hist.end());
    assert(
        my::float_equal(*hit, 0) && (hit == hist.begin()));

    my::cpp98::audio::envelope::floats_to_shorts(
        hist.begin(), hist.end(), (short*)shortbuf,
        (short*)shortbuf + sz, nch);

    hit = std::max_element(hist.begin(), hist.end());
    assert(my::float_equal(*hit, 0) && hit == hist.begin());

    my::cpp98::audio::test::reverse_vector();
    my::cpp98::audio::test::reverse_array(
        shortbuf, shortbuf + 5);

    my::cpp98::audio::test::check_envelope_timing(
        10.0f, 100.0f);

    delete[] shortbuf;
    delete[] floatbuf;

    return 0;
}
