class WtsRelease
  def parse_version(str)
    ver = [0,0,0]
    components = {
      "MAJOR" => 0,
      "MINOR" => 1,
      "PATCH" => 2,
    }
    str.each_line do | line |
      if /CPACK_PACKAGE_VERSION_(?<comp>[[:alpha:]]+)\s+"(?<value>\d+)"\)/ =~ line
        next unless components[comp]
        ver[ components[comp] ] = Integer(value)
      end
    end
    ver
  end

end
