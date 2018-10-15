#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace my {
template <typename T>
inline bool float_equal(float f, T t) {
    float diff = f - (float)t;
    static float eps
        = std::numeric_limits<float>::epsilon();
    if (fabsf(diff) >= eps) {
        return false;
    }
    return true;
}
namespace cpp98 {
    namespace audio {

        template <typename T>
        inline T* make_buffer(const T fill_with,
            size_t& actual_sz, const int nsecs,
            const int samplerate = 44100, int nch = 2) {

            actual_sz = size_t(samplerate * nch * nsecs);
            T* retbuf = new T[actual_sz];
            std::fill(
                retbuf, retbuf + actual_sz, fill_with);
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

        template <typename T> static inline T max_value() {
            return std::numeric_limits<T>::max();
        }

        template <typename T> static inline T min_value() {
            return std::numeric_limits<T>::min();
        }

        template <typename T> static inline T tiny_value() {
            return std::numeric_limits<T>::epsilon();
        }

        inline static short clip_short(float val) {

            if ((val) > tiny_value<float>()) {
                if (val > 32767.0f) val = 32767.0f;
            } else {
                if (val < -32768.0f) val = -32768.0f;
            }

            return (short)val;
        }

        template <typename T>
        inline static void reverse_samples(T begin, T end) {
            std::reverse(begin, end);
            return;
        }

        class envelope {
            public:
            typedef std::vector<float> history_t;
            typedef std::vector<float> floatvec_t;
            const history_t& history_const() const {
                return m_history;
            }
            history_t& history() { return m_history; }

            inline float update(const float value) {
                /*/ reference:
                http://www.musicdsp.org/showArchiveComment.php?ArchiveID=97
                * // Slightly faster version of the envelope
                follower using one multiply form.

                        // attTime and relTime is in seconds

                        float ga =
                exp(-1.0f/(sampleRate*attTime)); float gr =
                exp(-1.0f/(sampleRate*relTime));

                float envOut = 0.0f;

                for( ... )
                {
                        // get your data into 'input'
                        envIn = fabs(input);

                        if( envOut < envIn )
                                        envOut = envIn + ga
                * (envOut - envIn); else envOut = envIn + gr
                * (envOut - envIn);

                                // envOut now contains the
                envelope
                }
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

            float attack_coef(const float att_ms) {
                const double att_secs
                    = (double)att_ms / 1000.0;
                const double ga = exp(-1.0
                    / ((double)m_samplerate * att_secs));
                return (float)ga;
            }
            float release_coef(const float rel_ms) {

                const double rel_secs
                    = (double)rel_ms / 1000.0;
                const double gr = exp(-1.0
                    / ((double)m_samplerate * rel_secs));
                return (float)gr;
            }

            public:
            float operator()() { return m_env; }
            float attack() const { return m_attms; }
            float release() const { return m_relms; }
            void set_attack(float millisecs) {
                m_ga = attack_coef(millisecs);
            }
            void set_release(float millisecs) {
                m_gr = release_coef(millisecs);
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
                        ctr++;
                        if (ctr >= nsamps) {
                            puts("hello\n");
                        }
                        ++ch;
                    };
                };

                if (pvhist) {
                    float* vbeg = &pvhist->operator[](0);
                    float* pvhist_end
                        = vbeg + pvhist->size();
                    int hist_remain = pvhist_end - vptr;
                    fprintf(stdout, "%d\n", hist_remain);
                }
            }

            template <typename I>
            static inline void floats_to_shorts(
                const I begin, const I end, short* pdest,
                short* pdest_end, const int nch) {

                const float mult = 32767.0f;
                int nsamps_src = (end - begin);
                int nsamps_dst = (pdest_end - pdest);
                assert(nsamps_src == nsamps_dst);
                if (nsamps_dst != nsamps_src) return;

                /*/
                const int* ptr;
                You can modify ptr itself but the object
                        pointed to by ptr shall not be
                modified.
                        /*/

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
            const short* envelope_shorts(const short* begin,
                const short* end, float* sentinel = 0) {

                assert(m_nch > 0 && m_nch <= 2);
                assert(m_samplerate > 0
                    && m_samplerate < 192000);

                shorts_to_floats(begin, end, m_nch,
                    &m_conversion_buffer);

                typedef floatvec_t::const_iterator cit_t;
                cit_t b = m_conversion_buffer.cbegin();
                cit_t e = m_conversion_buffer.cend();
                cit_t it;
                bool done = false;

                for (it = b; it < e; ++it) {
                    int ch = 0;
                    while (ch++ < m_nch) {
                        float val = *it;
                        float e = this->update(val);
                        if (sentinel) {
                            if (e >= *sentinel) {
                                done = true;
                            }
                        }
                        ++it;
                    };
                    if (done) {
                        std::ptrdiff_t diff = it - b;
                        return (const short*)begin + diff;
                    }
                };
                return end;
            }
        };

        namespace test {

            namespace detail {
                float measure_attack_time(
                    float attack_ms, float release_ms) {
                    size_t sz = 0;
                    short* pbuffer
                        = my::cpp98::audio::make_buffer<
                            short>(32767, sz, 30);
                    envelope env(44100, 2, attack_ms);

                    float stop_when = 0.707f;
                    env.set_attack(attack_ms);
                    env.set_release(release_ms);
                    env.envelope_shorts(
                        pbuffer, pbuffer + sz, &stop_when);

                    return 0.0f;
                }
            } // namespace detail
            float check_envelope_timing(
                float attack_ms = 10.0f,
                float release_ms = 100.0f) {
                float actual_attack_time
                    = detail::measure_attack_time(
                        attack_ms, release_ms);

                return actual_attack_time;
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

            void reverse_array(short* b, short* e) {
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
