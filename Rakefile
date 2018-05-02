
require 'fileutils'

def reformat (file)
  puts "reformat #{file}"
  sh "uncrustify --replace -c default.cfg #{file}"
end


files = %w( src/Recorder.cpp src/vcvrecorder.cpp src/vcvrecorder.hpp )

task :reformat do
  files.each do |file|
    reformat file
  end
end



task "test"  do
  sh "./runtest.sh"
end


task :default => [ :reformat ]
