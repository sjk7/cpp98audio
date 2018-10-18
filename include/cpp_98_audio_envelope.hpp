#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <limits>

namespace my {
template <typename T>
inline bool float_equal(T f, T t,
    T tolerance = std::numeric_limits<float>::epsilon()) {

    float diff = f - (float)t;
    diff = fabsf(diff);
    if (diff <= tolerance) {
        return true;
    }
    return false;
}
namespace cpp98 {
    namespace audio {
#ifndef TAU
#define TAU 0.632f
#endif
#ifndef TAU_DECAY
#define TAU_DECAY 0.368f
#endif

        template <typename T> static inline T max_value() {
            return std::numeric_limits<T>::max();
        }

        template <typename T> static inline T min_value() {
            return std::numeric_limits<T>::min();
        }

        template <typename T> static inline T tiny_value() {
            return std::numeric_limits<T>::epsilon();
        }

        template <typename T>
        inline static void reverse_samples(T begin, T end) {
            std::reverse(begin, end);
            return;
        }

        inline static short clip_short(float val) {

            if ((val) > tiny_value<float>()) {
                if (val > 32767.0f) val = 32767.0f;
            } else {
                if (val < -32768.0f) val = -32768.0f;
            }

            return (short)val;
        }

        template <typename I>
        static inline void floats_to_shorts(const I begin,
            const I end, short* const pdest,
            short* const pdest_end, const int nch) {

            const float mult = 32767.0f;
            int nsamps_src = (end - begin);
            int nsamps_dst = (pdest_end - pdest);
            assert(nsamps_src == nsamps_dst);
            if (nsamps_dst != nsamps_src) return;

            short* dptr = pdest;
            I sptr = begin;
            I s_end = end;
            while (sptr < s_end) {
                int ch = 0;
                while (ch < nch) {
                    float fval = (float)*sptr;
                    ++sptr;
                    ++ch;
                    fval *= mult;
                    *dptr = clip_short(fval);
                    ++dptr;
                };
            }
        }

        template <typename T>
        inline T* make_buffer(const T fill_with,
            size_t& actual_sz, const int nsecs,
            const int samplerate = 44100, int nch = 2) {

            actual_sz = size_t(samplerate * nch * nsecs);
            T* retbuf = new T[actual_sz];
            std::fill(
                retbuf, retbuf + actual_sz, fill_with);
            T* test = ((retbuf + actual_sz) - 1);
            assert(*test == fill_with);
            return retbuf;
        }

        template <typename T>
        inline void make_buffer(std::vector<T>& v,
            const T fill_with, size_t& actual_sz,
            const int nsecs, const int samplerate = 44100,
            const int nch = 2) {

            actual_sz = size_t(samplerate * nch * nsecs);
            v.resize(actual_sz);
            if (fill_with != 0) {
                std::fill(v.begin(), v.end(), fill_with);
            }
            return;
        }

        class envelope {
            public:
            typedef std::vector<float> history_t;
            typedef std::vector<float> floatvec_t;
            const history_t& history_const() const {
                return m_history;
            }
            inline history_t& history() {
                return m_history;
            }

            inline float update(const float value) {
                /*/ reference:
                http://www.musicdsp.org/showArchiveComment.php?ArchiveID=97
                * // Slightly faster version of the envelope
                follower using one multiply form.

                // attTime and relTime is in seconds
                float ga = exp(-1.0f/(sampleRate*attTime));
                float gr = exp(-1.0f/(sampleRate*relTime));

                float envOut = 0.0f;

                for( ... )
                {
                    // get your data into 'input'
                    envIn = fabs(input);

                    if( envOut < envIn ){
                    envOut = envIn + ga * (envOut - envIn);
                    }
                    else{
                    envOut = envIn + gr* (envOut -envIn);
                    }

                    // envOut now contains the envelope
                };
                /*/
                float env_in = fabsf(value);
                if (m_env < env_in) {
                    m_env
                        = env_in + m_ga * (m_env - env_in);
                } else {
                    m_env
                        = env_in + m_gr * (m_env - env_in);
                }
                return m_env;
            }

            private:
            float m_samplerate;
            int m_nch;

            float m_env, m_attms, m_relms, m_ga, m_gr;
            history_t m_history;
            floatvec_t m_conversion_buffer;

            inline float attack_coef(float att_ms) {
                assert(m_nch);
                att_ms *= m_nch;
                // float ga =
                // exp(-1.0f/(sampleRate*attTime));
                const double att_secs
                    = (double)att_ms / 1000.0;
                const double ga = exp(-1.0
                    / ((double)m_samplerate * att_secs));
                return (float)ga;
            }
            inline float release_coef(float rel_ms) {
                assert(m_nch);
                rel_ms *= m_nch;
                const double rel_secs
                    = (double)rel_ms / 1000.0;

                const double gr = exp(-1.0
                    / ((double)m_samplerate * rel_secs));
                return (float)gr;
            }

            public:
            inline float operator()() { return m_env; }
            inline float attack_ms() const {
                return m_attms;
            }
            inline float release_ms() const {
                return m_relms;
            }
            inline void set_attack_ms(float millisecs) {
                m_ga = attack_coef(millisecs);
            }
            inline void set_release_ms(float millisecs) {
                m_gr = release_coef(millisecs);
            }
            inline int channels() const { return m_nch; }
            inline int samplerate() const {
                return (int)m_samplerate;
            }
            inline float sampleratef() const {
                return m_samplerate;
            }
            // You'll need this very rarely. For hard
            // gating, tests and friends.
            inline void set_envelope_to(const float f) {
                m_env = f;
            }

            envelope(int samplerate, int nch,
                float attms = 10.0f, float relms = 100.0f)
                : m_samplerate((float)samplerate)
                , m_nch(nch)
                , m_env(0)
                , m_attms(attms)
                , m_relms(relms)
                , m_ga(attack_coef(m_attms))
                , m_gr(release_coef(m_relms)) {
                (void)m_nch;
            }

            static inline void shorts_to_floats(
                const short* begin, const short* end,
                const int nch, history_t* pvhist = 0) {

                const float div = 32768.0f;
                int nsamps = end - begin;
                float* vptr = 0;
                if (pvhist) {

                    pvhist->resize(size_t(nsamps));
                    vptr = &pvhist->operator[](0);
                }

                const short* sptr = begin;
                int ctr = 0;

                while (sptr < end) {

                    int ch = 0;
                    while (ch < nch) {
                        float val = (float)*sptr;
                        ++sptr;
                        val /= div;
                        if (vptr) {
                            *vptr = val;
                            ++vptr;
                        }
                        if (ctr >= 22050) {
                            volatile int x = 0;
                            ++x;
                        }
                        ++ctr;
                        ++ch;
                    };
                };

                if (pvhist) {
                    float* vbeg = &pvhist->operator[](0);
                    float* pvhist_end
                        = vbeg + pvhist->size();
                    int hist_remain = pvhist_end - vptr;
                    assert(hist_remain == 0);
                    // fprintf(stdout, "%d\n", hist_remain);
                }
            }

            float sample_pos_in_secs(int sample_position) {
                sample_position /= m_nch;
                return (float)sample_position
                    / sampleratef();
            }
            float sample_pos_in_msecs(int sample_position) {
                float s
                    = sample_pos_in_secs(sample_position);
                return (s * 1000.0f);
            }
            typedef floatvec_t::const_iterator cit_t;

            inline cit_t envelope_floats(
                const float* const sentinel_attack,
                const float* const sentinel_release) {

                cit_t bit = m_conversion_buffer.begin();
                cit_t eit = m_conversion_buffer.end();
                cit_t it;
                int ctr = 0;
                bool done = false;

                for (it = bit; it < eit;) {
                    int ch = 0;
                    while (ch++ < m_nch) {
                        float val = *it;
                        ++it;
                        float e = this->update(val);

                        if (sentinel_attack) {
                            if (e >= *sentinel_attack) {
                                done = true;
                            }
                        }
                        if (sentinel_release) {

                            if (e <= *sentinel_release) {
                                done = true;
                            }
                        }

                        ++ctr;
                    };
                    if (done) {
                        return it;
                    }
                };
                return eit;
            }

            const short* envelope_shorts(const short* begin,
                const short* end,
                const float* const sentinel_attack = NULL,
                const float* const sentinel_release
                = NULL) {

                assert(m_nch > 0 && m_nch <= 2);
                assert(m_samplerate > 0
                    && m_samplerate < 192000);

                int nsamples = end - begin;
                (void)nsamples;
                shorts_to_floats(begin, end, m_nch,
                    &m_conversion_buffer);

                cit_t fiter = envelope_floats(
                    sentinel_attack, sentinel_release);

                ptrdiff_t nsamps_from_end
                    = m_conversion_buffer.end() - fiter;
                const short* retval = end - nsamps_from_end;
                return retval;
            }
        };

        namespace test {

            namespace detail {
                float measure_attack_time(float attack_ms,
                    float release_ms, short* const samps,
                    const short* const samps_end) {

                    short* rwsamps_end
                        = const_cast<short*>(samps_end);
                    std::fill(samps, rwsamps_end, 32767);
                    size_t sz = (size_t)(samps_end - samps);
                    const short* pbuffer = samps;

                    envelope env(44100, 2, attack_ms);

                    const float stop_when = TAU;
                    env.set_attack_ms(attack_ms);
                    env.set_release_ms(release_ms);
                    const short* pwhen
                        = env.envelope_shorts(pbuffer,
                            pbuffer + sz, &stop_when);

                    ptrdiff_t nsamps = pwhen - pbuffer;

                    float ms_actual
                        = (float)env.sample_pos_in_msecs(
                            nsamps);
                    const float ms_should_be
                        = env.attack_ms();
                    float time_diff
                        = (fabsf(ms_actual - ms_should_be));
                    float ten_percent
                        = 0.10f * ms_should_be;
                    assert(time_diff <= ten_percent);
                    return ms_actual;
                }

                float measure_release_time(float attack_ms,
                    float release_ms, short* const samps,
                    const short* const samps_end) {

                    size_t sz = (size_t)(samps_end - samps);
                    const short* const pbuffer = samps;

                    const short* const pbuffer_end
                        = pbuffer + sz;

                    envelope env(
                        44100, 2, attack_ms, release_ms);

                    env.set_attack_ms(attack_ms);
                    env.set_release_ms(release_ms);

                    int silence_start_pos = 0;

                    {
                        std::fill(samps,
                            const_cast<short* const>(
                                samps_end),
                            32767);
                        const int remove_audio_when
                            = 1; // after 1 sec, start
                                 // measuring release
                        const int nsamps = (env.samplerate()
                            * env.channels());
                        silence_start_pos
                            = nsamps * remove_audio_when;

                        const int shorts_remain
                            = pbuffer_end
                            - (pbuffer + nsamps);
                        const size_t bytes_remain
                            = (size_t)shorts_remain
                            * sizeof(short);

                        const short* const audio_begin
                            = (pbuffer + silence_start_pos);
                        memset((void*)audio_begin, 0,
                            bytes_remain);
                        const short* const my_end = pbuffer
                            + (silence_start_pos
                                  + shorts_remain);
                        assert(pbuffer_end == my_end);

                        env.envelope_shorts(pbuffer,
                            pbuffer + silence_start_pos,
                            NULL, NULL);
                    }

                    const float stop_when = TAU_DECAY;
                    assert(
                        my::float_equal(env(), 1.0f, 0.2f));

                    const short* const the_end
                        = pbuffer_end;

                    const short* pwhen
                        = env.envelope_shorts(pbuffer,
                            the_end, NULL, &stop_when);

                    assert(pwhen != the_end);
                    assert((pbuffer_end - pwhen)
                            % env.channels()
                        == 0);

                    assert(pwhen != pbuffer_end);
                    ptrdiff_t nsamps = pwhen - pbuffer;

                    float ms_actual
                        = (float)env.sample_pos_in_msecs(
                            nsamps);

                    ms_actual
                        -= 1000; // we only started looking
                                 // 'after' 1000ms
                    const float ms_should_be
                        = env.release_ms();
                    float time_diff
                        = (fabsf(ms_actual - ms_should_be));

                    float ten_percent = 0.1f * ms_should_be;
                    assert(time_diff <= ten_percent);
                    return ms_actual;
                }
            } // namespace detail
            inline float check_envelope_attack(
                short* const samps,
                const short* const samps_end,
                float attack_ms = 10.0f,
                float release_ms = 100.0f) {

                const size_t sz
                    = (size_t)(samps_end - samps);
                memset((void*)samps, 0, sz * sizeof(short));
                float actual_attack_time
                    = detail::measure_attack_time(attack_ms,
                        release_ms, samps, samps_end);

                return actual_attack_time;
            }

            inline float check_envelope_release(
                short* const samps,
                const short* const samps_end,
                float attack_ms = 10.0f,
                float release_ms = 100.0f) {

                const size_t sz
                    = (size_t)(samps_end - samps);
                memset((void*)samps, 0, sz * sizeof(short));
                float actual_release_time
                    = detail::measure_release_time(
                        attack_ms, release_ms, samps,
                        samps_end);

                return actual_release_time;
            }

            void reverse_vector() {
                std::vector<short> v;
                v.push_back(1);
                v.push_back(2);
                v.push_back(3);
                v.push_back(4);

                my::cpp98::audio::reverse_samples(
                    v.begin(), v.end());
                assert(v[0] == 4);
            }

            void reverse_array(
                short* const b, short* const e) {
                short* ps = b;
                short* ps_end = e;
                short int i = 0;

                while (ps < ps_end) {
                    *ps++ = i++;
                }
                ps = b;
                i = 0;
                while (ps < ps_end) {
                    assert(*ps++ == i++);
                }

                my::cpp98::audio::reverse_samples(b, e);
                ps = b;
                i = 4;
                while (ps < ps_end) {
                    int x = *ps++;
                    assert(x == i);
                    i--;
                }
            }

        } // namespace test

    } // namespace audio

} // namespace cpp98
} // namespace my
