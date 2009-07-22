class String
  def bit_string
    s = ""
    self.each_byte do |c|
      s += "%08d" % c.to_s(2)
    end
    s
  end
end
