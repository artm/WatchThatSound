require 'wtsrelease'

describe WtsRelease do
  before do
    @releaser = WtsRelease.new
  end
  it "can parse current version" do
    version = @releaser.parse_version <<END
SET(CPACK_PACKAGE_VERSION_MAJOR "3")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "14")
END
    version.should == [3,0,14]
  end
end
