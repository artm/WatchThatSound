require 'wtsrelease'

describe WtsRelease do
  before do
    @releaser = WtsRelease.new
  end

  it "can parse current version" do
    # the text is in CMakeModules/WTS-Package.cmake
    version = @releaser.parse_version <<END
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "14")
END
    version.should == [3,0,14]
  end

  it "can compare versions" do
    @releaser.version_cmp([1,0,0],[1,0,0]).should == 0
    @releaser.version_cmp([1,0,0],[0,0,0]).should == 1
    @releaser.version_cmp([0,0,0],[1,0,0]).should == -1
    @releaser.version_cmp([1,1,0],[1,0,0]).should == 1
    @releaser.version_cmp([1,0,0],[1,1,0]).should == -1
    @releaser.version_cmp([1,0,1],[1,0,0]).should == 1
    @releaser.version_cmp([1,0,0],[1,0,1]).should == -1
  end

  it "can determine if version bump is necessary" do
    # these normally come from `git tag`
    tags=<<END
3-alpha-9.2
3-alpha-9.3
3.0.10
3.0.11a
3.0.12
3.0.13
END
  @releaser.parse_tags tags
  @releaser.bump_needed([3,0,13]).should be
  @releaser.bump_needed([3,0,14]).should_not be
  end

  it "bumps version number" do
    cmake_text = <<END
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "14")
END
    new_text = []
    cmake_text.each_line do |line|
      new_text << @releaser.filter_cmake_version([3,0,15], line)
    end
    new_text.join('').should == <<END
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "15")
END
  end
end
