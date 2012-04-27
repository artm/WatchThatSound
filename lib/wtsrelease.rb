class WtsRelease
  def initialize
    @components = {
      "MAJOR" => 0,
      "MINOR" => 1,
      "PATCH" => 2,
    }
  end
  def parse_version str
    ver = [0,0,0]
    str.each_line do | line |
      if /CPACK_PACKAGE_VERSION_(?<comp>[[:alpha:]]+)\s+"(?<value>\d+)"\)/ =~ line
        next unless @components[comp]
        ver[ @components[comp] ] = Integer(value)
      end
    end
    ver
  end

  def parse_tags str
    str.each_line do | line |
      if /^(?<maj>\d+)\.(?<min>\d+)\.(?<patch>\d+)$/ =~ line
        ver = [maj,min,patch].map {|x| Integer(x)}
        if !@latest_version || version_cmp(ver,@latest_version) > 0
          @latest_version = ver
        end
      end
    end

  end

  def bump_needed version
    @latest_version && version_cmp(@latest_version,version) >= 0
  end

  def version_cmp v1, v2
    v1.zip(v2).each do |n1,n2|
      if n1 > n2
        return 1
      elsif n1 < n2
        return -1
      end
    end
    return 0
  end

  def filter_cmake_version version, line
    if /CPACK_PACKAGE_VERSION_(?<comp>[[:alpha:]]+)\s+"(?<value>\d+)"\)/ =~ line && @components[comp]
      line.sub!(/(?<=#{comp})\s+"\d+/, ' "' + version[ @components[comp] ].to_s)
    end
    line
  end
end
