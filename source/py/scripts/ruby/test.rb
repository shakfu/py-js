#!/usr/bin/env ruby
# Run automated tests in Max
# Copyright (c) 2014, Cycling '74

puts "Max Automated Test Runner"

olddir = Dir.getwd
estring = ""
testpass = ""

require 'rubygems'
require 'timeout'
gemlist = `gem list`.strip
if !/^sqlite3/.match gemlist
  puts
  puts "-> You are missing the 'sqlite3' rubygem, which you need for automated testing."
  puts "-> Please type 'gem install sqlite3' in the Terminal."
  puts
  exit
end
require 'fileutils'
require 'pathname'
require 'sqlite3'
require "#{olddir}/rosc/lib/osc.rb"
require "open3"


###################################################################
# argument processing
###################################################################

if (ARGV.length < 1 || ARGV.length > 2)
  puts "usage: ruby test.rb <path-to-max> <no-exit>"
  puts "examples:"
  puts '  ruby test.rb "/Applications"'
  puts '  ruby test.rb "/Applications/Max.app"  --- you can give the path to the max application directly'
  puts '  ruby test.rb "C:\Program Files\Cycling \'74\Max 8"'
  puts '  ruby test.rb "/Applications/Max8 i386" --- For MacOS, you can specify the arch to run ("i386" for Intel, "arm64" for M1)'
  puts
  exit;
end

@maxfolder = ARGV[0]
@arch = ARGV[1]
@noexit = false
@noexit = true if ARGV.length > 1



curdir = Dir.pwd
Dir.chdir @maxfolder
puts 'MAX FOLDER'
puts Dir.pwd
@maxfolder = Dir.pwd
Dir.chdir curdir


###################################################################
# initialization
###################################################################

@host = 'localhost'
@receivePort = 4792 # This must match the port-send number in testpackage-config.json
@sendPort = 4791    # This must match the port-listen number in testpackage-config.json
@passes = 0
@failures = 0

puts "  Starting the OSC Server..."
puts
@oscReceiver = OSC::UDPServer.new
@oscReceiver.bind @host, @receivePort
@oscSender = OSC::UDPSocket.new
@testdbPath = ""
@starttime = Time.now.iso8601.chop.chop.chop.chop.chop.chop

###################################################################
# sub routines
###################################################################

def establishCommunication
  @pingReturned = 0

  @oscReceiver.add_method('/testdb/path', 's') do |msg|
    @testdbPath = msg.args[0]
  end

  @oscReceiver.add_method('/ping/return', '') do |msg|
    puts "    Ping successfully returned."
    puts ""

    @oscSender.send(OSC::Message.new('/testdb/path?'), 0, @host, @sendPort)
    sleep 1
    @pingReturned = 1
  end

  Thread.new do
    @oscReceiver.serve
  end
  sleep 5

  ping = OSC::Message.new('/ping');
  while @pingReturned == 0
    puts "    Sending ping to Max."
    @oscSender.send(ping, 0, @host, @sendPort)
    sleep 2
  end
end


def waitOnDatabase
  @dbReady = 0

  @oscReceiver.add_method('/db/ready', '') do |msg|
    @dbReady = 1
  end

  Thread.new do
    @oscReceiver.serve
  end
  sleep 10

  ping = OSC::Message.new('/db/ready?');
  while @dbReady == 0
    puts "    Sending query to Max."
    @oscSender.send(ping, 0, @host, @sendPort)
    sleep 10
  end
end


def waitForTestCompletion
  @testCompleted = 0

  @oscReceiver.add_method('/test/complete', '') do |msg|
    @testCompleted = 1
  end

  Thread.new do
    @oscReceiver.serve
  end

  while @testCompleted == 0
    sleep 1
  end
end


def waitForAllTestCompletion
  @testCompleted = 0

  @oscReceiver.add_method('/test/all/complete', '') do |msg|
    @testCompleted = 1
  end

  Thread.new do
    @oscReceiver.serve
  end

  while @testCompleted == 0
    sleep 1
  end
end


def launchMax
  if RUBY_PLATFORM.match(/darwin/)
    archcmd = ""
    archcmd << "arch -arch x86_64" if @arch == 'x86_64'
    archcmd << "arch -arch arm64" if @arch == 'arm64'
    if @maxfolder.match(/\.app\/*$/) # check if app name given directly
      IO.popen("#{archcmd} \"#{@maxfolder}/Contents/MacOS/Max\"")
    else # nope, just a folder name, so assume Max.app
      IO.popen("#{archcmd} \"#{@maxfolder}/Max.app/Contents/MacOS/Max\"")
    end
  else
    IO.popen "\"#{@maxfolder}/Max.exe\""
  end
end


###################################################################
# here is where we actually run the tests
###################################################################

begin
	Timeout::timeout(120) { # For each phase, set a simple timeout so that we can exit test script if Max is not responding.
		puts "  Launching Max..."
		launchMax()
	}
rescue Timeout::Error
	estring << "\n\n  Max could not be launched."
	testpass = "fail"
end

begin
	if testpass != "fail"
		Timeout::timeout(120) {
			puts "  Establishing Communication with Max..."
			establishCommunication()
		}
	end
rescue Timeout::Error
	estring << "\n\n  Communication with Max could not be established."
	testpass = "fail"
end

begin
	if testpass != "fail"
		Timeout::timeout(600) {
			puts "  Waiting for the Max database to complete..."
			waitOnDatabase()
		}
	end
rescue Timeout::Error
	estring << "\n\n  Max Database harvesting did not complete."
	testpass = "fail"
end

begin
	if testpass != "fail"
		Timeout::timeout(600) {
			puts
			puts "  Telling Max to run all of the tests for us..."
			mess = OSC::Message.new "test.master run"
			@oscSender.send(mess, 0, @host, @sendPort)
			waitForAllTestCompletion()
		}
	end
rescue Timeout::Error
	estring << "\n\n  Max was interrupted during tests."
	testpass = "fail"
end


mess = OSC::Message.new 'max quit'
@oscSender.send(mess, 0, @host, @sendPort)

puts
puts "  RESULTS"

sleep 5 # hack -- the db might still be open because it doesn't get flushed in a quittask...

db = SQLite3::Database.new( "#{@testdbPath}" )

testcount = db.execute("SELECT test_name FROM tests WHERE test_start >= Datetime('#{@starttime}') ").length
assertcount = db.execute("SELECT assertion_name FROM assertions WHERE assertion_finish >= Datetime('#{@starttime}') ").length

estring << "\n"
estring << "    Executed #{testcount} Tests with #{assertcount} Assertions"
failed_assertions = db.execute("SELECT assertion_name FROM assertions WHERE assertion_value != 'Pass' AND assertion_finish >= Datetime('#{@starttime}')").length
if (failed_assertions == 0 && testpass != "fail")
  estring << "\n    All assertions passed.  Congratulations!"
  testpass = "pass"
else
  failed_tests = Hash.new
  testpass = "fail"
  db.execute("SELECT assertion_id, test_id_ext, assertion_name FROM assertions WHERE assertion_value != 'Pass' AND  assertion_finish >= Datetime('#{@starttime}')") do |row|
    failed_tests[row[1]] = true;
  end

  estring << "\n    #{failed_assertions} assertion(s) failed in #{failed_tests.length} test(s)"
  failed_tests.each do |test_id, unused|
    testname = db.execute("SELECT test_name FROM tests WHERE test_id = #{test_id}")
    estring << "\n\n    FAILED TEST ( #{testname} )"
    db.execute( "SELECT assertion_id, assertion_name, assertion_value, assertion_finish FROM assertions WHERE test_id_ext = #{test_id}" ) do |row|
      estring << "\n      assertion: #{row[1]}    result: #{row[2]} #{'****' if row[2]!='Pass'}"
	end
    db.execute( "SELECT log_id, text, timestamp FROM logs WHERE test_id_ext = #{test_id}" ) do |row|
      estring << "\n      log: #{row[1]}"
    end
  end
end

estring << "\n\n"
# export results so a caller of this script is able to access the summary for e.g. automated email delivery
ENV['MAXTEST'] = estring # log
ENV['MAXTEST_PASS'] = testpass # general pass/fail

puts estring
puts testpass

puts "    Full test results can be found @ "
puts "        #{@testdbPath} "
puts "    and explored in your favorite SQLite database client."
puts

@oscSender.close # don't forget to close the ports!
@oscReceiver.close

# Must explicitly exit or we end up with a zombie process due to un-joined threads
exit 0 if !@noexit
