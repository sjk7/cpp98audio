#include <iostream>
#include "../include/cpp_98_audio_envelope.hpp"
using namespace std;

void check_release_accuracy(
    short* const samples, const short* const samples_end) {
    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 10.0f, 100.0f);

    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 1.0f, 100.0f);

    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 10.0f, 1000.0f);

    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 10.0f, 500.0f);

    // ->
    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 50.0f, 2000.0f);

    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 0.1f, 1000.0f);

    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 50.0f, 10000.0f);

    my::cpp98::audio::test::check_envelope_release(
        samples, samples_end, 50.0f, 50.0f);
}
void check_attack_accuracy(
    short* const samples, const short* const samples_end) {
    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 1.0f, 100.0f);

    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 100.0f, 1000.0f);

    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 1000.0f, 10000.0f);
    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 2000.0f, 50.0f);

    // ->
    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 4000.0f, 100.0f);

    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 6000.0f, 1000.0f);

    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 8000.0f, 10000.0f);

    my::cpp98::audio::test::check_envelope_attack(
        samples, samples_end, 1.0f, 500.0f);
}
int main() {

    const int nch = 2;
    const int samplerate = 44100;
    const int secs = 30;

    assert(!my::float_equal(0.1f, 0.0f));
    const size_t sz = samplerate * secs * nch;
    size_t actual_sz = 0;

    short* const shortbuf = my::cpp98::audio::make_buffer(
        short(0), actual_sz, 30);
    assert(actual_sz == sz);

    float* const floatbuf = my::cpp98::audio::make_buffer(
        float(0), actual_sz, 30);

    short* shortbuf_raw = const_cast<short*>(shortbuf);
    short* shortbuf_raw_end
        = const_cast<short*>(shortbuf + sz);

    std::fill(shortbuf_raw, shortbuf_raw_end, 0);

    typedef my::cpp98::audio::envelope::history_t hist_t;
    typedef hist_t::const_iterator hist_it;

    my::cpp98::audio::envelope env(44100, 2, 20.0, 500.0);
    hist_t& history = env.history();
    history.resize(sz);
    env.envelope_shorts(shortbuf, shortbuf + sz);
    float env_value = env();
    assert(my::float_equal(env_value, 0.0f));
    env.envelope_shorts(shortbuf, shortbuf + sz);

    const hist_t& hist = env.history_const();
    hist_it hit
        = std::max_element(hist.begin(), hist.end());
    assert(my::float_equal(*hit, 0.0f)
        && (hit == hist.begin()));

    my::cpp98::audio::floats_to_shorts(hist.begin(),
        hist.end(), shortbuf_raw, shortbuf_raw_end, nch);

    hit = std::max_element(hist.begin(), hist.end());
    assert(
        my::float_equal(*hit, 0.0f) && hit == hist.begin());

    my::cpp98::audio::test::reverse_vector();
    my::cpp98::audio::test::reverse_array(
        const_cast<short* const>(shortbuf),
        const_cast<short* const>(shortbuf + 5));

    check_release_accuracy(
        const_cast<short* const>(shortbuf),
        shortbuf + actual_sz);
    check_attack_accuracy(
        const_cast<short* const>(shortbuf),
        shortbuf + actual_sz);

    delete[] shortbuf;
    delete[] floatbuf;

    return 0;
}
