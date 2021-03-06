/*
 * Copyright (C) 2013 midnightBITS
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "pch.h"
#include <crypt.hpp>

namespace Crypt
{
	namespace algorithm
	{
		struct SHA512
		{
			bool m_inited;
			SHA512_CTX m_ctx;
		public:
			SHA512()
			{
				m_inited = SHA512_Init(&m_ctx) != 0;
			}
			~SHA512()
			{
				if (m_inited)
					OPENSSL_cleanse(&m_ctx, sizeof(m_ctx));
			}
			bool Update(const void* msg, size_t len)
			{
				if (!m_inited) return false;
				return SHA512_Update(&m_ctx, msg, len) != 0;
			}
			bool Final(SHA512Hash::digest_t& digest)
			{
				if (!m_inited) return false;
				return SHA512_Final(digest, &m_ctx) != 0;
			}
		};

		struct MD5
		{
			bool m_inited;
			MD5_CTX m_ctx;
		public:
			MD5()
			{
				m_inited = MD5_Init(&m_ctx) != 0;
			}
			~MD5()
			{
				if (m_inited)
					OPENSSL_cleanse(&m_ctx, sizeof(m_ctx));
			}
			bool Update(const void* msg, size_t len)
			{
				if (!m_inited) return false;
				return MD5_Update(&m_ctx, msg, len) != 0;
			}
			bool Final(MD5Hash::digest_t& digest)
			{
				if (!m_inited) return false;
				return MD5_Final(digest, &m_ctx) != 0;
			}
		};
	};

	bool SHA512Hash::__crypt(const char* salt, const char* message, size_t len, digest_t &digest)
	{
		algorithm::SHA512 alg;
		if (!alg.Update(salt, SALT_LENGTH)) return false;
		if (!alg.Update(message, len)) return false;
		return alg.Final(digest);
	}

	bool SHA512Hash::simple(const char* message, size_t len, digest_t &digest)
	{
		algorithm::SHA512 alg;
		if (!alg.Update(message, len)) return false;
		return alg.Final(digest);
	}

	bool MD5Hash::__crypt(const char* salt, const char* message, size_t len, digest_t &digest)
	{
		algorithm::MD5 alg;
		if (!alg.Update(salt, SALT_LENGTH)) return false;
		if (!alg.Update(message, len)) return false;
		return alg.Final(digest);
	}

	bool MD5Hash::simple(const char* message, size_t len, digest_t &digest)
	{
		algorithm::MD5 alg;
		if (!alg.Update(message, len)) return false;
		return alg.Final(digest);
	}

	bool verify(const char* message, const char* hash)
	{
		if (!hash)
			return false;

		char alg = hash[strlen(hash) - 1];

		if (alg == SHA512Hash::ALG_ID)
			return SHA512Hash::verify(message, hash);
		if (alg == MD5Hash::ALG_ID)
			return MD5Hash::verify(message, hash);

		return false;
	}

	static char alphabet(size_t id)
	{
		static char alph[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		return alph[id];
	}

	template <class T>
	T sslrand()
	{
		T ret;
		RAND_bytes((unsigned char*)&ret, sizeof(ret));
		return ret;
	}

	static char randomChar()
	{
		// TODO : fixme!
		return alphabet(sslrand<unsigned char>() >> 2);
	}

	static void randomU32(char (&out)[4])
	{
		unsigned char ent[3];
		RAND_bytes(ent, sizeof(ent));
		out[0] = alphabet(ent[0] >> 2);
		out[1] = alphabet((ent[0] & 0x3) | (ent[1] >> 4));
		out[2] = alphabet((ent[1] & 0xF) | (ent[2] >> 6));
		out[3] = alphabet(ent[2] & 0x3F);
	}

	void newSalt(char* salt, size_t len)
	{
		char sample[4];
		salt[--len] = 0;
		size_t tmp = len / 4;
		for (size_t i = 0; i < tmp; ++i)
		{
			randomU32(sample);
			memcpy(salt, sample, 4);
			salt += 4;
		}

		tmp = len % 4;
		if (!tmp) return;

		randomU32(sample);
		memcpy(salt, sample, tmp);
	}

	void base64_encode(const void* data, size_t len, char* output)
	{
		const unsigned char* p = (const unsigned char*)data;
		size_t pos = 0;
		unsigned int bits = 0;
		unsigned int accu = 0;
		for (size_t i = 0; i < len; ++i)
		{
			accu = (accu << 8) | (p[i] & 0xFF);
			bits += 8;
			while (bits >= 6)
			{
				bits -= 6;
				output[pos++] = alphabet((accu >> bits) & 0x3F);
			}
		}
		if (bits > 0)
		{
			accu <<= 6 - bits;
			output[pos++] = alphabet(accu & 0x3F);
		}
	}
};