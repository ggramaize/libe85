# E85 

stable

    Name: github.com/ggramaize/libz85/spec_e85.mq
    Status: stable
    Editor: Geoffroy GRAMAIZE geoffroy+dev@gramaize.eu

This document specifies E85, a format for representing binary data as printable text. E85 is an extension of Z85 encoding mechanisms, modified to provide byte-accurate binary stream wrapping.

# Preamble

Copyright (c) 2017 Geoffroy GRAMAIZE.

This Specification is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version. This Specification is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program; if not, see http://www.gnu.org/licenses.

This Specification is a free and open standard and is governed by the Digital Standards Organization's Consensus-Oriented Specification System.

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

# Problem Statement

When representing binary values in code and in text files (such as configuration data), the developer must choose a printable representation. The simplest choice is Base16, where each byte is printed as two hexadecimal values.

For better efficiency than Base16, the developer might choose Base64. This is a common choice but problematic because it has more than a dozen variants. It works poorly with binary data since it works on three-byte chunks, whereas most binary data is chunked to 4 or 8 bytes. Base64 implementations are therefore more complex than they might be, and not necessarily interoperable.
œ
A more logical encoding is Z85, which works on 4-byte chunks. It is string-safe, so can be used cleanly in source code, XML, JSON, and so on.

However, Z85 does not enable byte-accurate binary stream transport, because the binary frame SHALL have a length divisible by 4.

Our ideal encoding is designed for 4-byte chunks like Z85, but is able to handle binary frames of non divisible-by-4 length. To achieve this, E85 implements an erasure mechanism similar to base64.

An E85 implementation is able to natively decode a Z85 string frame.

# Formal Specification

An E85 implementation takes a binary frame and encodes it as a printable ASCII string, or takes an ASCII encoded string and decodes it into a binary frame.

The erasure character is defined as the character which value equals 0x5F. It SHALL be placed at the end of a string frame. The erasure character SHALL NOT be repeated more than 3 times.

The binary frame SHALL have any length. If the binary frame length is not divisible by 4 with no remainder, an E85 implementation SHALL zero-pad the binary frame to generate a valid Z85 binary frame, and SHALL append the erasure character as many times as it added zero-padding bytes.

The string frame, excluding the erasure characters, SHALL have a length that is divisible by 5 with no remainder.

The encoding and decoding SHALL use this representation for each base-85 value from zero to 84:

 0 -  9:  0 1 2 3 4 5 6 7 8 9
10 - 19:  a b c d e f g h i j
20 - 29:  k l m n o p q r s t
30 - 39:  u v w x y z A B C D
40 - 49:  E F G H I J K L M N
50 - 59:  O P Q R S T U V W X
60 - 69:  Y Z . - : + = ^ ! /
70 - 79:  * ? & < > ( ) [ ] {
80 - 84:  } @ % $ #

To encode a frame, an implementation SHALL take four octets at a time from the binary frame and convert them into five printable characters. The four octets SHALL be treated as an unsigned 32-bit integer in network byte order (big endian). The five characters SHALL be output from most significant to least significant (big endian).

To decode a string, an implementation SHALL take five characters at a time from the string and convert them into four octets of data representing a 32-bit unsigned integer in network byte order. The five characters SHALL each be converted into a value 0 to 84, and accumulated by multiplication by 85, from most to least significant.

# Test Case

As a test case, a frame containing these 9 bytes:

+------+------+------+------+------+------+------+------+------+
| 0x86 | 0x4F | 0xD2 | 0x6F | 0xB5 | 0x59 | 0xF7 | 0x5B | 0x00 |
+------+------+------+------+------+------+------+------+------+

SHALL encode as the following 18 characters:

+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| H | e | l | l | o | W | o | r | l | d | 0 | 0 | 0 | 0 | 0 | _ | _ | _ |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

# Reference Implementation

A reference implementation in C is provided in the repository at https://github.com/ggramaize/libz85/

# Security Considerations

Implementations MUST take care that buffers intended to receive encoded and decoded data are large enough. Clearly, a properly-crafted encoded string can create any arbitrary binary sequence, which makes E85 an easy vector for attack into a badly-designed implementation.
