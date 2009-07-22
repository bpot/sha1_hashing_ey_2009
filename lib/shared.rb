require 'rubygems'
require 'right_aws'

def elwood_root
  File.join(File.dirname(__FILE__), '../')
end

def aws_credentials
  YAML.load_file("#{elwood_root}/data/aws_credentials.yml")
end

def stem_queue
  creds = aws_credentials
  @stem_queue ||=  RightAws::SqsGen2.new(creds['access_key_id'], creds['secret_access_key']).queue("elwood_stems_xl", true) 
end

def results_queue
  creds = aws_credentials
  @results_queue ||=  RightAws::SqsGen2.new(creds['access_key_id'], creds['secret_access_key']).queue("elwood_results_xl", true)
end

def load_words
  @words = []
  @words_by_size = {}
  File.open("data/words.txt").each do |l|
    l.chomp!
    @words << l 
    @words_by_size[l.size] ||= []
    @words_by_size[l.size] << l
  end
end

def generate_fixed_stem
  word_set = generate_word_set
  generate_phrase_for_word_set(word_set)
end

def generate_stem(number_of_words = 12)
  stem = []

  number_of_words.times do 
    index = rand(@words.size)
    stem << @words[index]
  end

  stem.join(" ")
end

def generate_word_set(words = 12, length = 63, word_length = 3..10)
  range_size = word_length.entries.size

  non_space_length = length - (words - 1)

  loop do
    set = []

    (words-1).times {
      size = rand(range_size)
      set << size + word_length.begin
    }


    padding = non_space_length - set.inject(0) { |total, s| total += s}

    if word_length.member?(padding)
      set << padding
      return set
    end
  end
end

def generate_phrase_for_word_set(set)
  words = []

  set.each do |s|
    possible_words = @words_by_size[s].size
    words << @words_by_size[s][rand(possible_words)]
  end

  words.join(" ") + " "
end
