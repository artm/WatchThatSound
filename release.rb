#!/usr/bin/env ruby

require 'tempfile'
require './lib/wtsrelease'

VERSION_CMAKE_SOURCE = 'CMakeModules/WTS-Package.cmake'
BUILD_DIR='../qtcreator-build'
UPDATE_XML='devel.xml'

def ver(a)
  a.join('.')
end

r = WtsRelease.new

cur_version = nil
File.open(VERSION_CMAKE_SOURCE) do |file|
  cur_version = r.parse_version file
end

puts "Current version according to #{VERSION_CMAKE_SOURCE} is #{ver(cur_version)}"

r.parse_tags `git tag`
if r.bump_needed cur_version
  cur_version[2] += 1
  puts "According to git tags this version already exists, bumping to #{ver(cur_version)}"
  tmp = Tempfile.new 'wts-release'
  File.open(VERSION_CMAKE_SOURCE) do |file|
    file.each_line do |line|
      tmp.puts r.filter_cmake_version(cur_version,line)
    end
  end
  tmp.close
  File.rename(tmp.path,VERSION_CMAKE_SOURCE)
end

v = ver(cur_version)

puts "TODO insert git log since the last version"
puts "TODO or use existing dist/ReleaseNotes-#{v}.html"
tmp = Tempfile.new 'release.html'
tmp.write <<END
<h2>Nieuw in deze versie</h2>
<ul>
<li></li>
</ul>
]]>
END
tmp.close
system "vim #{tmp.path}"
File.rename(tmp.path,"dist/ReleaseNotes-#{v}.html")

puts "Building the app"
unless system "make -C #{BUILD_DIR}"
  raise "Build failed - copping out"
end

zip_filename = "WatchThatSound-#{v}.zip"
puts "Building a package #{zip_filename}"

unless system "cd #{BUILD_DIR} ; rm -f #{zip_filename} ; zip -ry #{zip_filename} WatchThatSound.app"
  raise "Packaging failed - copping out"
end
zip_size = File.size "#{BUILD_DIR}/#{zip_filename}"

desc=<<END
<item>
<title>#{v}</title>
<sparkle:releaseNotesLink>
https://ftp.v2.nl/~artm/WTS3/ReleaseNotes-#{v}.html
</sparkle:releaseNotesLink>
<pubDate>#{`date`}</pubDate>
<enclosure
url="https://ftp.v2.nl/~artm/WTS3/WatchThatSound-#{v}.zip"
sparkle:version="#{v}"
length="#{zip_size}"
type="application/octet-stream"/>
</item>

END

tmp = Tempfile.new UPDATE_XML
File.open("dist/#{UPDATE_XML}") do |file|
  file.each_line do |line|
    tmp.puts desc if /<\/channel>/ =~ line
    tmp.puts line
  end
end
tmp.close
File.rename tmp.path, "dist/#{UPDATE_XML}"

puts "commit to git and tag"
unless system "git add dist/ReleaseNotes-#{v}.html && git commit -am 'releasing #{v}' && git tag #{v}"
  raise "Commiting or tagging the release failed"
end

puts "TODO upload all files to the ftp..."
puts "TODO send notification email (?)"


