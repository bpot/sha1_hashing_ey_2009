#!/usr/bin/env ruby

require File.join(File.dirname(__FILE__), '../lib/shared')

def get_message
  messages = stem_queue.receive_messages(1)
  return nil if messages.empty?

  message = messages.first
  message_body = YAML.load(message.body)
  message.delete

  return message_body
end

def process_stems(phrase, stems, permutations)
  results = []
  ss = stems.collect{|s| "\\\"#{s}\\\"" }.join(" ")
  t = Time.now
  results << %x(#{elwood_root}/bin/next_gen #{permutations} "#{stems[0]}" "#{stems[1]}" "#{stems[2]}" "#{stems[3]}" "#{stems[4]}" "#{stems[5]}" "#{stems[6]}" "#{stems[7]}" "#{stems[8]}" "#{stems[9]}" "#{stems[10]}" "#{stems[11]}" "#{stems[12]}" "#{stems[13]}" "#{stems[14]}" "#{stems[15]}" "#{stems[16]}" "#{stems[17]}" "#{stems[18]}" "#{stems[19]}" "#{stems[20]}" "#{stems[21]}" "#{stems[22]}" "#{stems[23]}" "#{stems[24]}" "#{stems[25]}" "#{stems[26]}" "#{stems[27]}" "#{stems[28]}" "#{stems[29]}" "#{stems[30]}" "#{stems[31]}" "#{stems[32]}").split("\n")

  @took = Time.now - t
  print "Command took #{Time.now - t}\n"
  results.flatten
end

def send_results(results,stems, permutations)
  res = {}
  res[:results] = results
  res[:hostname] = hostname
  res[:completed_at] = Time.now.to_s
  res[:took] = @took
	res[:ss] = stems.join(",")
	res[:permutations] = permutations
  results_queue.send_message(res.to_yaml)
end

def hostname
  `hostname`.chomp
end

loop do
  message = get_message

  if message
    t = Time.now
    stems = message[:stems]
    phrase = message[:official_phrase]
    permutations = message[:permutations]
    print "Processing message...\n"
    results = process_stems(phrase,stems, permutations)
    send_results(results,stems,permutations) 
    print "done (#{Time.now - t})\n"
  else
    print "No message\n"
    print "sleeping\n"
    sleep 5 
  end
end
