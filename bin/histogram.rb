histogram = {}
histogram.default = 0

File.open("data/results.txt").each_line do |l|
  n = l.split(" ").first.to_i
  histogram[n] += 1
end

histogram.keys.sort.reverse.each do |k|
  if histogram.has_key?(k + 1)
    ratio = histogram[k].to_f / histogram[k+1]
  else
    ratio = nil
  end
  print "#{k}: #{histogram[k]} (#{ratio})\n"
end
