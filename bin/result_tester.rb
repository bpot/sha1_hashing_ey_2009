#!/usr/bin/env ruby
require 'digest/sha1'
require 'lib/string'
def hamming_distance(a,b)
  idx = 0
  distance = 0

  a.each_byte do |a_byte|
    b_byte = b[idx]

    if a_byte != b_byte
      distance += 1
    end

    idx += 1
  end

  distance
end
official_phrase = "I would much rather hear more about your whittling project"
op_digest = Digest::SHA1.digest(official_phrase)

digest = Digest::SHA1.digest(ARGV[0])
d = hamming_distance(op_digest.bit_string, digest.bit_string)
p d
