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
lowest = 180
best = []
File.open("data/results.txt").each do |f|
  a = f.chomp.split(",")
  hd = a[0].to_i

  if hd <= lowest && hd > 0
    lowest = hd
    best = a
  end
end

print "Searching for a HD of #{lowest}\n"

suffix = best[1] + " "

stems = best[7..-1]
suffix[4] = best[2].to_i

official_phrase = "I would much rather hear more about your whittling project"
op_digest = Digest::SHA1.digest(official_phrase)

stems.each do |s|
  digest = Digest::SHA1.digest(s+suffix)
  d = hamming_distance(op_digest.bit_string, digest.bit_string)
  if d == lowest
    print "The phrase is:\n #{s+suffix}\n"
    exit
  end
end
