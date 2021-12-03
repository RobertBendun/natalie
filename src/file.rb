class File
  SEPARATOR = '/'.freeze

  def self.join(*parts)
    parts.join(SEPARATOR)
  end


  def self.dirname(path)
    dir = path.sub(%r{#{SEPARATOR}\z}, '').split(SEPARATOR)[0..-2].join(SEPARATOR)
    return SEPARATOR if dir.empty? && path.start_with?(SEPARATOR)
    return '.' if dir.empty?
    dir
  end
end

def File.readlines(filename)
  return File.open(filename) do |f|
    return f.readlines()
  end
end
