#!/usr/bin/env ruby

require 'tempfile'
require 'fileutils'
require_relative 'lib/wtsrelease'

SOURCE_DIR='../WatchThatSound'

unless File.exists? 'wts_version.h'
  puts "Have to call this script from a build directory"
  exit 1
end

cur_version = nil
update_channel = update_xml = nil
`cmake -L .`.each_line do |line|
  if /(?<varname>\w+):\w+=(?<varvalue>.*)/ =~ line
    case varname
    when 'PROJECT_VERSION'
      cur_version = varvalue
    when 'PROJECT_UPDATE_CHANNEL'
      update_channel = varvalue
      update_xml = File.basename varvalue
    end
  end
end

puts "current version: #{cur_version}"
puts "update channel:  #{update_channel}"
puts "update xml:      #{update_xml}"

r = WtsRelease.new
r.parse_tags `cd #{SOURCE_DIR} && git tag`
cur_version_list = cur_version.split('.').map{|v|Integer v}
if r.bump_needed cur_version_list
  cur_version_list[2] += 1
  cur_version = cur_version_list.join('.')
  puts "According to git tags this version already exists, bumping to #{cur_version}"
  tmp = Tempfile.new 'wts-release'
  wts_package_cmake = "#{SOURCE_DIR}/CMakeModules/WTS-Package.cmake"
  File.open(wts_package_cmake) do |file|
    file.each_line do |line|
      tmp.puts r.filter_cmake_version(cur_version_list,line)
    end
  end
  tmp.close
  File.rename(tmp.path,wts_package_cmake)
end


puts "TODO insert change log from devel versions to the public release"
tmp = Tempfile.new 'release.html'
tmp.write <<END
<h2>Nieuw in deze versie</h2>
<ul>
<li></li>
</ul>
END
tmp.close

rel_notes = "#{SOURCE_DIR}/dist/ReleaseNotes-#{cur_version}.html"
if File.exists? rel_notes
  FileUtils.cp rel_notes, tmp.path
end

system "vim #{tmp.path}"
File.rename(tmp.path,rel_notes)

puts "Building the app"
unless system "make"
  raise "Build failed - copping out"
end

zip_filename = "WatchThatSound-#{cur_version}.zip"
puts "Building a package #{zip_filename}"

unless system "rm -f #{zip_filename} ; zip -ry #{zip_filename} WatchThatSound.app"
  raise "Packaging failed - copping out"
end
zip_size = File.size "#{zip_filename}"

desc=<<END
<item>
<title>#{cur_version}</title>
<sparkle:releaseNotesLink>
https://ftp.v2.nl/~artm/WTS3/ReleaseNotes-#{cur_version}.html
</sparkle:releaseNotesLink>
<pubDate>#{`date`}</pubDate>
<enclosure
url="https://ftp.v2.nl/~artm/WTS3/WatchThatSound-#{cur_version}.zip"
sparkle:version="#{cur_version}"
length="#{zip_size}"
type="application/octet-stream"/>
</item>

END

update_xml_dst = "#{SOURCE_DIR}/dist/#{update_xml}"
tmp = Tempfile.new update_xml
File.open(update_xml_dst) do |file|
  file.each_line do |line|
    tmp.puts desc if /<\/channel>/ =~ line
    tmp.puts line
  end
end
tmp.close
File.rename tmp.path, update_xml_dst

puts "Uploading all files to the ftp..."
unless system "curl --netrc -T '{#{zip_filename},#{update_xml_dst},#{rel_notes}}' ftp://ftp.v2.nl/public_html/WTS3/ "
  raise "Upload failed"
end

puts "commit to git and tag"
unless system "cd #{SOURCE_DIR} && git add dist/ReleaseNotes-#{cur_version}.html && git commit -am 'releasing #{cur_version}' && git tag #{cur_version}"
  raise "Commiting or tagging the release failed"
end

