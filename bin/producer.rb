#!/usr/bin/env ruby

require 'lib/shared'

STEMS_PER_MESSAGE = 32 
THROTTLE = 15


def official_phrase
  @official_phrase ||= File.open("data/official_phrase.txt") { |f| f.read }.chomp
end

# stem is ASCII string
def work_message(stems)
  {
    :official_phrase => official_phrase,
    :stems => stems,
    :permutations => ((94**4) / 16) # Consumers will actually do (this number)*94*32
  }.to_yaml
end

def send_work_message(stems)
  wm = work_message(stems)
  stem_queue.send_message(work_message(stems))
end

load_words
loop do
  p stem_queue.size
  5.times { 
    stems = []
    STEMS_PER_MESSAGE.times { stems << generate_fixed_stem }
    send_work_message(stems)
  }
  sleep THROTTLE
end
