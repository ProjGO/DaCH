#ifndef RAW_CACHE_H
#define RAW_CACHE_H

#include "address.h"
#include "ap_int.h"
#ifdef __SYNTHESIS__
#include "hls_vector.h"
#else
#include <array>
#endif /* __SYNTHESIS__ */

template <typename T, size_t ADDR_SIZE, size_t TAG_SIZE, size_t N_WORDS_PER_LINE>
class raw_cache {
	private:
		static const size_t OFF_SIZE = (ADDR_SIZE - TAG_SIZE);

		typedef address<ADDR_SIZE, TAG_SIZE, 0, 0> addr_type;
#ifdef __SYNTHESIS__
		typedef hls::vector<T, N_WORDS_PER_LINE> line_type;
#else
		typedef std::array<T, N_WORDS_PER_LINE> line_type;
#endif /* __SYNTHESIS__ */

		bool m_valid;
		line_type m_line;
		ap_uint<(TAG_SIZE > 0) ? TAG_SIZE : 1> m_tag;

	public:
		void init() {
			m_valid = false;
		}

		void get_line(const T * const main_mem,
				const ap_uint<(ADDR_SIZE > 0) ? ADDR_SIZE : 1> addr_main,
				line_type &line) {
#pragma HLS inline
			const addr_type addr(addr_main);

			if (hit(addr)) {
				for (auto off = 0; off < N_WORDS_PER_LINE; off++) {
#pragma HLS unroll
					line[off] = m_line[off];
				}
			} else {
				const T *main_line = &(main_mem[addr.m_addr_main & (-1U << OFF_SIZE)]);
				for (auto off = 0; off < N_WORDS_PER_LINE; off++) {
#pragma HLS unroll
					line[off] = main_line[off];
				}
			}
		}

		void set_line(T * const main_mem,
				const ap_uint<(ADDR_SIZE > 0) ? ADDR_SIZE : 1> addr_main,
				const line_type &line) {
#pragma HLS inline
			const addr_type addr(addr_main);

			T * const main_line = &(main_mem[addr.m_addr_main & (-1U << OFF_SIZE)]);
			for (auto off = 0; off < N_WORDS_PER_LINE; off++) {
#pragma HLS unroll
				main_line[off] = line[off];
				m_line[off] = line[off];
			}
			
			m_tag = addr.m_tag;
			m_valid = true;
		}

	private:
		inline bool hit(const addr_type &addr) {
#pragma HLS inline
			return (m_valid && (addr.m_tag == m_tag));
		}
};

#endif /* RAW_CACHE_H */

