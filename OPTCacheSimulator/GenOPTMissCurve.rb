#!/usr/bin/ruby
if ARGV.length != 3
  puts "Usage: ./GenOPTMissCurve.rb TraceFileName StartingCacheSize EndingCacheSize"
  exit
end

originalTraceFileName = ARGV[0]
startingCacheSize = ARGV[1].to_i
endingCacheSize = ARGV[2].to_i

cacheSize = startingCacheSize
cacheLineSize = 64
cacheAssociativity = 16

parts = originalTraceFileName.split('_')
testcaseName = parts[0]
suffix = parts[2]
tdisTraceFileName = testcaseName + "_tdis_" + suffix

puts "running .... rm OPT_#{cacheAssociativity}_mc.dat"
system "rm -f OPT_#{cacheAssociativity}_mc.dat"

puts "running .... GenForwardTimeDistance.rb #{cacheSize} #{cacheLineSize} #{cacheAssociativity} #{originalTraceFileName}"
system "GenForwardTimeDistance.rb #{cacheSize} #{cacheLineSize} #{cacheAssociativity} #{originalTraceFileName}"

while cacheSize <= endingCacheSize
  puts "----------------------------------------------------"
  puts "running .... OPTCacheSimulator #{cacheSize} #{cacheLineSize} #{cacheAssociativity} #{tdisTraceFileName} >> OPT_#{cacheAssociativity}_mc.dat"
  system "OPTCacheSimulator #{cacheSize} #{cacheLineSize} #{cacheAssociativity} #{tdisTraceFileName} >> OPT_#{cacheAssociativity}_mc.dat"
  cacheSize = cacheSize*2
end

puts "rm -f #{tdisTraceFileName}"
system "rm -f #{tdisTraceFileName}"
