#pragma once

#include <cmath>
#include <vector>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <cstdio>

namespace my {
namespace cpp98 {
    namespace audio {

        template <typename T>
        inline T* make_buffer(const T fill_with, const int nsecs,
            const int samplerate = 44100, int nch = 2) {
            const size_t sz = samplerate * nch * nsecs;
            T* retbuf = new T[sz];
            std::fill(retbuf, retbuf + sz, fill_with);
            return retbuf;
        }

        template <typename T>
        inline void make_buffer(std::vector<T>& v, const T fill_with, const int nsecs,
            const int samplerate = 44100, const int nch = 2) {
            const size_t sz = samplerate * nch * nsecs;
            v.resize(sz);
            if (fill_with != 0) {
                std::fill(v.begin(), v.end(), fill_with);
            }
            return;
        }

#define TINY_FLOAT 0.00001
        inline static short clip_short(float val) {

            if ((val) > TINY_FLOAT) {
                if (val > 32767.0f) val = 32767.0f;
            } else {
                if (val < -32768.0f) val = -32768.0f;
            }

            return val;
        }

        /*/ reference: http://www.musicdsp.org/showArchiveComment.php?ArchiveID=97
         * // Slightly faster version of the envelope follower using one multiply form.

            // attTime and relTime is in seconds

            float ga = exp(-1.0f/(sampleRate*attTime));
            float gr = exp(-1.0f/(sampleRate*relTime));

            float envOut = 0.0f;

            for( ... )
            {
                // get your data into 'input'
                envIn = fabs(input);

                if( envOut < envIn )
                    envOut = envIn + ga * (envOut - envIn);
                else
                    envOut = envIn + gr * (envOut - envIn);

                // envOut now contains the envelope
             }
    /*/

        class envelope {
            public:
            typedef std::vector<float> history_t;
            const history_t& history_const() const { return m_history; }
            history_t& history() { return m_history; }

            private:
            float m_samplerate;
            int m_nch;
            float m_env, m_attms, m_relms, m_ga, m_gr;
            history_t m_history;

            float attack_coef(const float att_ms) {
                const float att_secs = att_ms / 1000.0f;
                const float ga = exp(-1.0f / (m_samplerate * att_secs));
                return ga;
            }
            float release_coef(const float rel_ms) {

                const float rel_secs = rel_ms / 1000.0f;
                const float gr = exp(-1.0f / (m_samplerate * rel_secs));
                return gr;
            }

            public:
            float operator()() { return m_env; }
            float attack() const { return m_attms; }
            float release() const { return m_relms; }

            envelope(int samplerate, int nch, float attms = 10.0f, float relms = 100.0f)
                : m_samplerate((float)samplerate)
                , m_nch(nch)
                , m_env(0)
                , m_attms(attms)
                , m_relms(relms)
                , m_ga(attack_coef(m_attms))
                , m_gr(release_coef(m_relms)) {}

            static inline void shorts_to_floats(const short* begin, const short* end,
                const int nch, history_t* pvhist = 0) {

                const float div = 32768.0f;
                int nsamps = end - begin;
                float* vptr = 0;
                if (pvhist) {
                    pvhist->resize(nsamps);
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
                    float* pvhist_end = vbeg + pvhist->size();
                    int hist_remain = pvhist_end - vptr;
                    fprintf(stdout, "%d\n", hist_remain);
                }
            }

            template <typename I>
            static inline void floats_to_shorts(const I begin, const I end, short* pdest,
                short* pdest_end, const int nch) {

                const float mult = 32767.0f;
                int nsamps_src = (end - begin);
                int nsamps_dst = (pdest_end - pdest);
                assert(nsamps_src == nsamps_dst);
                if (nsamps_dst != nsamps_src) return;

                /*/
                    const int* ptr;
                        You can modify ptr itself but the object
                        pointed to by ptr shall not be modified.
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
            void envelope_shorts(const short* begin, const short* end, const int nch,
                history_t* phistory = 0) {
                shorts_to_floats(begin, end, nch, phistory);
            }
        };

    } // namespace audio
} // namespace cpp98
} // namespace my
