#!/usr/bin/env ruby

require 'lib/shared'

RESULTS_FILE = "data/results.txt"

def get_results
  messages = results_queue.receive_messages(10)
  print "Get #{messages.size} result messages\n"
  return nil if messages.empty?

  results = []
  messages.each do |m|
    mb = YAML.load(m.body)
    results << mb
    m.delete
  end

  return results
end


loop do
  results = get_results

  if results
    p "got some results"
    f = File.open(RESULTS_FILE, "a")
    results.each do |r|
      f.write  "#{r[:results]},#{r[:permutations]},#{r[:hostname]},#{r[:completed_at]},#{r[:took]},#{r[:ss]}\n"
    end
    f.close
  else
    p "no messages"
  end

  print "sleeping\n"
  sleep  10 
end

