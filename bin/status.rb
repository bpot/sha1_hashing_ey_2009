#!/usr/bin/env ruby
require 'lib/shared'
require 'time'
RESULTS_FILE ="data/results.txt"

lowest = 180
work_units = 0
perms = 0
first_time = Time.now
last_time = Time.at(0)
servers = {}
servers.default = 0
File.open(RESULTS_FILE).each do |l|
  a = l.split(",")
  low = a[0].to_i
  if low < lowest
    lowest = low
  end

  host = a[4]
  servers[host] += 1

  perms += a[3].to_i
  work_units += 1
  begin
    next unless a.size == 39
    dt = Time.parse(a[5])
    if dt < first_time
      first_time = dt
    end
    if dt.to_i > last_time.to_i
      last_time = dt
    end
  rescue
  end
end

millions_of_keys = perms*32*94/1_000_000

print "Lowest: #{lowest}\n\n"

print "Work Units: #{work_units}\n"
print "First WU: #{first_time.to_s} - #{(Time.now-first_time).to_i/60} minutes ago\n"
print "Last WU: #{last_time.to_s} - #{(Time.now-last_time).to_i/60} minutes ago\n\n"

print "Total Keys Checked: #{millions_of_keys}M\n"
print "Total Keys/s: #{millions_of_keys/(last_time-first_time).to_i}M\n"
