#pragma once

/*
* original code .......
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#include <ct_type.h>

namespace _ns_tools
{
	namespace _algorithm
	{
		class ct_sha1
		{
        private:
            typedef union {
                unsigned char c[64];
                unsigned long l[16];
            } _type_CHAR64LONG16;

		public:
			ct_sha1() :
                m_v_buffer(64,0),
                m_v_ul_state(5,0),
                m_v_ul_count(2,0)
			{
				/* SHA1 initialization constants */
				m_v_ul_state[0] = 0x67452301;
				m_v_ul_state[1] = 0xEFCDAB89;
				m_v_ul_state[2] = 0x98BADCFE;
				m_v_ul_state[3] = 0x10325476;
				m_v_ul_state[4] = 0xC3D2E1F0;
				m_v_ul_count[0] = m_v_ul_count[1] = 0;
			}

            /* Run your data through this. */
            template <size_t _N>
            void update(const std::array<unsigned char, _N>& ar_data)
            {
                update(&ar_data[0], ar_data.size());
            }
            void update(const _ns_tools::type_v_buffer& v_data)
            {
                if (v_data.empty()) {
                    update(NULL, 0);
                }
                else {
                    update(&v_data[0], v_data.size());
                }
            }
            void update(const unsigned char* data, unsigned long len)
            {
                unsigned long i;
                unsigned long j;

                j = m_v_ul_count[0];
                if ((m_v_ul_count[0] += len << 3) < j)
                    m_v_ul_count[1]++;
                m_v_ul_count[1] += (len >> 29);
                j = (j >> 3) & 63;
                if ((j + len) > 63) {
                    memcpy(&m_v_buffer[j], data, (i = 64 - j));
                    _sha1_transform(&m_v_ul_state[0], &m_v_buffer[0]);
                    for (; i + 63 < len; i += 64) {
                        _sha1_transform(&m_v_ul_state[0], &data[i]);
                    }
                    j = 0;
                }
                else i = 0;
                memcpy(&m_v_buffer[j], &data[i], len - i);
            }


            /* Add padding and return the message digest. */
            void final(_ns_tools::type_v_buffer& v_digest)
            {
                v_digest.assign(20, 0);
                final(&v_digest[0]);
            }
            void final(_ns_tools::type_arrary_digest& ar_digest)
            {
                ar_digest.fill(0);
                final(&ar_digest[0]);
            }
            void final(unsigned char digest[20])
            {
                unsigned i;
                unsigned char finalcount[8];
                unsigned char c;

                for (i = 0; i < 8; i++) {
                    finalcount[i] = (unsigned char)((m_v_ul_count[(i >= 4 ? 0 : 1)]
                        >> ((3 - (i & 3)) * 8)) & 255);  /* Endian independent */
                }

                c = 0200;
                update(&c, 1);
                while ((m_v_ul_count[0] & 504) != 448) {
                    c = 0000;
                    update(&c, 1);
                }
                update(finalcount, 8);  /* Should cause a SHA1Transform() */
                for (i = 0; i < 20; i++) {
                    digest[i] = (unsigned char)
                        ((m_v_ul_state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
                }
                /* Wipe variables */
                _clear();
                memset(&finalcount, 0, sizeof(finalcount));
            }

        private:
            void _clear()
            {
                m_v_buffer.assign(m_v_buffer.size(), 0);
                m_v_ul_state.assign(m_v_ul_state.size(), 0);
                m_v_ul_count.assign(m_v_ul_count.size(), 0);
            }

            unsigned long _rol(unsigned long value, unsigned long bits)
            {
                return (((value) << (bits)) | ((value) >> (32 - (bits))));
            }
            unsigned long _blk0(ct_sha1::_type_CHAR64LONG16* block, unsigned long i)
            {
                return block->l[i] = (_rol(block->l[i], 24) & 0xFF00FF00) | (_rol(block->l[i], 8) & 0x00FF00FF);
            }
            unsigned long _blk(ct_sha1::_type_CHAR64LONG16* block, unsigned long i)
            {
                return block->l[i & 15] = _rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1);
            }

            void _R0(ct_sha1::_type_CHAR64LONG16* block,unsigned long &v, unsigned long& w, unsigned long& x, unsigned long& y, unsigned long& z, unsigned long i)
            {
                z += ((w & (x ^ y)) ^ y) + _blk0(block,i) + 0x5A827999 + _rol(v, 5); w = _rol(w, 30);
            }
            void _R1(ct_sha1::_type_CHAR64LONG16* block, unsigned long& v, unsigned long& w, unsigned long& x, unsigned long& y, unsigned long& z, unsigned long i)
            {
                z += ((w & (x ^ y)) ^ y) + _blk(block,i) + 0x5A827999 + _rol(v, 5); w = _rol(w, 30);
            }
            void _R2(ct_sha1::_type_CHAR64LONG16* block, unsigned long& v, unsigned long& w, unsigned long& x, unsigned long& y, unsigned long& z, unsigned long i)
            {
                z += (w ^ x ^ y) + _blk(block,i) + 0x6ED9EBA1 + _rol(v, 5); w = _rol(w, 30);
            }
            void _R3(ct_sha1::_type_CHAR64LONG16* block, unsigned long& v, unsigned long& w, unsigned long& x, unsigned long& y, unsigned long& z, unsigned long i)
            {
                z += (((w | x) & y) | (w & x)) + _blk(block,i) + 0x8F1BBCDC + _rol(v, 5); w = _rol(w, 30);
            }
            void _R4(ct_sha1::_type_CHAR64LONG16* block, unsigned long& v, unsigned long& w, unsigned long& x, unsigned long& y, unsigned long& z, unsigned long i)
            {
                z += (w ^ x ^ y) + _blk(block,i) + 0xCA62C1D6 + _rol(v, 5); w = _rol(w, 30);
            }

            /* Hash a single 512-bit block. This is the core of the algorithm. */
            void _sha1_transform(unsigned long state[5], const unsigned char buffer[64])
            {

                unsigned long a, b, c, d, e;

                ct_sha1::_type_CHAR64LONG16 block[1];  /* use array to appear as a pointer */
                memcpy(block, buffer, 64);
                /* Copy context->state[] to working vars */
                a = state[0];
                b = state[1];
                c = state[2];
                d = state[3];
                e = state[4];

                /* 4 rounds of 20 operations each. Loop unrolled. */
                _R0(block,a, b, c, d, e, 0);  _R0(block,e, a, b, c, d, 1);   _R0(block,d, e, a, b, c, 2);   _R0(block,c, d, e, a, b, 3);
                _R0(block,b, c, d, e, a, 4);  _R0(block,a, b, c, d, e, 5);   _R0(block,e, a, b, c, d, 6);   _R0(block,d, e, a, b, c, 7);
                _R0(block,c, d, e, a, b, 8);  _R0(block,b, c, d, e, a, 9);   _R0(block,a, b, c, d, e, 10);  _R0(block,e, a, b, c, d, 11);
                _R0(block,d, e, a, b, c, 12); _R0(block,c, d, e, a, b, 13);  _R0(block,b, c, d, e, a, 14);  _R0(block,a, b, c, d, e, 15);
                _R1(block,e, a, b, c, d, 16); _R1(block,d, e, a, b, c, 17);  _R1(block,c, d, e, a, b, 18);  _R1(block,b, c, d, e, a, 19);
                _R2(block,a, b, c, d, e, 20); _R2(block,e, a, b, c, d, 21);  _R2(block,d, e, a, b, c, 22);  _R2(block,c, d, e, a, b, 23);
                _R2(block,b, c, d, e, a, 24); _R2(block,a, b, c, d, e, 25);  _R2(block,e, a, b, c, d, 26);  _R2(block,d, e, a, b, c, 27);
                _R2(block,c, d, e, a, b, 28); _R2(block,b, c, d, e, a, 29);  _R2(block,a, b, c, d, e, 30);  _R2(block,e, a, b, c, d, 31);
                _R2(block,d, e, a, b, c, 32); _R2(block,c, d, e, a, b, 33);  _R2(block,b, c, d, e, a, 34);  _R2(block,a, b, c, d, e, 35);
                _R2(block,e, a, b, c, d, 36); _R2(block,d, e, a, b, c, 37);  _R2(block,c, d, e, a, b, 38);  _R2(block,b, c, d, e, a, 39);
                _R3(block,a, b, c, d, e, 40); _R3(block,e, a, b, c, d, 41);  _R3(block,d, e, a, b, c, 42);  _R3(block,c, d, e, a, b, 43);
                _R3(block,b, c, d, e, a, 44); _R3(block,a, b, c, d, e, 45);  _R3(block,e, a, b, c, d, 46);  _R3(block,d, e, a, b, c, 47);
                _R3(block,c, d, e, a, b, 48); _R3(block,b, c, d, e, a, 49);  _R3(block,a, b, c, d, e, 50);  _R3(block,e, a, b, c, d, 51);
                _R3(block,d, e, a, b, c, 52); _R3(block,c, d, e, a, b, 53);  _R3(block,b, c, d, e, a, 54);  _R3(block,a, b, c, d, e, 55);
                _R3(block,e, a, b, c, d, 56); _R3(block,d, e, a, b, c, 57);  _R3(block,c, d, e, a, b, 58);  _R3(block,b, c, d, e, a, 59);
                _R4(block,a, b, c, d, e, 60); _R4(block,e, a, b, c, d, 61);  _R4(block,d, e, a, b, c, 62);  _R4(block,c, d, e, a, b, 63);
                _R4(block,b, c, d, e, a, 64); _R4(block,a, b, c, d, e, 65);  _R4(block,e, a, b, c, d, 66);  _R4(block,d, e, a, b, c, 67);
                _R4(block,c, d, e, a, b, 68); _R4(block,b, c, d, e, a, 69);  _R4(block,a, b, c, d, e, 70);  _R4(block,e, a, b, c, d, 71);
                _R4(block,d, e, a, b, c, 72); _R4(block,c, d, e, a, b, 73);  _R4(block,b, c, d, e, a, 74);  _R4(block,a, b, c, d, e, 75);
                _R4(block,e, a, b, c, d, 76); _R4(block,d, e, a, b, c, 77);  _R4(block,c, d, e, a, b, 78);  _R4(block,b, c, d, e, a, 79);
                /* Add the working vars back into context.state[] */
                state[0] += a;
                state[1] += b;
                state[2] += c;
                state[3] += d;
                state[4] += e;
                /* Wipe variables */
                a = b = c = d = e = 0;

                memset(block, 0, sizeof(block));
            }

		private:
            _ns_tools::type_v_ul_buffer m_v_ul_state;
            _ns_tools::type_v_ul_buffer m_v_ul_count;
			_ns_tools::type_v_buffer m_v_buffer;

		};//ct_sha1
	}//_algorithm

}//_ns_tools
