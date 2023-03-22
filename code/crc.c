// SPDX-License-Identifier: zlib-acknowledgement

// Checksum used for accidental changes, hashing produces unique value

// CRC is more involved checksum (rather than just checking last bytes of literal sum)
// CRC better for runs of incorrect bits.
// Redundancy named as including redundant information to detect errors

// I do know that the particular crc polynomial I am using is good for detecting up to 8 bit flips in about 900 bytes. 
// I am using this to protect against flips in messages no longer than 512 bytes.
// What happens if I use the same polynomial to check chunks of data that are much bigger, say, 50kb or 100kb? Is the check weaker?
// 
// It will be slightly weaker for long messages. 
// For completely random error vectors in long messages, the probability of not detecting the error will be about 1 in 232. 
// That applies for any reasonable 32 bit CRC polynomial. The only way to improve on that is to use a longer polynomial, e.g. 64 bit.
// For shorter messages or shorter error bursts, the choice of polynomial makes a big difference to the detection capability.
// 
// number of flipped bits that can detected is Hamming distance
// 
// crc32 and crc32c.
// IEEE and Castagnoli polynomials

INTERNAL void
crc_init(void)
{
	__HAL_RCC_CRC_CLK_ENABLE();
}
